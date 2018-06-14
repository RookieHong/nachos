#include "Elevator.h"
#include "system.h"

Elevator::Elevator(char* debugName, int numFloors, int myID)
{
	name = debugName;
	this->numFloors = numFloors;
	this->myID = myID;
	requests = new DLList();
	lock = new Lock("Elevator's lock");
	state = STOP;
	elevatorOut = new EventBarrier*[numFloors + 1];
	currentfloor = 1;
	occupancy = 0;

	for(int i = 1; i <= numFloors; i++)
	{
		char *outName = new Char[25];
		sprintf(outName, "out barrier for %dth floor", i);
		elevatorOut[i] = new EventBarrier(outName);
	}
}

Elevator::~Elevator()
{
	delete requests;
	delete lock;
	delete elevatorOut;
}

void Elevator::ReceiveRequest(int Floor, int direction)
{
	lock->Acquire();
	if(direction == UP)
		requests->SortedInsert(&UP, Floor);
	else
		requests->SortedInsert(&DOWN, Floor);
	lock->Release();
}

void Elevator::Enter()
{
       myAlarm->Pause(TIME_RIDER_ENTER);
	occupancy++;
	if(state == UP)
		buildingInstance->elevatorUp[currentfloor]->Complete();
	else if(state == DOWN)
		buildingInstance->elevatorDown[currentfloor]->Complete();
}

void Elevator::Exit()
{
	myAlarm->Pause(TIME_DOOR_OPEN);
	occupancy--;
	elevatorOut[currentfloor]->Complete();
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
	elevator->ReceiveRequest(fromFloor, UP);
}

void Building::CallDown(int fromFloor)
{
	elevator->ReceiveRequest(fromFloor, DOWN);
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