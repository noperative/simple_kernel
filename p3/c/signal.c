/* signal.c - support for signal handling
   This file is not used until Assignment 3
 */

#include <xeroskernel.h>
#include <xeroslib.h>
extern pcb *findPCB( int pid );
/* Your code goes here */

/***********************
| name:		sigtramp
| handler:	address of a handler function for a signal
| cntx:		address of a context frame for the handler
| description: calls the handler and then executes sigreturn on
|				old_sp, which never returns
**********************/

void sigtramp(void (*handler)(void *), void *cntx) {
  handler(cntx);
	syssigreturn(cntx);
}


/*******************
| name:		signal
| pcb* p  :the PCB to signal
| int sig :the signal type
| description: 	sends a signal of type sig to the
|				pcb as long as it's valid
********************/
extern int signal(pcb* p, int sig) {
	if (p->pid == 0) return SIG_PROC_DNE;
  else if (sig < 0 || sig > 31) return SIG_BAD_SIG;
  else if (!p) return SIG_PROC_DNE;
  else if (p->sigtable[sig] == NULL) return 0;

  unsigned int signal = 1 << sig;
  if (signal > (p->sigmask >> 1)){
    p->sigserv |= signal;
  }
  return 0;
}

/**********************
| name: build_tramp
|
| description:  sets up the stack on pcb p to a sigtramp frame
|               instead of the current stack
| * the bit for the signal will be moved from sigserv to sigmask
| * this indicates that signals below sigmask's priority will be
| * blocked, but existing signals in sigserv can still be queued
***********************/

void build_tramp(pcb *p){

  // Figure out the signal to service
  int signal;
  for ( signal =  31; signal >= 0; signal-- ) {
    // take the first bit we find from sigserv
    if ( (p->sigserv >> signal) & 1 ) {
      // clear the signal from sigserv
      p->sigserv &= ~(1 << signal);
      // add to sigmask to indicate it is being handled
      p->sigmask |= 1 << signal;
      break;
    }
  }
  funcptr handler = (funcptr)p->sigtable[signal];

  // record old context
  unsigned long old_sp = (unsigned long) p->esp;

  unsigned long new_sp = (unsigned long) p->esp;
  new_sp -= sizeof(sig_context);
  sig_context* new_stack = (sig_context*)new_sp;
  new_stack->handler = handler;
  new_stack->context = old_sp;
  new_stack->old_ret = p->ret;
  new_stack->signal = signal;

  new_sp -= sizeof(context_frame);
  context_frame* new_context = (context_frame*) new_sp;
  new_context->edi = 0;
  new_context->esi = 0;
  new_context->ebp = new_sp;
  new_context->esp = new_sp;
  new_context->ebx = 0;
  new_context->edx = 0;
  new_context->ecx = 0;
  new_context->eax = 0;
  new_context->iret_eip = (unsigned long) sigtramp;
  new_context->iret_cs = getCS();
  new_context->eflags = STARTING_EFLAGS | ARM_INTERRUPTS;

  p->esp = (void* )new_sp;
}
