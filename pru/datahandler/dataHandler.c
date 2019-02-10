#include <stdint.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include <rsc_types.h>
#include <pru_rpmsg.h>
#include "resource_table.h"


#pragma DATA_SECTION(RX_DATA_BUF, ".RX_DATA_BUF")
#pragma DATA_SECTION(rpmsg_buf, ".RPMSG_BUF")
#define data_buf_size 8192/sizeof(uint32_t) /* Using entire data ram for PRU1 */
far uint32_t RX_DATA_BUF;
far uint32_t rpmsg_buf[data_buf_size];

volatile register uint32_t __R30;
volatile register uint32_t __R31;

/* Defines */
#define PRU1

/* Hosts 0 and 1 are automatically mapped to bits 30 and 31, respectively, in the r31 register.
   To view the status of Host 1 we mask out the lower 31 bits. */
#define HOST1_MASK      (0x80000000)
#define PRU0_PRU1_EVT   (16)

/* The PRU-ICSS system events used for RPMsg are defined in the Linux device tree
 * PRU0 uses system event 16 (To ARM) and 17 (From ARM)
 * PRU1 uses system event 18 (To ARM) and 19 (From ARM) */
#define TO_ARM_HOST     18
#define FROM_ARM_HOST   19

#define CHAN_NAME     "rpmsg-pru"

#define CHAN_DESC     "Channel 31"
#define CHAN_PORT     31

#define RPMSG_HDR_SIZE 16
#define RPMSG_PAYLOAD_SIZE 512-RPMSG_HDR_SIZE // number of bytes to send per transmission

/*
 * Used to make sure the Linux drivers are ready for RPMsg communication
 * Found at linux-x.y.z/include/uapi/linux/virtio_config.h
 */
#define VIRTIO_CONFIG_S_DRIVER_OK 4



uint8_t payload[16];

struct pru_rpmsg_transport transport;
uint16_t src, dst, len;
volatile uint8_t *status;

void send_buffer(uint32_t* buf) {
  int base = 0;
  for(base=0;base<data_buf_size*4;base+=RPMSG_PAYLOAD_SIZE) {
    __delay_cycles(100000);
    __R30 = 0xffff;
    *(rpmsg_buf+base) = base;
    while(pru_rpmsg_send(&transport, dst, src, (uint32_t*) (rpmsg_buf+base/4), RPMSG_PAYLOAD_SIZE) != PRU_RPMSG_SUCCESS);
    __R30 = 0x0000;
  }
}

void main(void)
{
  int heartbeat_cnt;
  uint32_t idx; // Where to store the next word

  /* Configure GPI and GPO as Mode 0 (Direct Connect) */
  CT_CFG.GPCFG0 = 0x0000;

  /* Allow OCP master port access by the PRU so the PRU can read external memories */
  CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

  /* Clear the status of the PRU-ICSS system event that the ARM will use to 'kick' us */
  CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

  /* Make sure the Linux drivers are ready for RPMsg communication */
  status = &resourceTable.rpmsg_vdev.status;
  while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK));

  /* Initialize the RPMsg transport structure */
  pru_rpmsg_init(&transport, &resourceTable.rpmsg_vring0, &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST);

  /* Create the RPMsg channel between the PRU and ARM user space using the transport structure. */
  while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME, CHAN_DESC, CHAN_PORT) != PRU_RPMSG_SUCCESS);

  __R30 = 0x0000;

  // ARM must send a single message to initialize the channel before we start the main loop
  //while (pru_rpmsg_receive(&transport, &src, &dst, payload, &len) != PRU_RPMSG_SUCCESS);

  __R30 = 0xffff;

  idx = 0;
  heartbeat_cnt = 10;
  while (1) {
    if (__R31 & HOST1_MASK) {

      /* Clear interrupt event (event 16)*/
      CT_INTC.SICR = 16;

      // IMPORTANT: Delay to avoid race condition when clearing interrupt
      __delay_cycles(10);

      // write data word to memory buffer
      rpmsg_buf[idx] = RX_DATA_BUF;

      idx += 1;
      if(idx==data_buf_size) {
        idx = 0;

//        rpmsg_buf[0] = 0xEFBEADDE;

        /* stop sending after a while if we dont get a heartbeat from the host. If the host
           is not ready to receive we will kill the pru driver with the we generate */
        if(pru_rpmsg_receive(&transport, &src, &dst, payload, &len) == PRU_RPMSG_SUCCESS) {
          heartbeat_cnt = 10;
        }

        if(heartbeat_cnt-- > 0) send_buffer(rpmsg_buf);
      }
      //__R30 = idx==0 ? 0x0000 : 0xffff;
    }
  }
}

