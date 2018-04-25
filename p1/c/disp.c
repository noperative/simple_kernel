/* disp.c : dispatcher
 */

#include <xeroskernel.h>

void printPCBT(void);
struct pcb* next(void);
void queue(struct pcb* queue, struct pcb* process);

struct pcb* dequeue(struct pcb* head);
struct pcb pcb_table[PCB_TABLE_SIZE];
static struct pcb *blockedQueue;
static struct pcb *readyQueue;

/************************
|
| name: dpinit
| description: set the PCB table and queues
|
************************/

extern void dpinit(void){
	// start first element as head of the blocked queue
    // inititalize the PCB table
    // add all blocks to the blocked queue initially

	readyQueue = NULL;

	pcb_table[0].pid = 0;
	pcb_table[0].next = NULL;
	pcb_table[0].cpustate = NULL;
	pcb_table[0].state = BLOCKED;
	blockedQueue = &pcb_table[0];

	int i;
	for (i = 1; i < PCB_TABLE_SIZE; i++) {
		pcb_table[i].pid = i;
		pcb_table[i].cpustate = NULL;
		pcb_table[i].next = blockedQueue;
		pcb_table[i].state = BLOCKED;
		blockedQueue = &pcb_table[i];
	}
	// DEBUG/TEST
	//printPCBT();
}

/************************
|
| name: dispatch
| description: infinite loop that deals with syscalls
|
*************************/

extern void dispatch(void){
	//printPCBT();
	//kprintf("getting next process\n");
	struct pcb* process = next();
	//kprintf("process is: %d , next: %d, state: %d |", process->pid, process->next, process->state);
	for(;;) {
		//kprintf("next process:%d||", process->pid);
		int request;
		//kprintf("about to call context switch ||");
		request = contextswitch(process);
		//kprintf("| request is: %d |", request);
		switch(request) {
			case(CREATE): {
				create(process->cpustate->iret_eip, 1600);
				break;
			}
			case(YIELD): ready(process); process = next();break;
			case(STOP): cleanup(process); process = next(); break;
		}
	}
}

/************************
|
| name: getFreePCB
| requires: non-empty blocked queue
| returns: pointer to a dequeued blocked PCB
| description: dequeues PCB from blocked queue, to be used by process creation
|
*************************/

extern struct pcb* getFreePCB(void){
	//kprintf("finding a free PCB");
	if (blockedQueue == NULL){
		return 0;
	}
	struct pcb* free = blockedQueue;
	blockedQueue = blockedQueue->next;
	free->next = NULL;
	//DEBUG
	//kprintf("Found one with pid: %d", free->pid);
	//printPCBT();
	return free;

}
/************************
|
| name: next
| requires: non-empty ready queue
| returns: pointer to a dequeued ready block
| description: dequeues PCB from ready queue, 
|
*************************/

struct pcb* next(void){
	struct pcb *next_process = readyQueue;
	readyQueue = readyQueue->next;
	next_process->next = NULL;
	next_process->state = RUNNING;
	//DEBUG
	//printPCBT();
	return next_process;
}

/************************
|
| name: ready
| requires: a valid PCB
| description: queues PCB to the ready bqueue
|
*************************/
extern void ready(struct pcb* process){
	//DEBUG
	//kprintf("ready()::readying pcb %d |", process->pid);

	process->state = READY;
	process->next = NULL;
	if (readyQueue == NULL){
		//kprintf("first element in ready queue");
		readyQueue = process;
	}
	else {
		struct pcb *current = readyQueue;
		while(current->next){
			current = current->next;
		}
		current->next = process;
	}
	//DEBUG
	//printPCBT();
}

/************************
|
| name: cleanup
| requires: a valid PCB
| description: queues PCB at the end of the blocked queue
|
*************************/
extern void cleanup(struct pcb* process){
	//DEBUG
	//kprintf("cleanup()::blocking pcb %d |", process->pid);

	
	kfree(process->datastart);
	process->state = BLOCKED;
	if (blockedQueue == NULL){
		process->next = NULL;
		blockedQueue = process;
	}
	else {
		struct pcb *current = blockedQueue;
		while(current->next){
			current = current->next;
		}
		current->next = process;
	}
	//DEBUG
	//printPCBT();
}

/************************
| TESTING BLOCK
| name: printPCBT
| description: prints out the PCB table for debugging
|
*************************/

void printPCBT(void){
	int i;
	for (i = 0; i < PCB_TABLE_SIZE; i++) {
		struct pcb* pcb = &pcb_table[i];
		kprintf("pcb:%d,next:%d,state:%d|| ", pcb->pid, pcb->next, pcb->state);
	}
	kprintf("blockedQueue:%d,readyQueue:%d,pcb_table:%d\n", blockedQueue->pid, readyQueue->pid, pcb_table->pid);
}
