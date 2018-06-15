#include "system.h"

Alarm::Alarm()
{
	queue = new List();
}

Alarm::~Alarm()
{
	delete queue;
}

void Alarm::check()
{
	while(pauseNum > 0)
	{
		currentThread->Yield();
	}
	currentThread->Finish();
}

static void AlarmHandler(int arg)
{
	Alarm* p = (Alarm*)arg;
	p->check();
}

void Alarm::Pause(int howlong)
{
	pauseNum++;
	if(pauseNum <= 1)
	{
		Thread* t = new Thread("check Thread");
		t->Fork(AlarmHandler, (int)this);
	}
	IntStatus oldlevel= interrupt->SetLevel(IntOff);
	int wakeTime = stats->totalTicks + TimerTicks*howlong;
	queue->SortedInsert((void*)currentThread, wakeTime);
	currentThread->Sleep();
	(void)interrupt->SetLevel(oldlevel);
}

void Alarm::Wakeup()
{
	int wakeTime;
	IntStatus oldlevel = interrupt->SetLevel(IntOff);
	Thread* thread = (Thread*)queue->SortedRemove(&wakeTime);
	while(thread != NULL)
	{
		if(stats->totalTicks >= wakeTime)
		{
			//DEBUG('e',"%s wakes up, now is %d\n", thread->getName(), stats->totalTicks);
			pauseNum--;
			scheduler->ReadyToRun(thread);
			thread = (Thread*)queue->SortedRemove(&wakeTime);
		}
		else
		{
			queue->SortedInsert((void*)thread, wakeTime);
			break;
		}
	}
	(void)interrupt->SetLevel(oldlevel);
}

