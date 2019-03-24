
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/poll.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

static volatile int keepRunning = 1;
static int has_notified = 0;

void intHandler(int dummy) {
    keepRunning = 0;
}

#define RPMSG_HDR_SIZE 16
#define RPMSG_PAYLOAD_SIZE 512-RPMSG_HDR_SIZE // number of bytes to send per transmission

#define HEARTBEAT_PERIOD 50
#define DEST_HOSTNAME "192.168.7.1"
#define DEST_PORT "1337"

#define MAX_BUFFER_SIZE 512
char readBuf[MAX_BUFFER_SIZE];
#define DEVICE_NAME1 "/dev/rpmsg_pru31"

int main(void)
{
 struct pollfd pollfds;
 int result = 0;

 int pru_data;
 int heartbeat_cnt = HEARTBEAT_PERIOD;
 int packet_cnt = 0;
 struct addrinfo hints;
 struct addrinfo* res=0;

 if(getuid() != 0) {
  printf("Please run this as root\n");
  return -1;
 }

 signal(SIGINT, intHandler); // stop on ctrl-c

  while(keepRunning) {
    /* Open the rpmsg_pru character device file */
    pollfds.fd = open(DEVICE_NAME1, O_RDWR);
    if(pollfds.fd < 0) {
      printf("Failed to open %s, retry in 30 seconds..\n",DEVICE_NAME1);
      usleep(30*1000*1000);
    } else {
      break;
    }
  }
  if(!keepRunning) return 0;

  printf("Successfully opened %s\n",DEVICE_NAME1);

  /* init UDP */
  memset(&hints,0,sizeof(hints));
  hints.ai_family=AF_INET;
  hints.ai_socktype=SOCK_DGRAM;
  hints.ai_protocol=0;
  hints.ai_flags=AI_ADDRCONFIG;

  int err=getaddrinfo(DEST_HOSTNAME,DEST_PORT,&hints,&res);
  if (err!=0) {
    printf("Failed to resolve remote socket address (err=%d)",err);
    return -1;
  } else {

  }

  int fd = socket(res->ai_family,res->ai_socktype,res->ai_protocol);
  if (fd == -1) {
    printf("Failed to open socket\n");
    return -1;
  } else {
    printf("Opened %s\n",DEST_HOSTNAME);
  }

  /* Instruct PRU to start sending data */
  result = write(pollfds.fd, "start", 13);
  printf("Initializing hardware..\n");

 while(keepRunning) {

  if(--heartbeat_cnt == 0) {
    /* let the PRU know that we are ready to receive data. Without this, it will stop sending data after a while */
    result = write(pollfds.fd, "alive", 13);
    heartbeat_cnt = HEARTBEAT_PERIOD;
  }

  result = read(pollfds.fd, readBuf, MAX_BUFFER_SIZE);
  if(!has_notified) {
    printf("Read first data buffer, further messages will be suppressed\n");
    has_notified = 1;
  }

  if (result > 0)
    packet_cnt++;
    if (sendto(fd,readBuf,RPMSG_PAYLOAD_SIZE,0,
        res->ai_addr,res->ai_addrlen)==-1) {
        printf("Failed to send\n");
    }

 }
 printf("\nClosing %s\n",DEVICE_NAME1);
 close(pollfds.fd);
 return 0;
}
