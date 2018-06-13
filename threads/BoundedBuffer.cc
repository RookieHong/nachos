#include "BoundedBuffer.h"

BoundedBuffer::BoundedBuffer(int maxSize)
{
	this->maxSize = maxSize;
	count = 0;
	nextIn = nextOut = 0;
	buffer = new char[maxSize];
	notEmpty = new Condition("notEmpty");
	notFull = new Condition("notFull");
	lock = new Lock("BoundedBuffer's lock");
}

BoundedBuffer::~BoundedBuffer()
{
	delete buffer;
	delete notEmpty;
	delete notFull;
	delete lock;
}

char BoundedBuffer::Read()
{
	lock->Acquire();
	while(count == 0) notEmpty->Wait(lock);
	char x = buffer[nextOut];
	nextOut = (nextOut + 1) % maxSize;
	count--;
	notFull->Signal(lock);
	lock->Release();
	return x;
}

void BoundedBuffer::Write(char x)
{
	lock->Acquire();
	while(count == maxSize) notFull->Wait(lock);
	buffer[nextIn] = x;
	nextIn = (nextIn + 1) % maxSize;
	count++;
	notEmpty->Signal(lock);
	lock->Release();
}

int BoundedBuffer::getCount()
{
	return count;
}
