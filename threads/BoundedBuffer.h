#include "synch.h"

class BoundedBuffer {
   public:
     // create a bounded buffer with a limit of 'maxsize' bytes
     BoundedBuffer(int maxsize);
     
     ~BoundedBuffer();

     // read 'size' bytes from the bounded buffer, storing into 'data'.
     // ('size' may be greater than 'maxsize')
     char Read();
     
     // write 'size' bytes from 'data' into the bounded buffer.
     // ('size' may be greater than 'maxsize')
     void Write(char x);

     int getCount();

   private:
     int maxSize;
     int count;
     int nextIn, nextOut;
     char *buffer;
     Condition *notEmpty, *notFull;
     Lock *lock;
};

