#include "EventBarrier.h"

EventBarrier::EventBarrier(char *debugName)
{
	this->name = debugName;
	numWaiting = 0;
	signalCon = new Condition("Signal Condition");
	completeCon = new Condition("Complete Condition");
	lock = new Lock("EventBarrier's lock");
	status = UNSIGNALED;
}

EventBarrier::~EventBarrier()
{
	delete signalCon;
	delete completeCon;
	delete lock;
}

void EventBarrier::Wait()
{
	lock->Acquire();
	numWaiting++;
	if(status == SIGNALED)
	{
		lock->Release();
		return;
	}
	signalCon->Wait(lock);
	lock->Release();
}

void EventBarrier::Signal()
{
	lock->Acquire();
	status = SIGNALED;
	signalCon->Broadcast(lock);
	completeCon->Wait(lock);
	status = UNSIGNALED;
	lock->Release();
}

void EventBarrier::Complete()
{
	lock->Acquire();
	numWaiting--;
	if(numWaiting == 0)
	{
		printf("%s is the last to call complete, elevator is leaving\n", currentThread->getName());
		completeCon->Broadcast(lock);
	}
	else
	{
		completeCon->Wait(lock);
	}
	lock->Release();		
}		

int EventBarrier::Waiters()
{
	return numWaiting;
}

int EventBarrier::Status()
{
	return status;
}
