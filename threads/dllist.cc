#include "dllist.h"
#include "system.h"

DLLElement::DLLElement( void *itemPtr, int sortKey )
{
	item = itemPtr;
	key = sortKey;
	next = NULL;
	prev = NULL;
}

DLList::DLList( )
{
	first = NULL;
	last = NULL;
}

DLList::DLList(int errType)
{
	first = NULL;
	last = NULL;
	this->errType = errType;
}

DLList::~DLList()
{
	while(Remove(NULL) != NULL);
}

void DLList::Prepend( void *item )
{
	DLLElement *temp = new DLLElement( item, DEFAULT_KEY );

	if(IsEmpty())
	{
		first = temp;
		last = temp;
	}
	else
	{
		temp->key = first->key - 1;
		temp->next = first;
		first->prev = temp;
		first = temp;
	}
}

void DLList::Append( void *item )
{
	DLLElement *temp = new DLLElement( item, DEFAULT_KEY );

	if(IsEmpty())
	{
		first = temp;
		last = temp;
	}
	else
	{
		temp->key = last->key + 1;
		temp->prev = last;
		last->next = temp;
		last = temp;
	}
}	

void* DLList::Remove( int *keyPtr )
{
	if(IsEmpty()) return NULL;

	DLLElement *temp = first;
	*keyPtr = temp->key;
	void *toRet = temp->item;

	if(this->errType == 3)
	{
		this->Show();
		printf("Thread Yield\n");
		currentThread->Yield();
	}

	if(first == last)
	{
		first = NULL;
		last = NULL;
	}
	else
	{
		first = first->next;
		first->prev = NULL;
	}

	delete temp;
	return toRet;
}

bool DLList::IsEmpty()
{
	if( first == NULL) return true;
	else return false;
}

void DLList::SortedInsert( void *item, int sortKey )
{
	DLLElement* toInsert = new DLLElement( item, sortKey );
	DLLElement *temp = first;
	if(IsEmpty())
	{
		first = toInsert;
		last = toInsert;
	}
	else
	{
		for( ;temp != NULL && toInsert->key > temp->key; temp = temp->next );
		if(this->errType == 2)
		{
			this->Show();
			printf("Thread yield\n");
			currentThread->Yield();
		}
		toInsert->next = temp;
		if( temp != NULL )
		{
			if( temp == first )
			{
				first->prev = toInsert;
				first = toInsert;
			}
			else
			{
				toInsert->prev = temp->prev;
				toInsert->next = temp;
				temp->prev->next = toInsert;
				temp->prev = toInsert;
			}
		}
		else
		{
			toInsert->prev = last;
			last->next = toInsert;
			last = toInsert;
		}
	}
}

void* DLList::SortedRemove(int sortKey)
{
	if(IsEmpty()) return NULL;

	DLLElement* temp = first;
	for( ;temp != NULL && temp->key != sortKey; temp = temp->next);

	if(temp == NULL) return NULL;
	else
	{
		if(temp == first || temp == last)
		{
			if(temp == first)
			{
				first = first->next;
				if(first != NULL) first->prev = NULL;
			}
			if(temp == last)
			{
				last = last->prev;
				if(last != NULL) last->next = NULL;
			}
		}
		else
		{
			temp->prev->next = temp->next;
			temp->next->prev = temp->prev;
		}
	}

	return temp->item;
}

void DLList::Show()
{
	DLLElement *temp = first;
	if(temp == NULL) return;
	for(; temp != NULL; temp = temp->next)
	{
		printf("%d->", temp->key);
	}
	printf("\n");
}
