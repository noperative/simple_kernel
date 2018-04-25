#include <xeroskernel.h>

// Used from the given txt with some formatting to compile
/*********************************************************/

#define KEY_UP   0x80            /* If this bit is on then it is a key   */
                                 /* up event instead of a key down event */
 
/* Control code */
#define LSHIFT  0x2a
#define RSHIFT  0x36
#define LMETA   0x38
 
#define LCTL    0x1d
#define CAPSL   0x3a
 
 
/* scan state flags */
#define INCTL           0x01    /* control key is down          */
#define INSHIFT         0x02    /* shift key is down            */
#define CAPSLOCK        0x04    /* caps lock mode               */
#define INMETA          0x08    /* meta (alt) key is down       */
#define EXTENDED        0x10    /* in extended character mode   */

#define EXTESC          0xe0    /* extended character escape    */
#define NOCHAR  256

static  int     state; /* the state of the keyboard */
 
/*  Normal table to translate scan code  */
unsigned char   kbcode[] = { 0,
          27,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',
         '0',  '-',  '=', '\b', '\t',  'q',  'w',  'e',  'r',  't',
         'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n',    0,  'a',
         's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'',
         '`',    0, '\\',  'z',  'x',  'c',  'v',  'b',  'n',  'm',
         ',',  '.',  '/',    0,    0,    0,  ' ' };
 
/* captialized ascii code table to tranlate scan code */
unsigned char   kbshift[] = { 0,
           0,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',
         ')',  '_',  '+', '\b', '\t',  'Q',  'W',  'E',  'R',  'T',
         'Y',  'U',  'I',  'O',  'P',  '{',  '}', '\n',    0,  'A',
         'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  '"',
         '~',    0,  '|',  'Z',  'X',  'C',  'V',  'B',  'N',  'M',
         '<',  '>',  '?',    0,    0,    0,  ' ' };
/* extended ascii code table to translate scan code */
unsigned char   kbctl[] = { 0,
           0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
           0,   31,    0, '\b', '\t',   17,   23,    5,   18,   20,
          25,   21,    9,   15,   16,   27,   29, '\n',    0,    1,
          19,    4,    6,    7,    8,   10,   11,   12,    0,    0,
           0,    0,   28,   26,   24,    3,   22,    2,   14,   13 };

/**********************************************************************/
#define ENTER           10
#define DEFAULT_EOF     59
#define BUFF_SIZE       4
#define KBD_PORT        0x60
#define KBD_EOF         53
#define KBD_ECHO_OFF    55
#define KBD_ECHO_ON     56

//state
#define STORE           0
#define READ            1


// The buffer will be a circular
// array in order to maintain FIFO
// order on the key inputs
typedef struct buffer {
  char buff[BUFF_SIZE];
  int head;
  int tail;
  int next;
} buffer;


unsigned int kbtoa( unsigned char code );
int kbd_write(pcb* p, void* buff, int bufflen);
int kbd_open(pcb *p, int deviceno);
int kbd_close( void );
int kbd_ioctl(unsigned long command, va_list vargs);
int kbd_read(pcb *p, void *buff, int bufflen);