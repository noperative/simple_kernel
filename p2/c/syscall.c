/* syscall.c : syscalls
 */

#include <xeroskernel.h>
#include <stdarg.h>


int syscall( int req, ... ) {
/**********************************/

    va_list     ap;
    int         rc;

    va_start( ap, req );

    __asm __volatile( " \
        movl %1, %%eax \n\
        movl %2, %%edx \n\
        int  %3 \n\
        movl %%eax, %0 \n\
        "
        : "=g" (rc)
        : "g" (req), "g" (ap), "i" (KERNEL_INT)
        : "%eax" 
    );
 
    va_end( ap );

    return( rc );
}

int syscreate( funcptr fp, size_t stack ) {
/*********************************************/

    return( syscall( SYS_CREATE, fp, stack ) );
}

void sysyield( void ) {
/***************************/
  syscall( SYS_YIELD );
}

 void sysstop( void ) {
/**************************/

   syscall( SYS_STOP );
}

PID_t sysgetpid( void ){
/**************************/
    return( syscall( SYS_GET_PID ) );
}

void sysputs( char *str ){
/**************************/
    syscall( SYS_PUTS, str );
}

int syskill( PID_t pid ){
/**************************/
    return( syscall( SYS_KILL, pid ) );
}

unsigned int syssleep( unsigned int milliseconds){
/*************************************************/
    return( syscall( SYS_SLEEP, milliseconds ) );
}

int syssend( PID_t dest_pid, void *buffer, int buffer_len ){
/***********************************************************/
    return( syscall( SYS_SEND, dest_pid, buffer, buffer_len) );
}

int sysrecv( PID_t *from_pid, void *buffer, int buffer_len ){
/************************************************************/
    return (syscall( SYS_RECV, from_pid, buffer, buffer_len));
}
