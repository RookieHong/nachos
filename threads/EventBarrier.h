#include "synch.h"
#include "system.h"

class EventBarrier {
	public:
		EventBarrier(char *debugName);
		~EventBarrier();

		void Wait(); //等电梯开门
		void Signal(); //开电梯的门
		void Complete(); //表示进入了电梯
		int Waiters();
		int Status();

	private:
		enum {SIGNALED, UNSIGNALED} status; //SIGNALED：电梯门开着， UNSIGNALED：电梯门关着
		int numWaiting; //每有一个人进电梯，就减一，为0时关电梯门
		Condition *signalCon;
		Condition *completeCon;
		Lock *lock;
		char *name;
};
