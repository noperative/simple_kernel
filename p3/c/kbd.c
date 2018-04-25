#include <kbd.h>
#include <i386.h>
#include <xeroslib.h>

char read_buffer(buffer *buffer);
Bool not_special(unsigned char scan_code);

// Keep track of the process using the keyboard
static pcb *process;

//Keep the state of the keyboard
static int kb_state = STORE; 
static int kb_echo;

static buffer kbuffer; // the buffer to hold the characters

// For the process buffer
static char *p_buff; 
static int p_len;
static int p_index;

static unsigned int eof; // an eof index for the buffer

/***************************
| name:       kbd_init
| desc:       adds 2 keyboards to the
              kernel device table
****************************/
void kbdinit(void){
  devsw *device = &dev_tab[0];
  device->dvnum = 0;
  device->dvname = "keyboard";
  device->dvopen = kbd_open;
  device->dvclose = kbd_close;
  device->dvwrite =  kbd_write;
  device->dvread = kbd_read;
  device->dvioctl = kbd_ioctl;

  device = &dev_tab[1];
  device->dvnum = 1;
  device->dvname = "keyboard_echo";
  device->dvopen = kbd_open;
  device->dvclose = kbd_close;
  device->dvwrite = kbd_write;
  device->dvread = kbd_read;
  device->dvioctl = kbd_ioctl;
}

/***************************
| name:       kbd_int
| desc:       processes keyboard
              inputs based on
              current state
****************************/

void kbd_int(void){
  unsigned char scan_code = inb(KBD_PORT);
  if (scan_code == NULL) return;
  unsigned int ascii = kbtoa(scan_code);


  switch (kb_state) {

    //There are 2 possible states,
    // we are currently storing key inputs
    // or a read is unfinished so we should
    // finish it
    case (STORE):
      if (kbuffer.next == BUFF_SIZE) return;

      kbuffer.buff[kbuffer.head] = (char)ascii;
      kbuffer.next++;
      kbuffer.head++;
      kbuffer.head = kbuffer.head % BUFF_SIZE;
      break;

    //unfinished read mode
    case ( READ ):
      if(not_special(scan_code)){
        p_buff[p_index] = (char) ascii;
        p_index++;
      }
      //finished transfer length
      if(p_len == p_index){
        ready(process);
        kb_state = STORE;
        process->ret = p_index;

      // CTRL D pressed
      } else if (scan_code == 0x20 && state & INCTL){
        //unblock process
        ready(process);
        kb_state = STORE;
        process->ret = p_index;
        //close the keyboard at this point too
        kbd_close();

      // End of file has been reached
      } else if (ascii == eof || ascii == ENTER){
        //unblock process
        ready(process); 
        kb_state = STORE;
        process->ret = p_index;
      }
      break;

  }


  if (kb_echo) kprintf("%c", ascii);
}

/***************************
| name:       kbd_open
| desc:       resets the values of the
              keyboard device and then
              opens the keyboard for interrupts
****************************/
int kbd_open(pcb *p, int deviceno){
  int i;
  // clear the buffers
  for (i = 0; i < BUFF_SIZE; i++){
    kbuffer.buff[i] = 0;
  }

  //reset buffer pointers
  kbuffer.head = 0;
  kbuffer.tail = 0;
  kbuffer.next = 0;

  //reset the parameters
  eof = DEFAULT_EOF;
  kb_state = -1;

  //enable keyboard interrupts
  enable_irq(1,0);


  kb_echo = deviceno; //deviceno is 0 for no echo, 1 for echo
  kb_state = STORE;
  process = p;

  return 0;
}


/***************************
| name:       kbd_close
| desc:       disables interrupts
****************************/

int kbd_close( void ){
  enable_irq(1,1);
  return 0;
}



/***************************
| name:       kbd_read
| desc:       takes a process p
              and a buffer and bufflen
              reads from the buffer
              amount transferred is returned
****************************/

int kbd_read(pcb *p, void *buff, int bufflen){
  p_buff = (char*)buff;
  p_len = bufflen;
  p_index = 0;
  kb_state = READ;


  while(kbuffer.next != 0){
    p_buff[p_index] = (char) read_buffer(&kbuffer);
    p_index++;
    if(p_index == p_len){
      //did not need to block so ready immediately
      ready(p);
    }
  }
  // DII level read already checks for us right
  return p_index;
}

/***************************
| name:       kbd_write
| desc:       not supported, always fails
****************************/
int kbd_write(pcb* p, void* buff, int bufflen) {
  return -1;
}


/***************************
| name:       kbd_ioctl
| command:    changes state based on command
| vargs:      EOF indicates the end of the file index 
****************************/
int kbd_ioctl(unsigned long command, va_list vargs){
    if (command == KBD_EOF) {
        eof = va_arg(vargs, unsigned int);
        return 0;
    }
    else if (command == KBD_ECHO_ON) {
        kb_echo = 1;
        return 0;
    }
    else if (command == KBD_ECHO_OFF) {
        kb_echo = 0;
        return 0;
    }
    return -1;
}

// Used from the given txt with some formatting to compile
/*********************************************************/
static int extchar(unsigned char code) {
  state &= ~EXTENDED;
  return 0;
}


unsigned int kbtoa( unsigned char code )
{
  unsigned int    ch;
  
  if (state & EXTENDED)
    return extchar(code);
  if (code & KEY_UP) {
    switch (code & 0x7f) {
    case LSHIFT:
    case RSHIFT:
      state &= ~INSHIFT;
      break;
    case CAPSL:
      //printf("Capslock off detected\n");
      state &= ~CAPSLOCK;
      break;
    case LCTL:
      state &= ~INCTL;
      break;
    case LMETA:
      state &= ~INMETA;
      break;
    }
    
    return NOCHAR;
  }
  
  
  /* check for special keys */
  switch (code) {
  case LSHIFT:
  case RSHIFT:
    state |= INSHIFT;
    //printf("shift detected!\n");
    return NOCHAR;
  case CAPSL:
    state |= CAPSLOCK;
    //printf("Capslock ON detected!\n");
    return NOCHAR;
  case LCTL:
    state |= INCTL;
    return NOCHAR;
  case LMETA:
    state |= INMETA;
    return NOCHAR;
  case EXTESC:
    state |= EXTENDED;
    return NOCHAR;
  }
  
  ch = NOCHAR;
  
  if (code < sizeof(kbcode)){
    if ( state & CAPSLOCK )
      ch = kbshift[code];
    else
      ch = kbcode[code];
  }
  if (state & INSHIFT) {
    if (code >= sizeof(kbshift))
      return NOCHAR;
    if ( state & CAPSLOCK )
      ch = kbcode[code];
    else
      ch = kbshift[code];
  }
  if (state & INCTL) {
    if (code >= sizeof(kbctl))
      return NOCHAR;
    ch = kbctl[code];
  }
  if (state & INMETA)
    ch += 0x80;
  return ch;
}

/*
main() {
  kbtoa(LSHIFT);
  printf("45 = %c\n", kbtoa(45));
  kbtoa(LSHIFT | KEY_UP);
  printf("45 = %c\n", kbtoa(45));
}
*/

/******************************
|     HELPERS
*******************************/

// Read out a circular array buffer
char read_buffer(buffer *buffer){
  char data = buffer->buff[buffer->tail];
  buffer->next--;
  buffer->tail++;
  buffer->tail = buffer->tail % BUFF_SIZE;
  return data;
}

Bool not_special(unsigned char scan_code){
  return  !(scan_code & KEY_UP) && 
          scan_code != LSHIFT && 
          scan_code != RSHIFT && 
          scan_code != LMETA && 
          scan_code != LCTL && 
          scan_code != CAPSL;
}