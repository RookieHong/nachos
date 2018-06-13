#include "dllist.h"

void Insert(int t, int N, DLList* list)
{
	srand((unsigned)time(NULL));
	for(int i = 0; i < N; i++)
	{
		int random = rand() % 101;
		list->SortedInsert(NULL, random);
		printf("Thread : %d inserted key = %d\n", t, random);
	}
}

void Remove(int t, int N, DLList* list)
{
	int keyPtr;
	for(int i = 0; i < N; i++)
	{
		list->Remove(&keyPtr);
		printf("Thread : %d removed key = %d\n", t, keyPtr);
	}
}
