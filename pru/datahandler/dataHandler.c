#include <stdint.h>
#include <pru_cfg.h>
#include "pru_intc.h"
#include "resource_table.h"

#pragma DATA_SECTION(RX_DATA_BUF, ".RX_DATA_BUF")
#pragma DATA_SECTION(rpmsg_buf, ".RPMSG_BUF")
far uint32_t RX_DATA_BUF; // Data received from PRU0

#define rpmsg_buf_size 8192/sizeof(uint32_t) /* Using entire data ram for PRU1 */
far uint32_t rpmsg_buf[rpmsg_buf_size]; // Storage for data read from the frontend

volatile register uint32_t __R30;
volatile register uint32_t __R31;

/* Defines */
#define PRU1

/* Hosts 0 and 1 are automatically mapped to bits 30 and 31, respectively, in the r31 register.
   To view the status of Host 1 we mask out the lower 31 bits. */
#define HOST1_MASK      (0x80000000)
#define PRU0_PRU1_EVT   (16)

// LED is on PRU1 pin 10 (P1.35)
#define TOGGLE_BLUE (__R30 ^= (1 << 14))

uint32_t idx; // Where to store the next word

void main(void)
{
  /* Configure GPI and GPO as Mode 0 (Direct Connect) */
  CT_CFG.GPCFG0 = 0x0000;

  /* Clear GPO pins */
  __R30 = 0x0000;

  idx = 0;

  while (1) {
    if (__R31 & HOST1_MASK) {

      /* Clear interrupt event (event 16)*/
      CT_INTC.SICR = 16;

      // IMPORTANT: Delay to avoid race condition when clearing interrupt
      __delay_cycles(5);

      // write data word to memory buffer
      rpmsg_buf[idx] = RX_DATA_BUF;
      idx += 1;
      if(idx==rpmsg_buf_size) idx = 0;
      __R30 = idx==0 ? 0x0000 : 0xffff;
    }
  }
}

