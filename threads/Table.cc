#include "Table.h"

Table::Table(int size)
{
	this->size = size;
	lock = new Lock("TableLock");
	table = new void *[size + 1]();
}

Table::~Table()
{
	delete[] table;
}

int Table::Alloc(void* object)
{
	lock->Acquire();
	int toRet = -1;
	for(int i = 1; i <= size; i++)
	{
		if(table[i] == NULL)
		{
			table[i] = object;
			toRet = i;
			break;
		}
	}
	lock->Release();
	return toRet;
}

void* Table::Get(int index)
{
	ASSERT(index >= 1 && index <= size);
	return table[index];
}

void Table::Release(int index)
{
	ASSERT(index >= 1 && index <= size);
	table[index] = NULL;
}
