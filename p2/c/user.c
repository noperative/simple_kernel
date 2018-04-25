/* user.c : User processes
 */

#include <xeroskernel.h>
#include <xeroslib.h>

static int TESTCASE = 11;

void extended_producer( void ) {    
    int sleepTime;
    PID_t root = 2;
    kprintf("alive\n");
    syssleep(5000);
    sysrecv(&root, &sleepTime, sizeof(sleepTime));
    
    kprintf("got msg, sleeping for: %d\n", sleepTime);
    syssleep(sleepTime);
    kprintf("Finished sleeping, running off\n");
}

void sendToRoot( void ) {
    char buffer[8] = "testdata";

    syssend(2, &buffer, 8);
}

void receiveFromRoot( void ) {
    char buffer[8];

    PID_t root_pid = 2;

    sysrecv(&root_pid, &buffer, sizeof(buffer));
}

void dummy( void ) {
  int self = sysgetpid();
  kprintf("Im a dummy, time to kill myself\n");
  syskill( self );
}

 void timer1( void ) {
/****************************/
    while(1) {
        kprintf("O");
    }
}

 void timer2( void ) {
/****************************/
    while(1) {
        kprintf( "X");
    }
}

 void producer( void ) {
/****************************/

    int         i;

    for( i = 0; i < 5; i++ ) {
        kprintf( "Produce %d\n", i );
        sysyield();
    }

    sysstop();
}

 void consumer( void ) {
/****************************/

    int         i;

    for( i = 0; i < 5; i++ ) {
        kprintf( "Consume %d \n", i );
        sysyield();
    }

    sysstop();
}

 void     root( void ) {
/****************************/
   PID_t proc_pid, con_pid, send_pid, recv_pid, root_id, rand_id;
   int retval;

   kprintf("Root has been called\n");
   
   sysyield();
   sysyield();
   switch(TESTCASE){

    /* original test to make sure it works */
    case 0 :
     proc_pid = syscreate( &producer, 4096 );
     con_pid =  syscreate( &consumer, 4096 );

     kprintf("Proc pid = %u Con pid = %u\n", proc_pid, con_pid);
     break;
    
    /* Send Test 1 */
    case 1 :
        recv_pid = syscreate(&timer1, 4096);
        char buffer1[8] = "testdata";        

        kprintf("checkpoint 1\n");

        retval = syssend(recv_pid, &buffer1, sizeof(buffer1));

        kprintf("checkpoint 2\n");

        break;

    /* Send Test 2 */
    case 2 :
       recv_pid = syscreate(&receiveFromRoot, 4096);
       char buffer2[8] = "testdata";

       retval = syssend(recv_pid, &buffer2, sizeof(buffer2));

       if (retval == sizeof(buffer2)) {
          kprintf("Test Case 2: retval is %d\n", retval);
      }
      else {
          kprintf("test failed\n");
          kprintf("retval = %d\n", retval);
      }

      break;

    /* Receive Test 1 */
    case 3 :
       send_pid = syscreate(&timer1, 4096);
       char buffer3[8];

       kprintf("checkpoint 1\n");

       retval = sysrecv(&send_pid, &buffer3, sizeof(buffer3));

       kprintf("checkpoint 2\n");

       break;

    /* Receive Test 2 */
    case 4 :
      send_pid = syscreate(&sendToRoot, 4096);
      char buffer[8];

      retval = sysrecv(&send_pid, &buffer, sizeof(buffer));

      if (retval == sizeof(buffer)) {
          kprintf("Test Case 4: retval is %d\n", retval);
      }
      else {
          kprintf("test failed");
      }

      break;

    /* Send Failure */
    case 5 :
      root_id = sysgetpid();
      kprintf("Attempting to send to %d\n", root_id);
      retval = syssend(root_id, &root_id, sizeof(root_id));
      kprintf("Test Case 5: retval is %d\n", retval);
      break;

    /* Receive Failure 1 */
    case 6 :
      root_id = sysgetpid();
      kprintf("Attempting to recv from %d\n", root_id);
      retval = sysrecv(&root_id, &root_id, sizeof(root_id));
      kprintf("Test Case 6: retval is %d\n", retval);
      break;
    /* Receive Failure 2 */
    case 7 :
      rand_id = 39;
      kprintf("Attempting to recv from %d\n", rand_id);
      retval = sysrecv(&rand_id, &rand_id, sizeof(rand_id));
      kprintf("Test Case 7: retval is %d\n", retval);
      break;

    /*  Timeshare test  */
    case 8 :
      proc_pid = syscreate( &timer1, 4096 );
      con_pid =  syscreate( &timer2, 4096 );
      break;

    /* Kill Test 1 */
    case 9 :
      proc_pid = syscreate( &timer1, 4096 );
      sysyield();
      syskill(proc_pid);
      break;

    /* Kill Test 2 */
    case 10:
      proc_pid = syscreate( &dummy, 4096 );
      int dummyval;
      int retval = sysrecv(&proc_pid, &dummyval, sizeof(dummyval));
      kprintf("Test Case 10: retval is %d\n", retval);
      break;

    default:
      ;

      int self = sysgetpid();
      PID_t proc_pid1 = syscreate( &extended_producer, 4096 );
      kprintf("PID: %d created\n", proc_pid1);

      PID_t proc_pid2 =  syscreate( &extended_producer, 4096 );
      kprintf("PID: %d created\n", proc_pid2);

      PID_t proc_pid3 = syscreate( &extended_producer, 4096 );
      kprintf("PID: %d created\n", proc_pid3);

      PID_t proc_pid4 =  syscreate( &extended_producer, 4096 );
      kprintf("PID: %d created\n", proc_pid4);

      syssleep(4000); 

      int time3 = 10000;
      int time2 = 7000;
      int time1 = 20000;
      int time4 = 27000;

      syssend(proc_pid3, &time3, sizeof(time3));
      syssend(proc_pid2, &time2, sizeof(time2));
      syssend(proc_pid1, &time1, sizeof(time1));
      syssend(proc_pid4, &time4, sizeof(time4));

      int msg;
      int return4 = sysrecv(&proc_pid4, &msg, sizeof(msg));
      kprintf("Tried to recv from %d, got %d\n", proc_pid4, return4);
      int send3 = syssend(proc_pid3, &msg, sizeof(msg));
      kprintf("Tried to send to %d, got %d\n", proc_pid3, send3);
      syskill( self );
      break;
   }
 }

 void     idleproc( void ) {
/****************************/
  for(;;){
    __asm __volatile( " \
      hlt \n\
      "
      : 
      : 
    );
  }
}
