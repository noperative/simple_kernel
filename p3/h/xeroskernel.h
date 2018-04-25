/* xeroskernel.h - disable, enable, halt, restore, isodd, min, max */
#include <stdarg.h>
#ifndef XEROSKERNEL_H
#define XEROSKERNEL_H

/* A typedef for the signature of the function passed to syscreate */
typedef void    (*funcptr)(void);

/* Symbolic constants used throughout Xinu */

typedef	char    Bool;        /* Boolean type                  */
typedef unsigned int size_t; /* Something that can hold the value of
                              * theoretical maximum number of bytes 
                              * addressable in this architecture.
                              */
#define	FALSE   0       /* Boolean constants             */
#define	TRUE    1
#define	EMPTY   (-1)    /* an illegal gpq                */
#define	NULL    0       /* Null pointer for linked lists */
#define	NULLCH '\0'     /* The null character            */

#define CREATE_FAILURE -1  /* Process creation failed     */



/* Universal return constants */

#define	OK            1         /* system call ok               */
#define	SYSERR       -1         /* system call failed           */
#define	EOF          -2         /* End-of-file (usu. from read)	*/
#define	TIMEOUT      -3         /* time out  (usu. recvtim)     */
#define	INTRMSG      -4         /* keyboard "intr" key pressed	*/
                                /*  (usu. defined as ^B)        */
#define	BLOCKERR     -5         /* non-blocking op would block  */

/* Functions defined by startup code */


void           bzero(void *base, int cnt);
void           bcopy(const void *src, void *dest, unsigned int n);
void           disable(void);
unsigned short getCS(void);
unsigned char  inb(unsigned int);
void           init8259(void);
int            kprintf(char * fmt, ...);
void           lidt(void);
void           outb(unsigned int, unsigned char);


/* Some constants involved with process creation and managment */
 
   /* Maximum number of processes */      
#define MAX_PROC        64           
   /* Kernel trap number          */
#define KERNEL_INT      80
   /* Interrupt number for the timer */
#define TIMER_INT      (TIMER_IRQ + 32)
    
  /* Interrupt number for the keyboard */
#define KEYBOARD_IRQ    1
#define KEYBOARD_INT    (KEYBOARD_IRQ + 32)

   /* Minimum size of a stack when a process is created */
#define PROC_STACK      (4096 * 4)    

   /* Number of milliseconds in a tick */
#define MILLISECONDS_TICK 10        

#define MAX_DEV         4
#define MAX_SIG         31

/* make sure interrupts are armed later on in the kernel development  */
#define STARTING_EFLAGS         0x00003000
#define ARM_INTERRUPTS          0x00000200

/* Constants to track states that a process is in */
#define STATE_STOPPED   0
#define STATE_READY     1
#define STATE_SLEEP     22
#define STATE_RUNNING   23
#define STATE_WAITING   24

/* System call identifiers */
#define SYS_STOP        10
#define SYS_YIELD       11
#define SYS_CREATE      22
#define SYS_TIMER       33
#define SYS_GETPID      144
#define SYS_PUTS        155
#define SYS_SLEEP       166
#define SYS_KILL        177
#define SYS_CPUTIMES    178
#define SYS_SIG_HANDLER 170
#define SYS_SIGRETURN   180
#define SYS_WAIT        190
#define SYS_OPEN        210
#define SYS_CLOSE       220
#define SYS_WRITE       230
#define SYS_READ        240
#define SYS_IOCTL       250

#define SYS_KEYBOARD    800

/* Signal Return Codes */
#define SIG_SUCCESS     111
#define SIG_PROC_DNE    404
#define SIG_BAD_SIG     405

typedef struct struct_pcb pcb;

/* Device structure */
typedef struct devsw {
  int dvnum;
  char *dvname;


  int (*dvopen)(pcb* p, int device_no);
  int (*dvclose)(void);
  int (*dvread)(pcb *p, void *buff, int bufflen);
  int (*dvwrite)(pcb *p, void *buff, int bufflen);
  int (*dvioctl)(unsigned long command, va_list vargs);

  // These are unused in our current kernel since we
  // only have di_calls for the above
  /*
  int (*dvinit)(void);
  int (*dvgetc)(void);
  int (*dvputc)(void);
  int (*dvcntl)(void);
  void (*dvcsr)(void);
  void (*dvivec)(void);
  void (*dvovec)(void);
  int (*dviint)(void);
  int (*dvoint)(void);
  void (*dvioblk)(void);
  int dvminor;
  */
} devsw;

/* Structure to track the information associated with a single process */


struct struct_pcb {
  void        *esp;    /* Pointer to top of saved stack           */
  pcb         *next;   /* Next process in the list, if applicable */
  pcb         *prev;   /* Previous proccess in list, if applicable*/
  int          state;  /* State the process is in, see above      */
  unsigned int pid;    /* The process's ID                        */
  int          ret;    /* Return value of system call             */
                       /* if process interrupted because of system*/
                       /* call                                    */
  long         args;   
  unsigned int otherpid;
  void        *buffer;
  int          bufferlen;
  int          sleepdiff;
  long         cpuTime;  /* CPU time consumed                     */
  funcptr      sigtable[32];
  unsigned int sigmask;  /* signals currently being handled */
  unsigned int sigserv;  /* signals to service */
                         /* no tramps can be built but sigs can come in */
  pcb          *wait_queue;
  devsw        *fdt[MAX_DEV];  /* pcb device table */
};


typedef struct struct_ps processStatuses;
struct struct_ps {
  int  entries;            // Last entry used in the table
  int  pid[MAX_PROC];      // The process ID
  int  status[MAX_PROC];   // The process status
  long  cpuTime[MAX_PROC]; // CPU time used in milliseconds
};

// Kernel device table
devsw dev_tab[2];

/* The actual space is set aside in create.c */
extern pcb     proctab[MAX_PROC];

#pragma pack(1)

/* What the set of pushed registers looks like on the stack */
typedef struct context_frame {
  unsigned long        edi;
  unsigned long        esi;
  unsigned long        ebp;
  unsigned long        esp;
  unsigned long        ebx;
  unsigned long        edx;
  unsigned long        ecx;
  unsigned long        eax;
  unsigned long        iret_eip;
  unsigned long        iret_cs;
  unsigned long        eflags;
  unsigned long        stackSlots[];
} context_frame;

/* Signal context to save onto the stack when a signal tramp is built */
typedef struct sig_context {
  unsigned long return_address; // Will never be used
  funcptr handler;
  unsigned long context;
  int old_ret;
  int signal;                   // which signal is serviced

} sig_context;

/* Memory mangement system functions, it is OK for user level   */
/* processes to call these.                                     */

int      kfree(void *ptr);
void     kmeminit( void );
void     *kmalloc( size_t );

/* Internal functions for the kernel, applications must never  */
/* call these.                                                 */
void     dispatch( void );
void     dispatchinit( void );
void     ready( pcb *p );
pcb      *next( void );
void     stop( pcb *p );
void     contextinit( void );
int      contextswitch( pcb *p );
int      create( funcptr fp, size_t stack );
void     set_evec(unsigned int xnum, unsigned long handler);
void     printCF (void * stack);  /* print the call frame */
int      syscall(int call, ...);  /* Used in the system call stub */
void     sleep(pcb *, unsigned int);
void     removeFromSleep(pcb * p);
void     tick( void );
int      getCPUtimes(pcb *p, processStatuses *ps);
void     kbdinit(void);
void     kbd_int( void );

/* Function prototypes for system calls as called by the application */
int          syscreate( funcptr fp, size_t stack );
void         sysyield( void );
void         sysstop( void );
unsigned int sysgetpid( void );
unsigned int syssleep(unsigned int);
void         sysputs(char *str);
int          syskill(int PID, int signalNumber);
int          sysgetcputimes(processStatuses *ps);
int          syssighandler(int signal, void (*newhandler)(void *), void (** oldHandler)(void *));
void         syssigreturn(void *old_sp);
int          syswait(int pid);
int          sysopen(int deviceno);
int          sysclose(int fd);
int          syswrite(int fd, void *buff, int bufflen);
int          sysread(int fd, void *buff, int bufflen);
int          sysioctl(int fd, unsigned long command, ...);

/* Function prototypes for dispatch helpers */
extern void remove(unsigned int pid, pcb* p);

/* signal related functions */
void         sigtramp(void (*handler)(void *), void *cntx);
extern int   signal(pcb* p, int sig);
void         build_tramp(pcb *p);

/* device related functions */
int di_open(pcb *p, int deviceno);
int di_close(pcb *p, int fd);
int di_read(pcb *p, int fd, void *buff, int bufflen);
int di_write(pcb *p, int fd, void *buff, int bufflen);
int di_ioctl(pcb *p, int fd, unsigned long command, va_list vargs);

/* The initial process that the system creates and schedules */
void     root( void );
void     shell( void );
/* Test processes */
void     test1( void );
void     test2( void );
void     test3( void );
void     test4( void );
void     test5( void );
void     test6( void );
void     test7( void );
void     test8( void );
void     testA( void );
void     testB( void );


void           set_evec(unsigned int xnum, unsigned long handler);


/* Anything you add must be between the #define and this comment */
#endif

