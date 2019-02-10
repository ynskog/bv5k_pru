
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/poll.h>
#include <signal.h>

static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

#define HEARTBEAT_PERIOD 50

#define MAX_BUFFER_SIZE 512
char readBuf[MAX_BUFFER_SIZE];
#define DEVICE_NAME1 "/dev/rpmsg_pru31"
int main(void)
{
 struct pollfd pollfds;
 int result = 0;

 int pru_data;
 int heartbeat_cnt = HEARTBEAT_PERIOD;

  signal(SIGINT, intHandler); // stop on ctrl-c

 /* Open the rpmsg_pru character device file */
 pollfds.fd = open(DEVICE_NAME1, O_RDWR);
 /*
 * If the RPMsg channel doesn't exist yet the character device
 * won't either.
 * Make sure the PRU firmware is loaded and that the rpmsg_pru
 * module is inserted.
 */
 if (pollfds.fd < 0) {
 printf("Failed to open %s\n",DEVICE_NAME1);
 return -1;
 } else {
  printf("Successfully opened %s\n",DEVICE_NAME1);
 }

result = write(pollfds.fd, "start", 13);

 while(keepRunning) {

  if(--heartbeat_cnt == 0) {
    /* let the PRU know that we are ready to receive data. without this, it will stop sending data after a while */
    result = write(pollfds.fd, "alive", 13);
    heartbeat_cnt = HEARTBEAT_PERIOD;
  }

  result = read(pollfds.fd, readBuf, MAX_BUFFER_SIZE);
  if (result > 0)

  for(int idx=0;idx<4;idx++)
    printf("%x",readBuf[idx]);
}
 printf("\nClosing %s\n",DEVICE_NAME1);
 close(pollfds.fd);
 return 0;
}
