/***** Here is example of script to build this application:
#!/bin/bash
rm -f m10xpon
rm -f m10xpoff
gcc -o m10xptgl m10xptgl.c
ln -s m10xptgl m10xpon
ln -s m10xptgl m10xpoff
cp -f m10xptgl /usr/local/sbin/m10xptgl
cp -f m10xpoff /usr/local/sbin/m10xpoff
cp -f m10xpon /usr/local/sbin/m10xpon
*****/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>

#define BCM2708_PERIPH_BASE   0x20000000
#define GPIO_BASE             (BCM2708_PERIPH_BASE + 0x200000)

#define GPIO_PIN_SYSREMOTE	17
//#define GPIO_PIN_OSCLSYNCH	25

// SR frame pulses (IR pulses without modulation)
#define HDR_MARK          8883  /*9000*/
#define HDR_PAUSE         4400  /*4500*/

#define BIT_MARK          476   /*560*/
#define ONE_PAUSE         1590  /*1680*/
#define ZERO_PAUSE        476   /*560*/

#define INTERFRAME_DELAY  9950

// M-10X SR control codes:
#define SR_TOGGLE_PWR     0xA55A38C7
#define SR_PWR_ON         0xA55A58A7
#define SR_PWR_OFF        0xA55AD827


int                  mem_fd, numRptCmd = 2;
long                 pgsz;
unsigned             cmd;
volatile unsigned    *gpio;
void                 *gpio_map;


// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g)       *(gpio + ((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g)       *(gpio + ((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio + (((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio + 7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio + 10) // clears bits which are 1 ignores bits which are 0

#define GPIO_READ(g) *(gpio + 13) &= (1<<(g)) //


void setup_io();
void sendPulse( long microsecs );
void sendCode( unsigned data, int nbits );


/*
uint64_t GetTimeStamp() {
    struct timeval tv;
    gettimeofday( &tv, NULL );
    return tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
} //GetTimeStamp
*/


int main(int argc, char **argv) {
    unsigned    j, m;

  if( 2 == argc ) numRptCmd = atoi(argv[1]);
  printf( "numRptCmd = %d\n", numRptCmd );

  printf( "callname = %s\n", argv[0] );
  cmd = SR_TOGGLE_PWR;
  if( NULL != strcasestr( argv[0], "m10xpon" ) ) cmd = SR_PWR_ON;
  if( NULL != strcasestr( argv[0], "m10xpoff" ) ) cmd = SR_PWR_OFF;

  nice( -19 );

  setup_io();

  INP_GPIO( GPIO_PIN_SYSREMOTE ); // must use INP_GPIO before we can use OUT_GPIO
  OUT_GPIO( GPIO_PIN_SYSREMOTE );

  //INP_GPIO( GPIO_PIN_OSCLSYNCH ); // must use INP_GPIO before we can use OUT_GPIO
  //OUT_GPIO( GPIO_PIN_OSCLSYNCH );

  //GPIO_SET = 1 << GPIO_PIN_OSCLSYNCH;

  for( j=0; j<=numRptCmd; j++ ) {
    sendCode( cmd, 32 );
    GPIO_CLR = 1 << GPIO_PIN_SYSREMOTE;
    usleep( INTERFRAME_DELAY );
  }

  GPIO_CLR = 1 << GPIO_PIN_SYSREMOTE;
  //GPIO_CLR = 1 << GPIO_PIN_OSCLSYNCH;

  munmap( gpio_map, pgsz );
  return 0;

} //main


void setup_io() {

  pgsz = sysconf( _SC_PAGESIZE );

  if((mem_fd = open("/dev/mem", O_RDWR|O_SYNC)) < 0) {
    printf("can't open /dev/mem\n");
    exit(-1);
  }

  gpio_map = mmap(
    NULL,                     // Any adddress in our space will do
    pgsz,                     // Map length
    PROT_READ|PROT_WRITE,     // Enable reading & writting to mapped memory
    MAP_SHARED,               // Shared with other processes
    mem_fd,                   // File to map
    GPIO_BASE                 // Offset to GPIO peripheral
  );

  close(mem_fd); // No need to keep mem_fd open after mmap

  if (gpio_map == MAP_FAILED) {
    printf( "mmap error %d\n", (int)gpio_map ); //errno also set!
    exit(-1);
  }

  gpio = (volatile unsigned *)gpio_map; // Always use volatile pointer!

} //setup_io


void sendPulse( long microsecs ) {
  GPIO_SET = 1 << GPIO_PIN_SYSREMOTE;
  usleep( microsecs );
  GPIO_CLR = 1 << GPIO_PIN_SYSREMOTE;
} //sendPulse


void sendCode( unsigned data, int nbits ) {
    int         i;
    uint64_t	t1, t2;

  sendPulse( HDR_MARK );
  usleep( HDR_PAUSE );

  for ( i = 0; i < nbits; i++ ) {
    sendPulse( BIT_MARK );
    if( data & 0x80000000 ) usleep( ONE_PAUSE );
    else usleep( ZERO_PAUSE );
    data <<= 1;
  }

  sendPulse( BIT_MARK ); // !!! stop-bit !!!

} //sendCode
