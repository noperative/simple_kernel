/* syscall.c : syscalls
 */

#include <xeroskernel.h>
#include <stdarg.h>

/* Your code goes here */
static int CALL;
static va_list ARGS;
static int RETURN;

/*********************
|name: syscall
|requires: valid call parameters for each case
|returns: return value after calling the process
|description: runs some assembly routines on a systemcall
*********************/
extern int syscall(int call, ...){
	va_list args;
    va_start(args, call);
	CALL = call;
	ARGS = args;

	switch (call) {
		case (CREATE): {
			void* func = va_arg(args, void*);
			int size = va_arg(args, int);
			return create(func, size);
		}
		case (YIELD): break;
		case (STOP): break;
	}	

	va_end(args);

	__asm __volatile("\
		movl CALL, %%eax\n\
		movl ARGS, %%edx\n\
		int $55\n\
		movl %%eax, RETURN\n\
	"
	:
	:
	: "%eax");

	return RETURN;
}

/******************
|name: syscreate
|requires: valid function and stack
|name: sysyield
|name: sysstop
|desciption: wrapper for calling syscall with the correct arguments
*****************/

extern unsigned int syscreate( void (*func)(void), int stack ){
	return syscall(CREATE, func, stack);
}

extern void sysyield( void ){
	//kprintf("|yielding...|");
	syscall(YIELD);
}
extern void sysstop( void ){
	syscall(STOP);
}
