/* disp.c : dispatcher
 */

#include <xeroskernel.h>
#include <i386.h>
#include <xeroslib.h>

static int  kill(pcb *currP, int pid, int sig);
int checkValidAddr(void* addr);
int wait(int pid, pcb* p);

static pcb      *head = NULL;
static pcb      *tail = NULL;

void     dispatch( void ) {
/********************************/

    pcb         *p;
    int         r;
    funcptr     fp;
    int         stack;
    va_list     ap;
    char        *str;
    int         len;
    int         sig;
    void        *newhandler;
    void        **oldhandler;
    unsigned int pid;
    void        *old_stack;
    int          deviceno, fd, bufflen;
    void*        buff;

    for( p = next(); p; ) {
      //      kprintf("Process %x selected stck %x\n", p, p->esp);
      //If the process has signals pending then setup the sigtramp
      //right before you context switch to it
      //if there is a higher priority signal to service then
      //build a tramp for it
      if( p->sigserv > p->sigmask){
        //kprintf("Signal pending, building tramp\n");
        build_tramp(p);
        //kprintf("mask: %u, serv: %u\n", p->sigmask, p->sigserv);
      }
      //kprintf("Context Switch to PID: %d\n", p->pid);
      r = contextswitch( p );
      switch( r ) {
      case( SYS_CREATE ):
        ap = (va_list)p->args;
        fp = (funcptr)(va_arg( ap, int ) );
        stack = va_arg( ap, int );
	      p->ret = create( fp, stack );
        break;
      case( SYS_YIELD ):
        ready( p );
        p = next();
        break;
      case( SYS_STOP ):
        stop(p);
        p = next();
        break;
      case ( SYS_KILL ):
        ap = (va_list)p->args;
        pid = va_arg(ap, int);
        sig = va_arg(ap, int);
        p->ret = kill(p, pid, sig);
        break;

      case (SYS_CPUTIMES):
      	ap = (va_list) p->args;
      	p->ret = getCPUtimes(p, va_arg(ap, processStatuses *));
      	break;
      case( SYS_PUTS ):
    	  ap = (va_list)p->args;
    	  str = va_arg( ap, char * );
    	  kprintf( "%s", str );
    	  p->ret = 0;
    	  break;
      case( SYS_GETPID ):
      	p->ret = p->pid;
      	break;
      case( SYS_SLEEP ):
      	ap = (va_list)p->args;
      	len = va_arg( ap, int );
      	sleep( p, len );
      	p = next();
      	break;
      case( SYS_TIMER ):
      	tick();
      	//kprintf("T");
      	p->cpuTime++;
      	ready( p );
      	p = next();
      	end_of_intr();
      	break;

      case( SYS_SIG_HANDLER ):
        ap = (va_list)p->args;
        sig = va_arg(ap, int);
        newhandler = va_arg(ap, void*);
        oldhandler = va_arg(ap, void**);

        if (sig < 0 || sig > 30) {
            p->ret = -1;
        }
        else if (!checkValidAddr(newhandler)) {
            p->ret = -2;
        }
        else {
            *oldhandler = p->sigtable[sig];
            p->sigtable[sig] = newhandler;
            p->ret = 0;
        }
        break;

      case( SYS_SIGRETURN ):                
        ap = (va_list) p->args;
        old_stack = va_arg(ap, void*);
        p->esp = old_stack;
        sig_context* sig_stack = (sig_context*)(p->esp - sizeof(sig_context));
        context_frame *cf = (context_frame *)p->esp;
        cf->esp = *((unsigned long *) old_stack);
        p->ret = sig_stack->old_ret;
        p->sigmask &= ~(1 << sig_stack->signal); // clear it from sigmask
        //kprintf("old ret %d, signal no: %d\n", sig_stack->old_ret, sig_stack->signal);
        //kprintf("Status of the sig masks are: %u for mask, %u for serv\n", p->sigmask, p->sigserv);
        break;

      case( SYS_WAIT ):                
        ap = (va_list)p->args;
        pid = va_arg(ap, unsigned int);
        p->ret = wait(pid, p);
        p = next();
        break;

     case( SYS_OPEN ):
        ap = (va_list)p->args;
        deviceno = va_arg(ap, int);
        p->ret = di_open(p, deviceno);
        break;

      case( SYS_CLOSE ):
        ap = (va_list)p->args;
        fd = va_arg(ap, int);
        p->ret = di_close(p, fd);
        break;

      case( SYS_READ ):
        ap = (va_list)p->args;
        fd = va_arg(ap, int);
        buff = va_arg(ap, void*);
        bufflen = va_arg(ap, int);
        p->ret = di_read(p, fd, buff, bufflen);
        if (p->ret >= 0) p = next();
        break;

      case( SYS_WRITE ):
        ap = (va_list)p->args;
        fd = va_arg(ap, int);
        buff = va_arg(ap, void*);
        bufflen = va_arg(ap, int);
        p->ret = di_write(p, fd, buff, bufflen);
        break;

      case( SYS_IOCTL ):
        ap = (va_list)p->args;
        fd = va_arg(ap, int);
        unsigned long command = va_arg(ap, unsigned long);
        va_list vargs = va_arg(ap, va_list);
        p->ret = di_ioctl(p, fd, command, vargs);
        break;

      case( SYS_KEYBOARD ):
        kbd_int();
        end_of_intr();
        break;

      default:
        kprintf( "Bad Sys request %d, pid = %d\n", r, p->pid );
      }
    }

    kprintf( "Out of processes: dying\n" );
    
    for( ;; );
}

extern void dispatchinit( void ) {
/********************************/

  //bzero( proctab, sizeof( pcb ) * MAX_PROC );
  memset(proctab, 0, sizeof( pcb ) * MAX_PROC);
}



extern void     ready( pcb *p ) {
/*******************************/

    p->next = NULL;
    p->state = STATE_READY;

    if( tail ) {
        tail->next = p;
    } else {
        head = p;
    }

    tail = p;
}

extern pcb      *next( void ) {
/*****************************/

    pcb *p;

    p = head;

    if( p ) {
        head = p->next;
        if( !head ) {
            tail = NULL;
        }
    } else { // Nothing on the ready Q and there should at least be the idle proc.
        kprintf( "BAD\n" );
        for(;;);
    }

    p->next = NULL;
    p->prev = NULL;
    return( p );
}


extern pcb *findPCB( int pid ) {
/******************************/

    int	i;

    for( i = 0; i < MAX_PROC; i++ ) {
        if( proctab[i].pid == pid ) {
            return( &proctab[i] );
        }
    }

    return( NULL );
}


// This function takes a pointer to the pcbtab entry of the currently active process. 
// The functions purpose is to remove the process being pointed to from the ready Q
// A similar function exists for the management of the sleep Q. Things should be re-factored to 
// eliminate the duplication of code if possible. There are some challenges to that because
// the sleepQ is a delta list and something more than just removing an element in a list
// is being preformed. 


void removeFromReady(pcb * p) {

  if (!head) {
    kprintf("Ready queue corrupt, empty when it shouldn't be\n");
    return;
  }

  if (head == p) { // At front of list
    // kprintf("Pid %d is at front of list\n", p->pid);
    head = p->next;

    // If the implementation has idle on the ready list this next statement
    // isn't needed. However, it is left just in case someone decides to 
    // change things so that the idle process is kept separate. 

    if (head == NULL) { // If the implementation has idle process  on the 
      tail = head;      // ready list this should never happen
      kprintf("Kernel bug: Where is the idle process\n");
    }
  } else {  // Not at front, find the process.
    pcb * prev = head;
    pcb * curr;
    
    for (curr = head->next; curr!=NULL; curr = curr->next) {
      if (curr == p) { // Found process so remove it
	// kprintf("Found %d in list, removing\n", curr->pid);
	prev->next = p->next;
	if (tail == p) { //last element in list
	    tail = prev;
	    // kprintf("Last element\n");
	}
	p->next = NULL; // just to clean things up
	break;
      }
      prev = curr;
    }
    if (curr == NULL) {
      kprintf("Kernel bug: Ready queue corrupt, process %d claimed on queue and not found\n",
	      p->pid);
      
    }
  }
}

// This function takes 3 paramenters:
//  currP  - a pointer into the pcbtab that identifies the currently running process
//  pid    - the proces ID of the process to be killed.
//  signal - a signal number to be sent
// Note: this function needs to be augmented so that it delivers a kill signal to a 
//       a particular process. The main functionality of the this routine will remain the 
//       same except that when the process is located it needs to be put onto the readyq
//       and a signal needs to be marked for delivery. 
//

static int  kill(pcb *currP, int pid, int sig) {
  pcb * targetPCB;
  int r;
  
  //kprintf("Current pid %d Killing %d\n", currP->pid, pid);

  // Don't let it kill the idle process, which from the user side
  // of things isn't a real process
  // IDLE process had PID 0

  if (pid == 0) {
    return -512;
  }
    
  if (!(targetPCB = findPCB( pid ))) {
    // kprintf("Target pid not found\n");
    return -512;
  }

  if (targetPCB->state == STATE_STOPPED) {
    kprintf("Target pid was stopped\n");
    return  -512;
  }
  
  // PCB has been found,  and the proces is either sleepign or running.
  // based on that information remove the process from 
  // the appropriate queue/list.

  // Check other states and do state specific cleanup before stopping
  // the process 
  // In the new version the process will not be marked as stopped but be 
  // put onto the readyq and a signal marked for delivery. 

  r = signal(targetPCB, sig);

  if (r == SIG_PROC_DNE)
      return -512;

  if (r == SIG_BAD_SIG)
      return -561;
  //kprintf("Sent a signal to %d\n", pid);
  //kprintf("Status of the sig masks are: %u for mask, %u for serv\n", p->sigmask, p->sigserv);

  if (targetPCB->state == STATE_SLEEP) {
    // kprintf("Target pid %d sleeping\n", targetPCB->pid);
    removeFromSleep(targetPCB);
    targetPCB->ret = targetPCB->sleepdiff * MILLISECONDS_TICK;
  }

  if (targetPCB->state == STATE_WAITING) {
    pcb* tmp = findPCB(targetPCB->otherpid);
    remove(targetPCB->pid, tmp->wait_queue);
    targetPCB->ret = -99;
  }
  ready(targetPCB);
  return 0;
}

/*****************************
| name: wait
| takes a process and puts it on the waiting queue of process pid
| returns -1 if it doesn't exist or 0 if successful
*****************************/
int wait(int pid, pcb* p) {
    pcb *w;
    w = findPCB(pid);

    if (!w || pid == 0)
        return -1;

    // queue onto wait queue
    if (!w->wait_queue) {
        w->wait_queue = p;
    } else {
        while (w->next) {
            w = w->next;
        }
        w->next = p;
    }
    p->otherpid = pid;
    p->next = NULL;
    p->state = STATE_WAITING;

    return 0;
}

/**********************
*   name: stop
*   pcb* p: the pcb to stop
*   description: stops the pcb
*   if the pcb had any waiters
*   ready them with return code 0
**********************/


extern void       stop(pcb *p) {
/***************************************/
  pcb* tmp;
  // release senders
  tmp = p->wait_queue;
  while (tmp) {
      tmp->ret = 0;
      ready(tmp);
      tmp = tmp->next;
  }
  p->state = STATE_STOPPED;
  kfree((void *)p->esp);
}

// This function is the system side of the sysgetcputimes call.
// It places into a the structure being pointed to information about
// each currently active process. 
//  p - a pointer into the pcbtab of the currently active process
//  ps  - a pointer to a processStatuses structure that is 
//        filled with information about all the processes currently in the system
//

extern char * maxaddr;
  
int getCPUtimes(pcb *p, processStatuses *ps) {
  
  int i, currentSlot;
  currentSlot = -1;

  // Check if address is in the hole
  if (((unsigned long) ps) >= HOLESTART && ((unsigned long) ps <= HOLEEND)) 
    return -1;

  //Check if address of the data structure is beyone the end of main memory
  if ((((char * ) ps) + sizeof(processStatuses)) > maxaddr)  
    return -2;

  // There are probably other address checks that can be done, but this is OK for now


  for (i=0; i < MAX_PROC; i++) {
    if (proctab[i].state != STATE_STOPPED) {
      // fill in the table entry
      currentSlot++;
      ps->pid[currentSlot] = proctab[i].pid;
      ps->status[currentSlot] = p->pid == proctab[i].pid ? STATE_RUNNING: proctab[i].state;
      ps->cpuTime[currentSlot] = proctab[i].cpuTime * MILLISECONDS_TICK;
    }
  }

  return currentSlot;
}

/******************
      Helpers
      Functions
*******************/

//Removes the pcb from it's position in the queue
extern void remove(unsigned int pid, pcb* p){
  pcb *prev;
  prev = NULL;

  if (pid == 0) {
      return;
  }

  while (p->pid != pid || !p) {
      prev = p;
      p = p->next;
  }

  if (!p) {
      // did not find it so do nothing
      return;
  }

  if (!prev) {
    //removing the first element in head
      p = p->next;
  } else {
    //remove yourself from list
      prev->next = p->next;
  }
  p->next = NULL;
}

// check that the address is in a valid position
int checkValidAddr(void* addr) {
    if (addr <= 0) {
      return 0;
    }
    if ((char *) addr > maxaddr) {
      return 0;
    }

    if ((unsigned long) addr > HOLESTART && (unsigned long) addr < HOLEEND){
      return 0;
    }
    return 1;
}
