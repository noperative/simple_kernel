/* disp.c : dispatcher
 */

#include <xeroskernel.h>
#include <xeroslib.h>
#include <stdarg.h>
#include <i386.h>


static pcb      *head = NULL;
static pcb      *tail = NULL;

// idle proc will always be initiated at 0
static int      idleProcID = 0;

void     dispatch( void ) {
/********************************/

    pcb         *p;
    int         r;
    funcptr     fp;
    int         stack;
    va_list     ap;
    PID_t       dstPID;
    PID_t       *srcPID;
    void        *buffer;
    int         len;
    unsigned int time;

    for( p = next(); p; ) {
      //      kprintf("Process %x selected stck %x\n", p, p->esp);

      r = contextswitch( p );
      switch( r ) {
      case( SYS_CREATE ):
      /*****************/
        ap = (va_list)p->args;
        fp = (funcptr)(va_arg( ap, int ) );
        stack = va_arg( ap, int );
	      p->ret = create( fp, stack );
        break;
      case( SYS_YIELD ):
      /*****************/
        ready( p );
        p = next();
        break;
      case( SYS_STOP ):
      /*****************/
        stop( p );
        p = next();
        break;
      case ( SYS_GET_PID ):
      /*****************/
        p->ret = p->pid;
        break;
      case ( SYS_PUTS ):
      /*****************/
        ap = (va_list)p->args;
        char* str = va_arg(ap, char*);
        kprintf("%s", str);
        break;
      case ( SYS_KILL ):
      /*****************/
        ap = (va_list)p->args;
        PID_t pid = va_arg(ap, PID_t);
        pcb* target = findPCB(pid);
        if (target == NULL){
          p->ret = -1;
          break;
        } else if (p->pid == pid) {
          //you're killing yourself so dont need to remove
          stop( target );
          p = next();
        } else {
          stop( target );
          remove( pid );
          p->ret  = 0;
        }
        break;
      case ( SYS_TIMER ):
      /*****************/
        tick();
        ready( p );
        p = next();
        end_of_intr();
        break;

      case( SYS_SLEEP ):
      /*****************/
        ap = (va_list)p->args;
        time = va_arg(ap, unsigned int);
        p->ret = sleep(p, time);
        p = next(); //need to get the next process, current process stopped
        break;

      case( SYS_SEND ):
        ap = (va_list)p->args;
        dstPID = (va_arg( ap, PID_t ) );
        buffer = va_arg( ap, void* );
        len = va_arg( ap, int );

        p->ret = send( p, dstPID, buffer, len );
        // if it's an error then get the next process
        if (p->ret < -2){
          p = next();
        }

        break;
      case( SYS_RECV ):       
        ap = (va_list)p->args;
        srcPID = va_arg( ap, PID_t*);
        buffer = va_arg( ap, void* );
        len = va_arg( ap, int );

        p->ret = recv( p, srcPID, buffer, len );
        // if it's an error then get the next process
        if (p->ret < -2){
          p = next();
        }

        break;

      default:
      /*****************/
        kprintf( "Bad Sys request %d, pid = %u\n", r, p->pid );
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

    if (p->pid == idleProcID) {
      // don't ready the idle process
      return;
    }
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
    } else {
      // if no process available then use idle process
      p = findPCB(idleProcID);
    }

    return( p );
}

/**********************
*   name: stop
*   pcb* p: the pcb to stop
*   description: stops the pcb
*   if the pcb had any receivers
*   or senders in queue, return -1
*   and ready them
**********************/


extern void       stop(pcb *p) {
/***************************************/
  pcb* tmp;
  // release senders
  tmp = p->send_queue;
  while (tmp) {
      tmp->ret = -1;
      ready(tmp);
      tmp = tmp->next;
  }

  // release receivers
  tmp = p->recv_queue;
  while (tmp) {
      tmp->ret = -1;
      ready(tmp);
      tmp = tmp->next;
  }

  p->state = STATE_STOPPED;
  kfree((void *)p->esp);
}

/******************
      Helpers
      Functions
*******************/

extern pcb *findPCB(PID_t pid) {
    /******************************/

    int i;
    for (i = 0; i < MAX_PROC; i++) {
        if (proctab[i].pid == pid &&  proctab[i].state != STATE_STOPPED) {
            return ( &proctab[i]);
        }
    }

    return (NULL);
}

//Removes the pcb from it's position in the queue
extern void remove(PID_t pid){
  pcb* p;
  pcb *prev;
  prev = NULL;
  p = head;

  if (pid == idleProcID) {
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
      head = head->next;
  } else {
    //remove yourself from list
      prev->next = p->next;
  }
  p->next = NULL;
}
