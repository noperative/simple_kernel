/* user.c : User processes
 */

#include <xeroskernel.h>
#include <xeroslib.h>

void t( void );
void alarm( void );
void alarm_handler( void* cntx);

void busy( void ) {
  int myPid;
  char buff[100];
  int i;
  int count = 0;

  myPid = sysgetpid();
  
  for (i = 0; i < 10; i++) {
    sprintf(buff, "My pid is %d\n", myPid);
    sysputs(buff);
    //if (myPid == 2 && count == 1) //syskill(3);
    count++;
    sysyield();
  }
}



void sleep1( void ) {
  int myPid;
  char buff[100];

  myPid = sysgetpid();
  sprintf(buff, "Sleeping 1000 is %d\n", myPid);
  sysputs(buff);
  syssleep(1000);
  sprintf(buff, "Awoke 1000 from my nap %d\n", myPid);
  sysputs(buff);
}



void sleep2( void ) {
  int myPid;
  char buff[100];

  myPid = sysgetpid();
  sprintf(buff, "Sleeping 2000 pid is %d\n", myPid);
  sysputs(buff);
  syssleep(2000);
  sprintf(buff, "Awoke 2000 from my nap %d\n", myPid);
  sysputs(buff);
}



void sleep3( void ) {
  int myPid;
  char buff[100];

  myPid = sysgetpid();
  sprintf(buff, "Sleeping 3000 pid is %d\n", myPid);
  sysputs(buff);
  syssleep(3000);
  sprintf(buff, "Awoke 3000 from my nap %d\n", myPid);
  sysputs(buff);
}








void producer( void ) {
/****************************/

    int         i;
    char        buff[100];


    // Sping to get some cpu time
    for(i = 0; i < 100000; i++);

    syssleep(3000);
    for( i = 0; i < 20; i++ ) {
      
      sprintf(buff, "Producer %x and in hex %x %d\n", i+1, i, i+1);
      sysputs(buff);
      syssleep(1500);

    }
    for (i = 0; i < 15; i++) {
      sysputs("P");
      syssleep(1500);
    }
    sprintf(buff, "Producer finished\n");
    sysputs( buff );
    sysstop();
}

void consumer( void ) {
/****************************/

    int         i;
    char        buff[100];

    for(i = 0; i < 50000; i++);
    syssleep(3000);
    for( i = 0; i < 10; i++ ) {
      sprintf(buff, "Consumer %d\n", i);
      sysputs( buff );
      syssleep(1500);
      sysyield();
    }

    for (i = 0; i < 40; i++) {
      sysputs("C");
      syssleep(700);
    }

    sprintf(buff, "Consumer finished\n");
    sysputs( buff );
    sysstop();
}

// void     oldroot( void ) {
// /****************************/

//     char  buff[100];
//     int proc_pid, con_pid;
//     int i;

//     sysputs("Root has been called\n");


//     // Test for ready queue removal. 
   
//     proc_pid = syscreate(&busy, 1024);
//     con_pid = syscreate(&busy, 1024);
//     sysyield();
//     //syskill(proc_pid);
//     sysyield();
//     //syskill(con_pid);

    
//     for(i = 0; i < 5; i++) {
//       pids[i] = syscreate(&busy, 1024);
//     }

//     sysyield();
    
//     //syskill(pids[3]);
//     sysyield();
//     //syskill(pids[2]);
//     //syskill(pids[4]);
//     sysyield();
//     //syskill(pids[0]);
//     sysyield();
//     //syskill(pids[1]);
//     sysyield();

//     syssleep(8000);;



//     kprintf("***********Sleeping no kills *****\n");
//     // Now test for sleeping processes
//     pids[0] = syscreate(&sleep1, 1024);
//     pids[1] = syscreate(&sleep2, 1024);
//     pids[2] = syscreate(&sleep3, 1024);

//     sysyield();
//     syssleep(8000);;



//     kprintf("***********Sleeping kill 2000 *****\n");
//     // Now test for removing middle sleeping processes
//     pids[0] = syscreate(&sleep1, 1024);
//     pids[1] = syscreate(&sleep2, 1024);
//     pids[2] = syscreate(&sleep3, 1024);

//     syssleep(110);
//     //syskill(pids[1]);
//     syssleep(8000);;

//     kprintf("***********Sleeping kill last 3000 *****\n");
//     // Now test for removing last sleeping processes
//     pids[0] = syscreate(&sleep1, 1024);
//     pids[1] = syscreate(&sleep2, 1024);
//     pids[2] = syscreate(&sleep3, 1024);

//     sysyield();
//     //syskill(pids[2]);
//     syssleep(8000);;

//     kprintf("***********Sleeping kill first process 1000*****\n");
//     // Now test for first sleeping processes
//     pids[0] = syscreate(&sleep1, 1024);
//     pids[1] = syscreate(&sleep2, 1024);
//     pids[2] = syscreate(&sleep3, 1024);

//     syssleep(100);
//     //syskill(pids[0]);
//     syssleep(8000);;

//     // Now test for 1 process


//     kprintf("***********One sleeping process, killed***\n");
//     pids[0] = syscreate(&sleep2, 1024);

//     sysyield();
//     //syskill(pids[0]);
//     syssleep(8000);;

//     kprintf("***********One sleeping process, not killed***\n");
//     pids[0] = syscreate(&sleep2, 1024);

//     sysyield();
//     syssleep(8000);;



//     kprintf("***********Three sleeping processes***\n");    // 
//     pids[0] = syscreate(&sleep1, 1024);
//     pids[1] = syscreate(&sleep2, 1024);
//     pids[2] = syscreate(&sleep3, 1024);


//     // Producer and consumer started too
//     proc_pid = syscreate( &producer, 4096 );
//     con_pid = syscreate( &consumer, 4096 );
//     sprintf(buff, "Proc pid = %d Con pid = %d\n", proc_pid, con_pid);
//     sysputs( buff );


//     processStatuses psTab;
//     int procs;
    



//     syssleep(500);
//     procs = sysgetcputimes(&psTab);

//     for(int j = 0; j <= procs; j++) {
//       sprintf(buff, "%4d    %4d    %10d\n", psTab.pid[j], psTab.status[j], 
// 	      psTab.cpuTime[j]);
//       kprintf(buff);
//     }


//     syssleep(10000);
//     procs = sysgetcputimes(&psTab);

//     for(int j = 0; j <= procs; j++) {
//       sprintf(buff, "%4d    %4d    %10d\n", psTab.pid[j], psTab.status[j], 
// 	      psTab.cpuTime[j]);
//       kprintf(buff);
//     }

//     sprintf(buff, "Root finished\n");
//     sysputs( buff );
//     sysstop();
    
//     for( ;; ) {
//      sysyield();
//     }
    
// }

int shellpid;
void root(void){

    int fd;
    char str[512];
    char username[5];
    char password[15];
    char* validUsername = "cs415";
    char* validPassword = "EveryoneGetsAnA";
    while(1){
        memset( &username, 0, 20 );
        memset( &password, 0, 20 );
        // 1. Prints a banner that says Welcome to Xeros - an experimental OS
        sprintf(str, "Welcome to Xeros - an experimental OS\n");
        sysputs(str);
        // 2. Opens the keyboard.
        fd = sysopen(1);
        // 3. Prints Username:
        sprintf(str, "Username: \n");
        sysputs(str);
        // 4. Reads the username - the only username you need to support is cs415
        sysread(0, username, 5);
        // 5. Turns keyboard echoing of
        sysioctl(fd, 55, 0);
        // 6. Prints Password:
        sprintf(str, "\nPassword: \n");
        sysputs(str);
        // 7. Reads the password - password is EveryoneGetsAnA
        sysread(0, password, 15);
        // 8. Closes the keyboard.
        sysclose(fd);
        // 9. Verifies the username and password
        //10. If verification fails go back to step 1
        if ( strncmp( username, validUsername, 5 ) != 0 ) {
          sysputs("Username invalid\n");
          //kprintf( "\n%s vs %s", username, validUsername );
          continue;
        }

        if ( strncmp( password, validPassword, 15) != 0 ) {
          sysputs("Password invalid\n");
          //kprintf( "\n%s vs %s", password, validPassword );
          continue;
        }
        // 11. Create the shell program.
        // 12. Wait for the shell program to exit.
        shellpid = syscreate(&shell, PROC_STACK);
        syswait(shellpid);
        // 13. Go back to step 1
    }
}

int alarm_ticks;

void shell( void ) {
  int ret, pid;
  char* command_k  = "k";
  char* command_ex = "ex";
  char* command_a  = "a";
  char* command_ps = "ps";
  char* command_t  = "t";

  int proc_index = 0;
  int proc[10];
  char input[256];
  int fd = sysopen(1);
  //shell loop
  while ( 1 ) {
    sysputs("\n> ");
    ret = sysread(fd, &input, 256);

    if (ret > 0){
      if (input[ret-2] == ';') break;
      char* current = input;

      // parsing loop
      while (1){
        //kprintf("The command input is: %s\n", current);
        while(*current == ' '){
          current++;
        }
        /***************
        | Print Process status
        ****************/
        if (!( strncmp (current, command_ps, 2) )){
          processStatuses ps_tab;
          int procs, j;
          char buff[500], status[30];
          procs = sysgetcputimes(&ps_tab);

          sysputs("\n PID      State          Time\n");
          for(j = 0; j <= procs; j++) {

            switch (ps_tab.status[j]) {
              case ( STATE_READY ):
                sprintf(status, "%s", "  READY");
                break;            

              case ( STATE_SLEEP ):
                sprintf(status, "%s", " ASLEEP");
                break;   

              case ( STATE_RUNNING ):
                sprintf(status, "%s", "RUNNING");
                break;            
              case ( STATE_WAITING ):
                sprintf(status, "%s", "WAITING");
                break;
              default:
                sprintf(status, "%s", "UNKNOWN");
            }

            sprintf(buff, "%4d    %s    %10d\n", ps_tab.pid[j], status, 
             ps_tab.cpuTime[j]);
            kprintf(buff);
          }
          //adjust pointer to read next
          current += 2;
          while(*current == ' '){
            current++;
          }
          ///
        }

        /***************
        | Exit
        ****************/

        else if (!( strncmp (current, command_ex, 2) )) break;
        else if (*current == ';') break;

        /***************
        | Kill Process
        ****************/
        else if (!( strncmp (current, command_k, 1) )){
          if (ret >= 3){
            current += 2;
            int pid = atoi(current);
            kprintf("Killing %d\n", pid);
            int kill = syskill(pid, 31);
            if (kill){
              sysputs("No such process.\n");
            }

          } else {
            sysputs("incorrect argument for k");
          }

          //adjust pointer to read next
          while(*current != ' ' && *current != '&'){
            current++;
          }
          while(*current == ' '){
            current++;
          }
          ///

        }

        /***************
        | Alarm
        ****************/

        else if (!( strncmp (current, command_a, 1) )){
          current += 2;
          void (*oldhandler)(void *);
          syssighandler(15, &alarm_handler, &oldhandler);
          alarm_ticks = atoi(current);
          pid = syscreate(&alarm, PROC_STACK);

          proc[proc_index++] = pid;


          //adjust pointer to read next
          while(*current != ' ' && *current != '&'){
            current++;
          }
          while(*current == ' '){
            current++;
          }
          ///
        }

        /***************
        | Timer
        ****************/
        else if (!( strncmp (current, command_t, 1) )){
          //kprintf("making new process then sleeping");
          pid = syscreate(&t, PROC_STACK);

          proc[proc_index++] = pid;

          current += 1;
          while(*current == ' '){
            current++;
          }
        }

        else if (*current != ' ' || *current != '&'){
          sysputs("Command not found\n");
        }

        if (*current != '&') break;
        current += 1;
      }

      // Wait on all the processes
      while ( proc_index > 0 ) {
        int r = syswait(proc[proc_index]);

        if ( r == 0 || r == -1 ) proc_index--; 
      }
      
      if (input[ret-2] != '&') break;
    }
  }
  sysclose(fd);
  sysputs("Shell Exiting.\n");
}


void alarm_handler( void *cntx ) {
  void (*oldhandler)(void *);
  sysputs("ALARM ALARM ALARM\n");
  syssighandler(15, NULL, &oldhandler);
}
void alarm( void ) {
  //sysputs("setting alarm..\n");
  syssleep( alarm_ticks * MILLISECONDS_TICK );
  //sysputs("sending signal 15\n");
  syskill( shellpid, 15 );
}
void t( void ) {
  while ( 1 ) {
    syssleep(10000);
    sysputs("\nT\n");
  }
}
/*******************
*   TEST BLOCK
*
********************/

void print_handler1( void* cntx ){
  int pid;
  pid = sysgetpid();
  kprintf("Handler 1 Running\n");
  syskill(pid, 20);
  sysyield();
  kprintf("Handler 1 Leaving\n");
}

void print_handler2( void* cntx ){
  kprintf("Handler 2 Running\n");
  sysyield();
  kprintf("Handler 2 Leaving\n");
}

void test1( void ){
  int pid;
  void(*oldhandler)(void *);
  kprintf("Starting Test 1\n");
  pid = sysgetpid();
  kprintf("PID: %d\n", pid);
  syssighandler(10, print_handler1, &oldhandler);
  syssighandler(20, print_handler2, &oldhandler);
  kprintf("Handlers set, signaling\n");
  syskill(pid, 10);
  sysyield();
  kprintf("Test 1 done");
}

void test2( void ){
  void(*oldhandler)(void *);
  kprintf("Starting Test 2\n");
  kprintf("Address of handler1: %d\n", print_handler1);
  syssighandler(10, print_handler1, &oldhandler);
  syssighandler(10, print_handler2, &oldhandler);
  kprintf("Address in old handler: %d\n", oldhandler);
  kprintf("Test 2 done");
}

void test3( void ){
  int pid;
  void(*oldhandler)(void *);
  kprintf("Starting Test 3\n");
  pid = sysgetpid();
  kprintf("PID: %d\n", pid);
  syssighandler(20, print_handler2, &oldhandler);
  kprintf("Handlers set, signaling\n");
  syskill(pid, 20);
  sysyield();
  kprintf("Test 3 done");
}

void child1( void ){    
  char  buff[100];
  sprintf(buff, "Child1::waiting...\n");
  sysputs(buff);
  syswait(1);
  kprintf("Parent terminated, now I'm up!\n");
  kprintf("Test 4 done\n");
}
void test4( void ){
  kprintf("Starting Test 4\n");
  syscreate(&child1, 4096);
  sysyield();
  sysyield();
  kprintf("test4 parent ending...\n");
}

void infinite_loop( void ){
  for(;;){
    kprintf("L");
  }
}

void testA( void ){
  kprintf("Starting Test A\n");
  syscreate(&infinite_loop, 4096);
  sysyield();
  syskill(2, 31);
  sysyield();
  kprintf("Test A done\n");
}

void test5( void ){
  kprintf("Starting Test 5\n");
  int fd = sysopen(5);
  kprintf("FD is %d\n", fd);
  kprintf("Test 5 done\n");
}

void test6( void ){
  kprintf("Starting Test 6\n");
  int result;
  char str[256];
  int bufflen = 1;
  unsigned char  testval = 1;
  unsigned char* buff = &testval;

  int fd = sysopen(0);
  sprintf(str, "Opened keyboard, FD: %d\n", fd);
  sysputs(str);
  result = syswrite(fd, (void*) buff, bufflen);
  sprintf(str, "Result: %d\n", result);
  sysputs(str);
  kprintf("Test 6 done\n");
}

void test7( void ){
  kprintf("Starting Test 7\n");
  int result;
  char str[256];

  int fd = sysopen(0);
  sprintf(str, "Opened keyboard, FD: %d\n", fd);
  sysputs(str);
  result = sysioctl(fd, 20, 0);
  sprintf(str, "Result: %d\n", result);
  sysputs(str);
  kprintf("Test 7 done\n");
}

void test8( void ){
  kprintf("Starting Test 8\n");
  int result;
  char str[256];
  int input[4];

  int fd = sysopen(1);
  sprintf(str, "Opened echo keyboard, FD: %d\n", fd);
  sysputs(str);
  sprintf(str, "Before I wake up, type 2+ characters manually\n");
  sysputs(str);
  syssleep(10000);
  sprintf(str, "\nI'm up... printing\n");
  sysputs(str);
  result = sysread(fd, &input, 1);
  int i;
  for(i = 0; i < result; i++) {
    sprintf( (char *)str, "%c", input[i]);
    sysputs( (char *)str );
  }
  sysputs("\n");
  sysclose(fd);
  kprintf("Test 8 done\n");
}


void testB( void ){
  kprintf("Starting Test B\n");
  int result;
  char str[256];
  char input[4];

  int fd = sysopen(1);
  sprintf(str, "Opened echo keyboard, FD: %d\n", fd);
  sysputs(str);
  result = sysioctl(fd, 53, 50);
  sysputs("Please type '1234'\n");
  result = sysread(fd, &input, 4);
  sprintf(str, "\nRead only: %d characters\n", result);
  sysputs(str);
  sysclose(fd);
  kprintf("Test B done\n");
}
