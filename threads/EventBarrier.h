#include "synch.h"
#include "system.h"

class EventBarrier {
	public:
		EventBarrier(char *debugName);
		~EventBarrier();

		void Wait(); 
		void Signal(); 
		void Complete();
		int Waiters();
		int Status();

	private:
		enum {SIGNALED, UNSIGNALED} status; 
		int numWaiting; 
		Condition *signalCon;
		Condition *completeCon;
		Lock *lock;
		char *name;
};
