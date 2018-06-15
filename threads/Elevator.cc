#include "Elevator.h"
#include "system.h"

extern Building *buildingInstance;

Elevator::Elevator(char* debugName, int numFloors, int myID)
{
	name = debugName;
	this->numFloors = numFloors;
	this->myID = myID;
	lock = new Lock("Elevator's lock");
	state = STOP;
	elevatorOut = new EventBarrier*[numFloors + 1];
	myThread = NULL;
	currentfloor = 1;
	occupancy = 0;

	for(int i = 1; i <= numFloors; i++)
	{
		char *outName = new char[25];
		sprintf(outName, "out barrier for %dth floor", i);
		elevatorOut[i] = new EventBarrier(outName);
	}
}

Elevator::~Elevator()
{
	delete lock;
	delete elevatorOut;
}

void Elevator::OpenDoors()
{
	myAlarm->Pause(TIME_DOOR_OPEN);
	
	if(elevatorOut[currentfloor]->Waiters() > 0)
		elevatorOut[currentfloor]->Signal();

	if(state == UP && buildingInstance->elevatorUp[currentfloor]->Waiters() > 0)
		buildingInstance->elevatorUp[currentfloor]->Signal();
	else if(state == DOWN && buildingInstance->elevatorDown[currentfloor]->Waiters() > 0)
		buildingInstance->elevatorDown[currentfloor]->Signal();
}

void Elevator::CloseDoors()
{
	myAlarm->Pause(TIME_DOOR_OPEN);
}

void Elevator::VisitFloor(int floor)
{
	int moveFloors = currentfloor - floor;
	moveFloors = (moveFloors > 0) ? moveFloors : -moveFloors;
	int moveTime = TIME_ELEVATOR_MOVE * moveFloors;
	myAlarm->Pause(moveTime);
	DEBUG('e', "Elevator arrived at %dth floor\n", floor);
	currentfloor = floor;
}

int Elevator::getNextFloor()
{
	int longestRequest = 0;
	int i;
	if(state == STOP)
	{
		for(i = 1; i <= numFloors; i++)
		{
			if(buildingInstance->elevatorUp[i]->Waiters() > 0 || buildingInstance->elevatorDown[i]->Waiters() > 0 || elevatorOut[i]->Waiters() > 0)
				longestRequest = i;
		}
	}
	else if(state == UP)
	{
		for(i = currentfloor; i <= numFloors; i++)
		{
			if(buildingInstance->elevatorUp[i]->Waiters() > 0 || buildingInstance->elevatorDown[i]->Waiters() > 0 || elevatorOut[i]->Waiters() > 0)
				longestRequest = i;
		}

	}
	else if(state == DOWN)
	{
		for(i = currentfloor; i >= 1;  i--)
		{
			if(buildingInstance->elevatorUp[i]->Waiters() > 0 || buildingInstance->elevatorDown[i]->Waiters() > 0 || elevatorOut[i]->Waiters() > 0)
				longestRequest = i;
		}
	}
	
	return longestRequest;
}

void Elevator::Run()
{
	myThread = currentThread;
	while(1)
	{
		int destFloor = getNextFloor();
		DEBUG('t', "Elevator's destination=%d\n", destFloor);
		if(state == STOP)
		{
			if(destFloor == 0)
			{
				IntStatus oldlevel = interrupt->SetLevel(IntOff);
				currentThread->Sleep();
				(void)interrupt->SetLevel(oldlevel);
			}
			else state = (destFloor - currentfloor) > 0 ? UP : DOWN;
				
		}
		else if(state == UP)
		{
			while(currentfloor != destFloor && destFloor != 0)
			{
				if(buildingInstance->elevatorUp[currentfloor]->Waiters() > 0 || elevatorOut[currentfloor]->Waiters() > 0)
				{
					OpenDoors();
					CloseDoors();
				}
				VisitFloor(currentfloor + 1);
				destFloor = getNextFloor();
			}
			if(destFloor == 0)
			{
				state = STOP;
			}
			else
			{
				if(buildingInstance->elevatorUp[currentfloor]->Waiters() == 0)
				{
					state = DOWN;
					OpenDoors();
					CloseDoors();
					VisitFloor(currentfloor - 1);
				}
				else
				{
					OpenDoors();
					CloseDoors();
					VisitFloor(currentfloor + 1);
				}
			}
		}
		else if(state == DOWN)
		{
			while(currentfloor != destFloor && destFloor != 0)
			{
				if(buildingInstance->elevatorDown[currentfloor]->Waiters() > 0 || elevatorOut[currentfloor]->Waiters() > 0)
				{
					OpenDoors();
					CloseDoors();
				}
				VisitFloor(currentfloor - 1);
				destFloor = getNextFloor();
			}
			if(destFloor == 0)
			{
				state = STOP;
			}
			else
			{
				if(buildingInstance->elevatorDown[currentfloor]->Waiters() == 0)
				{
					state = UP;
					OpenDoors();
					CloseDoors();
					VisitFloor(currentfloor + 1);
				}
				else
				{
					OpenDoors();
					CloseDoors();
					VisitFloor(currentfloor - 1);
				}
			}
		}
	}
}

bool Elevator::Enter()
{
       myAlarm->Pause(TIME_RIDER_ENTER);
	occupancy++;
	if(state == UP)
		buildingInstance->elevatorUp[currentfloor]->Complete();
	else if(state == DOWN)
		buildingInstance->elevatorDown[currentfloor]->Complete();
	return true;
}

void Elevator::Exit()
{
	myAlarm->Pause(TIME_DOOR_OPEN);
	occupancy--;
	elevatorOut[currentfloor]->Complete();
}

void Elevator::RequestFloor(int floor)
{
	elevatorOut[floor]->Wait();
}

void Elevator::ReceiveRequest(int Floor)
{
	if(state == STOP)
	{
		scheduler->ReadyToRun(myThread);
		if(Floor > currentfloor) state = UP;
		else state = DOWN;
	}
	DEBUG('e', "Elevator received request, state=%d\n", state);
}

Building::Building(char* debugName, int numFloors, int numElevators)
{
	name = debugName;
	this->numFloors = numFloors;
	this->numElevators = numElevators;
	elevator = new Elevator("elevator", numFloors, 1);
	
	elevatorUp = new EventBarrier*[numFloors + 1];
	elevatorDown = new EventBarrier*[numFloors + 1];

	for(int i = 1; i <= numFloors; i++)
	{
		char *upName = new char[25];
		char *downName = new char[25];
		sprintf(upName, "up barrier for %dth floor", i);
		sprintf(downName, "down barrier for %dth floor", i);
		elevatorUp[i] = new EventBarrier(upName);
		elevatorDown[i] = new EventBarrier(downName);
	}
}

Building::~Building()
{
	delete elevator;
	delete[] elevatorUp;
	delete[] elevatorDown;
}

void Building::CallUp(int fromFloor)
{
	elevator->ReceiveRequest(fromFloor);
}

void Building::CallDown(int fromFloor)
{
	elevator->ReceiveRequest(fromFloor);
}

Elevator* Building::AwaitUp(int fromFloor)
{
	elevatorUp[fromFloor]->Wait();
	return elevator;
}

Elevator* Building::AwaitDown(int fromFloor)
{
	elevatorDown[fromFloor]->Wait();
	return elevator;
}

Elevator* Building::getElevator()
{
	return elevator;
}