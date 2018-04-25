/* create.c : create a process
 */

#include <xeroskernel.h>

extern int create( void (*func)(void), int stack ){
	struct pcb* pcb = getFreePCB();
	if (pcb == 0) {
		//kprintf("no pcb gotten");
		return 0;
	}

	// Allocate the stack using kmalloc()
	unsigned int datastart = kmalloc(stack + sizeof(struct context_frame));
	struct memHeader* header = datastart - 16;
	pcb->datastart = datastart;
	// Position the context at the end of the stack
	//kprintf("size: %d", header->size);
	struct context_frame *context = datastart + stack - sizeof(struct context_frame) - 64; // safety margin
	// Fill in context
	context->edi = 0;
	context->esi = 0;
	context->ebp = 0;
	context->esp = (unsigned int) context;
	context->ebx = 0;
	context->edx = 0;
	context->ecx = 0;
	context->eax = 0;
	context->iret_eip = func;
	context->iret_cs = getCS();
	context->eflags = 0;	

	pcb->cpustate = context;	

	// Process is now ready
	ready(pcb);
	//kprintf("created a process: %d , next: %d , status: %d\n", pcb->pid, pcb->next, pcb->state);

	return 1;
}
