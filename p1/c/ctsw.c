/* ctsw.c : context switcher
 */

#include <xeroskernel.h>

/* Your code goes here - You will need to write some assembly code. You must
   use the gnu conventions for specifying the instructions. (i.e this is the
   format used in class and on the slides.) You are not allowed to change the
   compiler/assembler options or issue directives to permit usage of Intel's
   assembly language conventions.
*/
void _ISREntryPoint();
static void *k_stack;
static unsigned long ESP;
static int request;

/**********************
|name: contextinit
|description: sets an interrupt into the interrupt table for our entry point
|
**********************/
extern void contextinit(void) {
	set_evec(55, &_ISREntryPoint);
}

/**********************
|name: contextswitch
|requires: valid process block with context
|returns: a number that indicates whether to block or stop the process or create a new one
|description: inline assembly code for switching to a process and running it
**********************/
extern int contextswitch(struct pcb* process) { 
	ESP = process->cpustate->esp;
	//kprintf("ESP before: %d ||", ESP);
	__asm __volatile("\
		pushf\n\
		pusha\n\
		movl %%esp, k_stack\n\
		movl ESP, %%esp\n\
		popa\n\
		iret\n\
		\
		_ISREntryPoint:\n\
			pusha\n\
			movl %%esp, ESP\n\
			movl k_stack, %%esp\n\
			movl %%eax, request\n\
			popa\n\
			popf\n\
		"
		:
		:
		: "%eax"
	);
	process->cpustate->esp = ESP;
	//kprintf("ESP after: %d\n", ESP);
	return request;
}
