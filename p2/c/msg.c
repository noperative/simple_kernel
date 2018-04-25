/* msg.c : messaging system 
   This file does not need to modified until assignment 2
 */

#include <xeroskernel.h>
#include <i386.h>
#include <xeroslib.h>

extern  long  freemem;  /* start of free memory (set in i386.c) */
extern char *maxaddr; /* max memory address (set in i386.c) */

/*******************
*   Helpers
*
*********************/

/* Check if the buffer is at a reasonable spot */
Bool invalidBufferLoc( void* address) {
return (long) address < freemem || 
(long) address > (long) maxaddr || 
((long) address > HOLESTART && (long) address < HOLEEND);
}

/* Add a pcb to the end of the send queue */
void queueSend( pcb* src, pcb* dst ){
    pcb* temp = dst->send_queue;
    if(!temp){
        dst->send_queue = src;
    } else {
        while (temp->next != NULL){
            temp = temp->next;
        }
        temp->next = src;
        src->next = NULL;
    }
}

/* Add a pcb to the end of the recv queue */
void queueRecv( pcb* src, pcb* dst ){
    pcb* temp = dst->recv_queue;
    if(!temp){
        dst->recv_queue = src;
    } else {
        while (temp->next != NULL){
            temp = temp->next;
        }
        temp->next = src;
        src->next = NULL;
    }
}

/* remove a sender from a receiver's send queue */
void dequeueSend( pcb* sender, pcb* receiver){
  pcb* temp =  receiver->send_queue;
  if (temp->pid == sender->pid){
    receiver->send_queue = temp->next;
  } else {
    while ((temp->next != NULL) && (temp->next->pid != sender->pid)){
      temp = temp->next;
    }
    temp->next = temp->next->next;
  }
}


/* remove a receiver from a sender's recv queue */
void dequeueRecv( pcb* receiver, pcb* sender){
  pcb* temp =  sender->recv_queue;
  if (temp->pid == receiver->pid){
    sender->recv_queue = temp->next;
  } else {
    while ((temp->next != NULL) && (temp->next->pid != receiver->pid)){
      temp = temp->next;
    }
    temp->next = temp->next->next;
  }
}

/*********************
*
* name:   recv
* receives data into the buffer from srcPID
* receiverPCB   : the PCB that is receiving
* srcPID        : the PCB we wish to get data from (or 0)
* buffer        : the address of the allocated buffer space
* len           : length of the buffer
*
*********************/

int recv(pcb* receiverPCB, PID_t *srcPID, void *buffer, int len) {
  int sizeToCopy;
  pcb* srcPCB = findPCB(*srcPID);

  //ERROR CHECKING BLOCK
  if (srcPCB == NULL) return -1; //src does not exist
  if (receiverPCB->pid == *srcPID) return -2; //Trying to recv from self
  if (len < 0) return -3; //Negative buffer length
  if (invalidBufferLoc(buffer)) return -3; //the location for the buffer is invalid

  /* Receive from any */
  if (*srcPID == 0){
    // Try to take something from send queue
    if (receiverPCB->send_queue){
      //get any srcPCB
      srcPCB = receiverPCB->send_queue;
      //remove it from your send queue
      receiverPCB->send_queue = receiverPCB->send_queue->next;
      sizeToCopy = len < srcPCB->buffer.size ? len : srcPCB->buffer.size;
      srcPCB->ret = sizeToCopy;
      blkcopy(buffer, srcPCB->buffer.address, sizeToCopy);
      ready (srcPCB);
      return sizeToCopy;

    // Empty Send Queue -> Receive from anything state
    } else {
      receiverPCB->state = STATE_RECV_ANY;
      receiverPCB->buffer.size = len;
      receiverPCB->buffer.address = buffer;
      return -4;
    }
  }


  /* srcPCB is already sending */
  else if (srcPCB->state == STATE_SEND) {

    // Check if the srcPCB is in your send queue
    for (pcb* p = receiverPCB->send_queue ; p != NULL ; p = p->next){
      if (p->pid == srcPCB->pid){
        dequeueSend(srcPCB, receiverPCB);
        sizeToCopy = len < srcPCB->buffer.size ? len : srcPCB->buffer.size;
        srcPCB->ret = sizeToCopy;
        blkcopy(buffer, srcPCB->buffer.address, sizeToCopy);
        srcPCB->next = NULL;
        ready (srcPCB);
        return sizeToCopy;
      }
    }
  }


  /* block in recv state on the srcPCB */
  receiverPCB->state = STATE_RECV;
  receiverPCB->buffer.size = len;
  receiverPCB->buffer.address = buffer;
  queueRecv(receiverPCB, srcPCB);
  return -4;
}

/*********************
*
* name:   send
* send data from the buffer into dstPID
* senderPCB     : the PCB that is sending
* dstPID        : the PCB we wish to send data to
* buffer        : the address of the allocated buffer space
* len           : length of the buffer
*
*********************/

int send(pcb* senderPCB, PID_t dstPID, void *buffer, int len) {
  int sizeToCopy;
  pcb* dstPCB = findPCB(dstPID);

  //ERROR CHECKING BLOCK
  if (dstPCB == NULL) return -1; //src does not exist
  if (dstPID == senderPCB->pid) return -2; //Trying to send to self
  if (len < 0) return -3; //Negative buffer length
  if (invalidBufferLoc(buffer)) return -3; //the location for the buffer is invalid

  /* The dstPCB is already waiting for a send */
  if (dstPCB->state == STATE_RECV_ANY) {
    // check that the receiving buffer is big enough
    sizeToCopy = dstPCB->buffer.size < len ? dstPCB->buffer.size : len;
    // Copy the data to the destination process' buffer
    blkcopy(dstPCB->buffer.address, buffer, sizeToCopy);
    dstPCB->ret = sizeToCopy;
    ready(dstPCB);
    return sizeToCopy;

  }

  else if (dstPCB->state == STATE_RECV){
    // check that dstPCB is in sender's recv queue
    for (pcb* p = senderPCB->recv_queue ; p != NULL ; p = p->next){
      if (p->pid == dstPCB->pid){
        dequeueRecv(dstPCB, senderPCB);
        sizeToCopy = len < dstPCB->buffer.size ? len : dstPCB->buffer.size;
        blkcopy(dstPCB->buffer.address, buffer, sizeToCopy);
        dstPCB->ret = sizeToCopy;
        ready(dstPCB);
        return sizeToCopy;
      }
    }

  }

  /* the dstPCB is running so we will block first */
  senderPCB->state = STATE_SEND;
  senderPCB->buffer.size = len;
  senderPCB->buffer.address = buffer;
  queueSend(senderPCB, dstPCB);
  return -4;
}
