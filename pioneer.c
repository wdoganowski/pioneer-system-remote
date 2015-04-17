/*
gcc -o x_pigpiod_if x_pigpiod_if.c -lpigpiod_if -lrt -lpthread
./x_pigpiod_if
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <pigpiod_if.h>

#define GPIO 4

#define TIME_START_0	9000
#define TIME_START_1	4500
#define TIME_LOW_0	560
#define TIME_HIGH_0	560
#define TIME_LOW_1	560
#define TIME_HIGH_1	1680

gpioPulse_t wf_start_bit[] =
{
   {0, 0, 0}, // delay
   {1<<GPIO, 0, TIME_START_0},
   {0, 1<<GPIO, TIME_START_1},
};

gpioPulse_t wf_stop_bit[] =
{
   {0, 0, 100000}, // delay
   {1<<GPIO, 0, TIME_LOW_0},
   {0, 1<<GPIO, TIME_HIGH_0},
};

gpioPulse_t wf_data[1+(4*8*2)] =
{
   {0, 0, TIME_START_0 + TIME_START_1} // delay of start bit
};

int prepare(int start, uint8_t data)
{
   int i, j, size = 0;

   j = 1 + (8*2 * start);

   for (i=0; i<8; i++)
   {
     if (data & 0x80)
     { // 1
       //printf("1");
       wf_data[j].gpioOn = 1<<GPIO;
       wf_data[j].usDelay = TIME_LOW_1;
       size += wf_data[j].usDelay;
       j++;
       wf_data[j].gpioOff = 1<<GPIO;
       wf_data[j].usDelay = TIME_HIGH_1;
       size += wf_data[j].usDelay;
       j++;
     } else
     { // 0
       //printf("0");
       wf_data[j].gpioOn = 1<<GPIO;
       wf_data[j].usDelay = TIME_LOW_0;
       size += wf_data[j].usDelay;
       j++;
       wf_data[j].gpioOff = 1<<GPIO;
       wf_data[j].usDelay = TIME_HIGH_0;
       size += wf_data[j].usDelay;
       j++;
     }
     data<<=1;
   }
   //printf("\n%d\n", size);

   return size;
}

void transmit(uint8_t device_id, uint8_t command_id)
{
   int e, delay, wave;

   set_mode(GPIO, PI_OUTPUT);

   e = wave_clear();
   if (e<0) {
     fprintf(stderr, "wave_clear error %d.\n", e);
     return;
   }

   e = wave_add_generic(sizeof(wf_start_bit)/sizeof(gpioPulse_t), wf_start_bit);
   if (e<0) {
     fprintf(stderr, "wave_add error %d.\n", e);
     return;
   }

   delay = wf_data[0].usDelay;
   delay += prepare(0, device_id);
   delay += prepare(1, ~device_id);
   delay += prepare(2, command_id);
   delay += prepare(3, ~command_id);
   wf_stop_bit[0].usDelay = delay;

   e = wave_add_generic(sizeof(wf_data)/sizeof(gpioPulse_t), wf_data);
   if (e<0) {
     fprintf(stderr, "wave_add error %d.\n", e);
     return;
   }

   e = wave_add_generic(sizeof(wf_stop_bit)/sizeof(gpioPulse_t), wf_stop_bit);
   if (e<0) {
     fprintf(stderr, "wave_add error %d.\n", e);
     return;
   }

   wave = wave_create();
   if (wave<0) {
     fprintf(stderr, "wave_create error %d.\n", wave);
     return;
   }

   e = wave_send_once(wave);
   if (e<0) {
     fprintf(stderr, "wave_add error %d.\n", e);
     return;
   }

   e = wave_delete(wave);
   if (e<0) {
     fprintf(stderr, "wave_add error %d.\n", e);
     return;
   }
}

int main(int argc, char *argv[])
{
   int status;
   uint8_t  device_id=0xa5, command_id=0x38;

   printf("Content-type: text/html\n\n");	// Tell the browser the type of document being sent
   printf("<html>\n<head>\n<title>Pioneer Remote</title>\n</head>\n<body>\n");

   if (argc!=2)
   {
      printf("<p>Usage: %s?cmd where xx is pwr, power, toggle, on, off, vol+, vol-</p>\n</body>\n</html>\n", argv[0]);
      return -1;
   } else {
     if (     strcmp( argv[1], "pwr" ) == 0)	  command_id = 0x38;
     else if (strcmp( argv[1], "power" ) == 0)    command_id = 0x38;
     else if (strcmp( argv[1], "toggle" ) == 0)   command_id = 0x38;
     else if (strcmp( argv[1], "on" )   == 0)	  command_id = 0x58;
     else if (strcmp( argv[1], "off" )  == 0)	  command_id = 0xd8;
     else if (strcmp( argv[1], "vol+" ) == 0)	  command_id = 0x50;
     else if (strcmp( argv[1], "up" )   == 0)	  command_id = 0x50;
     else if (strcmp( argv[1], "vol-" ) == 0)	  command_id = 0xD0;
     else if (strcmp( argv[1], "down" ) == 0)	  command_id = 0xD0;
   }

   printf("<p>Executing command %s 0x%02x%02x</p>\n", argv[1], device_id, command_id);

   status = pigpio_start(0, 0);

   if (status < 0)
   {
      fprintf(stderr, "<p>pigpio initialisation failed</p>\n");
      return 1;
   }

   //printf("<p>Connected to pigpio daemon</p>\n");

   gpio_write(5, 1);
   gpio_write(GPIO, 0);
   time_sleep(0.01);

   transmit(device_id, command_id);

   time_sleep(0.1);
   gpio_write(5, 0);

   pigpio_stop();

   printf("</body>\n</html>\n");

   return 0;
}

