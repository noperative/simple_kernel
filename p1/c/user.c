/* user.c : User processes
 */

#include <xeroskernel.h>
void producer(void);
void consumer(void);
void idle(void);

extern void root( void ) {
	kprintf("Hello World\n");
	syscreate(&producer, 8000);
	syscreate(&consumer, 8000);
	syscreate(&idle, 8000);
	sysstop();
	/*for (;;) {
		sysyield();
	}*/
}

void producer(void) {
	int i;
	for (i = 0; i < 12; i++) {
		kprintf("Happy ");
		sysyield();
	}
	sysstop();
}
void consumer(void) {
	int i;
	for (i = 0; i < 15; i++) {
		kprintf("New Year\n");
		sysyield();
	}

	sysstop();	
}

void idle(void){
	for (;;){
		sysyield();
	}
}
