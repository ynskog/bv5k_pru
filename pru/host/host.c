
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

#define MAX_BUFFER_SIZE 512
char readBuf[MAX_BUFFER_SIZE];
#define DEVICE_NAME1 "/dev/rpmsg_pru31"
int main(void)
{
 struct pollfd pollfds;
 int result = 0;

 int pru_data;

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

 /* Send something to create the communication channel, doesnt matter what we send. PRU will block until receiving this message */
 result = write(pollfds.fd, "Start", 13);

 /* Poll until we receive a message from the PRU and then print it */
 while(keepRunning) {
  result = read(pollfds.fd, readBuf, MAX_BUFFER_SIZE);
  if (result > 0)
  for(int idx=0;idx<4;idx++)
    printf("%x",readBuf[idx]);
}
 close(pollfds.fd);
 return 0;
}
