#include <xeroskernel.h>
#include <i386.h>
#include <xeroslib.h>

extern char * maxaddr;
int verify_buffer(void *buff, int bufflen);

/************************
| name:   di_open
| pcb *p  :      the pcb whose FDT we use
| int deviceno : device number we want
| returns the FD number if sucessful, -1 if fail
*************************/
int di_open(pcb *p, int deviceno){
  if (deviceno < 0 || deviceno >= 2) return -1;

  devsw* dev = &dev_tab[deviceno];
  if (dev->dvopen == NULL) return -1;

  //Search for first available space in fd table
  int fd;
  for (fd = 0; fd < MAX_DEV; fd++){
    if(p->fdt[fd] == NULL){
      //Found a spot, open device
      dev->dvopen(p, deviceno); 
      p->fdt[fd] = dev;
      return fd;
    }
  }
  //Can't find space
  return -1;
}


/*************************
| name:   di_close
| pcb *p:   the pcb whose FDT we want
| int fd:   the file descriptor we want to close
|           (returned from a di_open)
| returns 0 if successful, -1 if fail
****************************/
int di_close(pcb* p, int fd){
  devsw *devptr;  

  // Check validity of device table
  if ( 0 > fd || fd > MAX_DEV) return -1;
  else if (p->fdt[fd] == NULL) return -1;

  devptr = p->fdt[fd];
  (devptr->dvclose)();
  p->fdt[fd] = NULL;

  return 0;
}

/*************************
| name:   di_write
| pcb *p:   the pcb whose FDT we want
| int fd:   the file descriptor we want to write
|           (returned from a di_open)
| void *buff: the buffer we give
| int bufflen: the length of the buffer
| returns bytes written if successful, -1 if fail
*************************************/

int di_write(pcb *p, int fd, void *buff, int bufflen){
  devsw *devptr;  

  // Check validity of device table
  if ( 0 > fd || fd > MAX_DEV) return -1;
  else if (p->fdt[fd] == NULL) return -1;

  devptr = p->fdt[fd];
  return (devptr->dvwrite)(p, buff, bufflen);
}

/*************************
| name:   di_read
| pcb *p:   the pcb whose FDT we want
| int fd:   the file descriptor we want to read
|           (returned from a di_open)
| void *buff: the buffer we give
| int bufflen: the length of the buffer
| returns bytes read if successful, -1 if fail
*************************************/
int di_read(pcb* p, int fd, void *buff, int bufflen){
  devsw *devptr;

  // Check validity of device table
  if ( 0 > fd || fd > MAX_DEV) return -1;
  else if (p->fdt[fd] == NULL) return -1;

  devptr = p->fdt[fd];
  p->state = STATE_WAITING;
  return (devptr->dvread)(p, buff, bufflen);
}

/*************************
| name:   di_ioctl
| pcb *p:   the pcb whose FDT we want
| int fd:   the file descriptor we're issuing a command to
|           (returned from a di_open)
| command:  the device specific command
| vargs  :  the arguments (up to device to read)
| returns 0 if successful, -1 if fail
****************************/

int di_ioctl(pcb* p, int fd, unsigned long command, va_list vargs){
  devsw *devptr;

  // Check validity of device table
  if ( 0 > fd || fd > MAX_DEV) return -1;
  else if (p->fdt[fd] == NULL) return -1;

  devptr = p->fdt[fd];
  return (devptr->dvioctl)(command, vargs);
}



// Helper for verifying buffer address
int verify_buffer(void *buff, int bufflen) {

  // check if buffer is valid
  if ( buff < 0 || ((char *) buff) > maxaddr ) {
    return 0;
  }

  // check if buffer is in hole
  if ( ((unsigned long) buff > HOLESTART) && ((unsigned long) buff < HOLEEND) ) {
    return 0;
  }

  // check if bufflen is valid
  if ( bufflen <= 0 ) {
    return 0;
  }

  return 1;
}