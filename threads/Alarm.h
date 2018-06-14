#include "synch.h"

class Alarm {
	public:
		Alarm();
		~Alarm();
		void Pause(int howlong);
		void Wakeup();
		void check();

	private:
		List* queue;
		int pauseNum;
};
