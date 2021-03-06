// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "dllist.h"
#include "BoundedBuffer.h"
#include "Table.h"
#include "Elevator.h"

// testnum,threadnum and nodenum are set in main.cc
int testnum = 1, threadnum = 1, nodenum = 1, errType = 0, sleepTime = 2, numFloors = 20, riderOperations = 5;
DLList *dllist;
Lock *listLock;
Condition *listCondition;
BoundedBuffer *bbuffer;
Table *table;
EventBarrier *eBarrier;
Building *buildingInstance;
Elevator *elevator;

extern void Insert(int t, int N, DLList* list), Remove(int t, int N, DLList* list);

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
OperateTable(int thread)
{
	for(int i = 1; i <= nodenum; i++)
	{
		char x = '0' + i;
		int index = table->Alloc((void*)(&x));
		if(index != -1)
		{
			printf("Thread:%d inserted %c\n", thread, x);
			char *item = (char*)table->Get(index);
			printf("Thread:%d got %c by index %d\n", thread, *item, index);
		}
	}
}

void
OperateList(int thread)
{
	listLock->Acquire();

	Insert(thread, nodenum, dllist);
	if(errType == 1)
	{
		dllist->Show();
		printf("Thread yield\n");
		currentThread->Yield();
	}
	Remove(thread, nodenum, dllist);

	listLock->Release();
}

void
KeepReading(int x)
{
	while(1)
	{
		printf("Reader is trying to read, count = %d\n", bbuffer->getCount());
		char temp = bbuffer->Read();
		printf("Reader read %c\n", temp);
	}
}

void
KeepWriting(int x)
{
	while(1)
	{
		printf("Writer is trying to write, count = %d\n", bbuffer->getCount());
		bbuffer->Write('1');
		printf("Writer wrote 1\n");
	}
}

void
testEventBarrier(int thread)
{
		printf("thread:%d is waiting to enter the elevator\n", thread);
		eBarrier->Wait();
		printf("thread:%d entered the elevator\n", thread);
		printf("thread:%d completed\n", thread);
		eBarrier->Complete();
}

void
testAlarm(int thread)
{
	printf("thread:%d sleeps for %d, now is %d\n", thread, sleepTime*TimerTicks, stats->totalTicks);
	myAlarm->Pause(sleepTime);
}

void 
rider(int id, int srcFloor, int dstFloor) 
{
	Elevator *e;

	if (srcFloor == dstFloor) return;

	DEBUG('e',"Rider %d travelling from %d to %d\n",id,srcFloor,dstFloor);
	do {
		if (srcFloor < dstFloor) 
		{
			DEBUG('e', "Rider %d CallUp(%d)\n", id, srcFloor);
			buildingInstance->CallUp(srcFloor);
			DEBUG('e', "Rider %d AwaitUp(%d)\n", id, srcFloor);
			e = buildingInstance->AwaitUp(srcFloor);
		} else
		{
			DEBUG('e', "Rider %d CallDown(%d)\n", id, srcFloor);
			buildingInstance->CallDown(srcFloor);
			DEBUG('e', "Rider %d AwaitDown(%d)\n", id, srcFloor);
			e = buildingInstance->AwaitDown(srcFloor);
		}
		DEBUG('e', "Rider %d Enter()\n", id);
	} while (!e->Enter()); // elevator might be full!

	DEBUG('e', "Rider %d RequestFloor(%d)\n", id, dstFloor);
	e->RequestFloor(dstFloor); // doesn't return until arrival
	DEBUG('e', "Rider %d Exit()\n", id);
	e->Exit();
	DEBUG('e', "Rider %d finished\n", id);
}

void
elevatorOperator(int dummy)
{
	buildingInstance->getElevator()->Run();
}

void
riderOperator(int id)
{
	RandomInit(id + (unsigned)time(NULL));
	int srcFloor = Random() % numFloors + 1;
	int dstFloor = Random() % numFloors + 1;
	rider(id, srcFloor, dstFloor);
}
//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1\n");

    table = new Table(5);

    for(int i = 1; i <= threadnum; i++)
     {
    	char *threadName = new char[10];
    	sprintf(threadName, "Thread:%d", i);
    	Thread *t = new Thread(threadName);
    	t->Fork(OperateTable, i);
     }
}

void
ThreadTest2()
{
	DEBUG('t', "Entering ThreadTest2\n");
	dllist = new DLList(errType);
	listLock = new Lock("listLock");
	listCondition = new Condition("listCondition");
	for(int i = 1; i < threadnum; i++)
	{
		char *threadName = new char[10];
		sprintf(threadName, "Thread:%d", i);
		Thread *t = new Thread(threadName);
		t->Fork(OperateList, i);
	}
	OperateList(threadnum);
}

void
ThreadTest3()
{
	DEBUG('t', "Entering ThreadTest3\n");
	bbuffer = new BoundedBuffer(5);
	Thread *reader = new Thread("reader");
	Thread *writer = new Thread("writer");
	reader->Fork(KeepReading,1);
	writer->Fork(KeepWriting,1);
}

void
ThreadTest4()
{
	DEBUG('t',"Entering ThreadTest4\n");
	eBarrier = new EventBarrier("EventBarrier");
	for(int i = 1; i <= threadnum; i++)
	{
		char *threadName = new char[10];
		sprintf(threadName, "Thread:%d", i);
		Thread *t = new Thread(threadName);
		t->Fork(testEventBarrier, i);
	}
	eBarrier->Signal();
}

void
ThreadTest5()
{
	DEBUG('t',"Entering ThreadTest5\n");
	for(int i = 1; i <= threadnum; i++)
	{
		char *threadName = new char[10];
		sprintf(threadName, "Thread:%d", i);
		Thread *t = new Thread(threadName);
		t->Fork(testAlarm, i);
	}
}

void
ThreadTest6()
{
	DEBUG('t', "Entering ThreadTest6\n");
	buildingInstance= new Building("building", 20,1);
	Thread *e = new Thread("Elevator");
	e->Fork(elevatorOperator, 1);
	for(int i = 1; i <= threadnum; i++)
	{
		char *riderName = new char[20];
		sprintf(riderName, "rider:%d", i);
		Thread *r = new Thread(riderName);
		r->Fork(riderOperator, i);
	}
}
//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
   switch (testnum) {
   	   case 1:
   		   ThreadTest1();
   		   break;
   	   case 2:
   		   ThreadTest2();
   		   break;
   	   case 3:
   		   ThreadTest3();
   		   break;
   	   case 4:
   		   ThreadTest4();
   		   break;
	   case 5:
	   	   ThreadTest5();
		   break;
	   case 6:
	   	   ThreadTest6();
		   break;
   	   default:
   		   printf("No test specified.\n");
   		   break;
    }
}

