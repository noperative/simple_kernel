/* sleep.c : sleep device 
   This file does not need to modified until assignment 2
 */

#include <xeroskernel.h>
#include <xeroslib.h>

#define TICK_TIME 10;

pcb* sleepQ = NULL;

/****************
*	name: 	sleep
*	p: 		a pointer to the process control block
*			to put to sleep
*	time:	the time in milliseconds to sleep for
*	
*	Converts the time to system ticks and then
*	adds the pcb to the sleep queue

****************/
unsigned int sleep(pcb* p, unsigned int time ){
/****************************************/
	//Just ready it if it's not going to sleep
    if( time < 1 ) {
        ready( p );
        return 0;
    }

    //Calculate the number of system ticks
    unsigned int ticks = time / TICK_TIME;

    p->state = STATE_SLEPT;
    p->next = NULL;

    //If queue is empty just add it to the beginning
    if( !sleepQ ) {
        sleepQ = p;
        p->ticks = ticks;

    //Add to the beginning of the queue
    } else if( sleepQ->ticks > ticks ) {
        sleepQ->ticks -= ticks;
        p->next = sleepQ;
        p->ticks = ticks;
        sleepQ = p;

    //Search for the correct spot in the queue
    } else {
        ticks -= sleepQ->ticks;
        pcb* i;
        for( i = sleepQ; i->next; i = i->next ) {
            if( ticks < i->next->ticks ) {
                break;
            } else {
                ticks -= i->next->ticks;
            }
        }

        p->next = i->next;
        p->ticks = ticks;
        i->next = p;
        if( p->next ) {
            p->next->ticks -= ticks;
        }
    }

    return 0;
}


/****************
*	name: 	tick
*
*	Decrements the ticks on the head pcb
*	of the sleep queue by 1
*	If the ticks hit zero then we ready
*	We only dequeue one pcb at a time

****************/
void	tick( void ) {
/****************************/

	if (sleepQ) {
		if (sleepQ->ticks <= 0) {
			if (sleepQ->next) {
				pcb *temp = sleepQ;
				sleepQ = sleepQ->next;
				ready(temp);
			}else{	
				ready(sleepQ);
				sleepQ = NULL;
			}
		}else{
			sleepQ->ticks--;
		}
	}
}