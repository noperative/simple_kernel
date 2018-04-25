/* mem.c : memory manager
 */

#include <xeroskernel.h>
#include <i386.h>

/* Your code goes here */

void mergeSlots(void);
void freeSlotTraversalTest(void);
void allofreeTest(void);
void fragmentedMemTest(void);

extern long freemem;
extern long *maxaddr;

/* points to the beginning of the free mem linked list */
static struct memHeader *ptrFreeMem;


/********************

| name: kmeminit
| description: creates memory headers in free memory around the whole

*********************/

extern void kmeminit(void){
  //debug
  //kprintf( "kmeminit()::initiating memory at %d\n", freemem);

  /* initialize a memory header at the beginning of freemem to the hole */
  ptrFreeMem = freemem;
  ptrFreeMem->size = HOLESTART - (freemem + sizeof (struct memHeader));
  ptrFreeMem->next = HOLEEND;
  ptrFreeMem->prev = NULL;
  ptrFreeMem->sanityCheck = ptrFreeMem->dataStart;
  
  //DEBUG
  //kprintf("kmeminit()::initiating the second memory block at %d\n", HOLEEND);
  /* initialize a second memory header from the end of the hole to the end of memory */
  ptrFreeMem->next->next = NULL;
  ptrFreeMem->next->prev = ptrFreeMem;
  ptrFreeMem->next->size = (unsigned char *)maxaddr - HOLEEND - sizeof (struct memHeader);
  ptrFreeMem->next->sanityCheck = ptrFreeMem->next->dataStart;

  //Testing/Debugging Process
  //freeSlotTraversalTest();
  //allofreeTest();
  //fragmentedMemTest();
}

/******************

|name: kmalloc
|require: size of block to allocate (must be greater than 0)
|return: memory address of allocated block, or 0 if cannot allocate

******************/

extern void *kmalloc(size_t size){
 /* allocate memory from free space */
  
  
  //determine required size to allocate
  int reqSize = (size)/16 + ((size%16)?1:0);
  reqSize = reqSize*16  + sizeof(struct memHeader);
  
  //DEBUG
  //kprintf("kmalloc():: Allocating a chunk of size %d\n", reqSize);

  //find a free slot to allocate
  struct memHeader *header = ptrFreeMem;
  while(header->next){
    if (header->size >= reqSize){
      break;
    }
    else{
      header = header->next;
    }
  }
  if (header->next == NULL && header->size < reqSize)
    return 0;

  struct memHeader *memSlot = header;
  
  struct memHeader *newFreeSpace = memSlot + reqSize/16;
  //copy old header with adjusted size to new spot
  newFreeSpace->size = header->size - reqSize;
  newFreeSpace->next = header->next;
  newFreeSpace->next->prev = newFreeSpace;
  newFreeSpace->prev = header->prev;
  newFreeSpace->sanityCheck = newFreeSpace->dataStart;

  if (newFreeSpace->prev == NULL){
    ptrFreeMem = newFreeSpace;
  }
 
  memSlot->size = reqSize;
 
  // TESTING/DEBUG
  //kprintf("Allocated successfully\n");
  //freeSlotTraversalTest();

  return memSlot->dataStart;
}

/***********

|name: kfree
|requires: pointer to a valid memory chunk
|returns: 0 if failure, 1 if successful
|description: frees the memory and merges it with any contiguous free chunks

***********/
extern int kfree(void *ptr){
/* free the memory on this spot */

  struct memHeader *memSlot = ptr - sizeof(struct memHeader);
  
  //DEBUG
  //kprintf("kfree()::attempting to free address at: %d | ", memSlot);
  //kprintf("kfree()::current freemem start is: %d\n", ptrFreeMem);
  if (memSlot->dataStart != memSlot->sanityCheck){
    //kprintf("expected: %d, got: %d", memSlot->dataStart, memSlot->sanityCheck);
    return 0;
  }
  if (ptrFreeMem > memSlot){
    ptrFreeMem->prev = memSlot;
    memSlot->prev = NULL;
    memSlot->next = ptrFreeMem;
    ptrFreeMem = memSlot;
  }
  else{
    struct memHeader *prevSlot = ptrFreeMem;
    while ((prevSlot->next < ptr) && prevSlot->next){
      prevSlot = prevSlot->next;
    }
    memSlot->next = prevSlot->next;
    memSlot->prev = prevSlot;
    memSlot->next->prev = memSlot;
    prevSlot->next = memSlot;
  }
 
 //DEBUG
 //kprintf("kfree()::Memory free'd successfully, checking free memory list status\n");
 //freeSlotTraversalTest();
 //kprintf("kfree()::going to merge fragmented memory if it is there\n");

 mergeSlots();

 //DEBUG/TEST
 //kprintf("kfree()::memory defragmented\n");
 //freeSlotTraversalTest();

 return 1;
}

/**************

|name: mergeSlots
|description: finds contiguous blocks in free mem and merges them

**************/

void mergeSlots(void){
  struct memHeader *currentHeader = ptrFreeMem;
  do{
    long memBorder = (long) currentHeader + (long) currentHeader->size;
    //DEBUG
    //kprintf("Current header: %d, | header size: %d | next: %d |", currentHeader, currentHeader->size, currentHeader->next);
    //kprintf("boundary1: %d |", memBorder);
 
    //merge to the next block
    if (currentHeader->next == memBorder){
      currentHeader->size = currentHeader->size + currentHeader->next->size;
      currentHeader->next = currentHeader->next->next;
      currentHeader->next->prev = currentHeader;
    }

    memBorder = (long) currentHeader->prev + (long) currentHeader->prev->size;
    //DEBUG
    //kprintf("boundary2: %d\n", memBorder);
    //merge to the previous block
    if (currentHeader == memBorder){
      currentHeader->prev->next = currentHeader->next;
      currentHeader->next->prev = currentHeader->prev;
      currentHeader->prev->size = currentHeader->prev->size + currentHeader->size;
    }
    
    currentHeader = currentHeader->next;

  }while(currentHeader->next);

}

/**************

TEST BLOCK

***************/

/* 
|name: freeSlotTraversalTest
|description: runs through the free space headers and prints them out
*/
void freeSlotTraversalTest(void){
  kprintf("freeSlotTraversalTest():: printing out headers\n");
  struct memHeader *header = ptrFreeMem;
  while(header->next){
   kprintf("======Header=======");
   kprintf("Header address: %d |", header);
   kprintf("Datablock size: %d |",header->size);
   kprintf("sanityCheck: %d\n", header->sanityCheck);       
   header = header->next;
  };
  kprintf("======Header=====");
  kprintf("Header address: %d |", header);
  kprintf("Datablock size: %d |", header->size);
  kprintf("sanityCheck: %d |", header->sanityCheck);
  kprintf("=All Headers printed=\n");  
}

/* 
|name: allofreeTest
|description: Allocates a piece of memory and then frees it
*/
void allofreeTest(void){
  kprintf("allofreeTest()::allocating a memory address and then freeing it\n");
  void *address = kmalloc(1600);
  kprintf("allofreeTest()::got an address: %d will now attempt to free it\n", address);
  kfree(address);
  kprintf("allofreeTest()::address was freed\n");
}

/* 
|name: fragmentedMemTest
|description: Allocates 2 adjacent pieces of memory and determines if they are merged
*/
void fragmentedMemTest(void){
  kprintf("fragmentMemTest()::allocating 2 chunks and freeing them\n");
  void *address1 = kmalloc(1600);
  void *address2 = kmalloc(3200);
  kprintf("fragmentMemTest()::freeing 2 chunks, should expect only 1 large chunk as a result\n");
  kfree(address1);
  kfree(address2);
  kprintf("Done. Looks like memory was reclaimed successfully.\n");
}
