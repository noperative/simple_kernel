/* xeroskernel.h - disable, enable, halt, restore, isodd, min, max */

#ifndef XEROSKERNEL_H
#define XEROSKERNEL_H

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
void           set_evec(unsigned int xnum, unsigned long handler);

/**********

| struct: memHeader
| size: size of the free memory
| *prev: pointer to the prev free memory header
| *next: pointer to the next free memory header
| *sanityCheck: address of the free mem header
| dataStart[0]: address to the start of the memory block

************/

struct memHeader {
  unsigned long size;
  struct memHeader *prev;
  struct memHeader *next;
  char *sanityCheck;
  unsigned char dataStart[0];
};

/*
|name: context_frame
|description: the registers and flags to be saved in context
*/
struct context_frame {
	unsigned long edi;
	unsigned long esi;
	unsigned long ebp;
	unsigned long esp;
	unsigned long ebx;
	unsigned long edx;
	unsigned long ecx;
	unsigned long eax;
	unsigned long iret_eip;
    unsigned long iret_cs;
	unsigned long eflags;
};

/*
|name: PCB
|description: a process control block to be used in the table
*/
struct pcb {
	int pid;
	int state;
	struct context_frame *cpustate;
	struct pcb *next;
	unsigned int datastart;
};

/* Memory */
extern void kmeminit(void);
extern void *kmalloc (size_t size);
extern int kfree(void *ptr);

/* Dispatcher */
extern void dpinit(void);
extern void dispatch(void);
extern struct pcb* getFreePCB(void);
extern void ready(struct pcb* process);
extern void cleanup(struct pcb* process);

/* Context Switcher */
extern int contextswitch(struct pcb*);
extern void contextinit(void);

/* Process Creator */
extern int create( void (*func)(void), int stack );

/* Syscalls */
extern int syscall(int call, ...);
extern unsigned int syscreate( void (*func)(void), int stack );
extern void sysyield( void );
extern void sysstop( void );

/* User */
extern void root( void );
 
#define PCB_TABLE_SIZE 4
#define READY 0
#define RUNNING 1
#define BLOCKED 2
#define STOPPED 3
#define CREATE 0
#define YIELD 1
#define STOP 2
/* Anything you add must be between the #define and this comment */
#endif
