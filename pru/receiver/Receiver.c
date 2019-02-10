#include <stdint.h>
#include <pru_cfg.h>
#include "pru_intc.h"
#include "resource_table.h"

#pragma DATA_SECTION(RX_DATA_BUF, ".RX_DATA_BUF")
far uint32_t RX_DATA_BUF;

volatile register uint16_t __R30;
volatile register uint16_t __R31;

/* ===================================
 * SYSTEM PARAMTERS
 * =================================== */

#define PRU0

/* PRU0-to-PRU1 interrupt */
#define PRU0_PRU1_EVT       (16)
#define PRU0_PRU1_TRIGGER   (__R31 = (PRU0_PRU1_EVT - 16) | (1 << 5))

/* ===================================
 * APPLICATION SPECIFIC PARAMTERS
 * =================================== */
#define BLOCKSIZE 8192/4 // block size in 32 bit words
#define BIT_DELAY 2

/* SW1 offset */

/* INTC configuration
 * We are going to map User event 16 to Host 1
 * PRU1 will then wait for r31 bit 31 (designates Host 1) to go high
 * */
void configIntc(void)
{
    /* Clear any pending PRU-generated events */
    __R31 = 0x00000000;

    /* Map event 16 to channel 1 */
    CT_INTC.CMR4_bit.CH_MAP_16 = 1;

    /* Map channel 1 to host 1 */
    CT_INTC.HMR0_bit.HINT_MAP_1 = 1;

    /* Ensure event 16 is cleared */
    CT_INTC.SICR = 16;

    /* Enable event 16 */
    CT_INTC.EISR = 16;

    /* Enable Host interrupt 1 */
    CT_INTC.HIEISR |= (1<<0);

    // Globally enable host interrupts
    CT_INTC.GER = 1;
}

// Write 0xF8 to slave to initialize block transfer
inline void initBlockTransfer()
{
    __R30 |= 0x0040;  __delay_cycles(BIT_DELAY);

    __R30 |= 0x0044; __delay_cycles(BIT_DELAY);
    __R30 &= 0xfffb;  __delay_cycles(BIT_DELAY);

    __R30 |= 0x0044; __delay_cycles(BIT_DELAY);
    __R30 &= 0xfffb;  __delay_cycles(BIT_DELAY);

    __R30 |= 0x0044; __delay_cycles(BIT_DELAY);
    __R30 &= 0xfffb;  __delay_cycles(BIT_DELAY);

    __R30 |= 0x0044; __delay_cycles(BIT_DELAY);
    __R30 &= 0xfffb;  __delay_cycles(BIT_DELAY);

    __R30 |= 0x0044; __delay_cycles(BIT_DELAY);
    __R30 &= 0xffbb;  __delay_cycles(BIT_DELAY);

    __R30 |= 0x0004; __delay_cycles(BIT_DELAY);
    __R30 &= 0xfffb;  __delay_cycles(BIT_DELAY);

    __R30 |= 0x0004; __delay_cycles(BIT_DELAY);
    __R30 &= 0xfffb;  __delay_cycles(BIT_DELAY);

    __R30 |= 0x0004; __delay_cycles(BIT_DELAY);
    __R30 &= 0xfffb;  __delay_cycles(BIT_DELAY);

}

// Read a single word into rxbuf
uint32_t rx_word;
inline void rxWord()
{
  int i;
  for(i=0;i<32;i++) {
    __R30 = 0x0004;
    rx_word = (rx_word << 1) | ((__R31 & 0x8) != 0);
    __R30 = 0x0000;
  }
}

void main(){
    int iter;

    /* GPI Mode 0, GPO Mode 0 */
    CT_CFG.GPCFG0 = 0;

   /* Clear GPO pins */
    __R30 = 0x0000;

    /* Configure interrupt controller */
    configIntc();

    while(1) {
        while( (__R31 & 0x20)==0 ); // Wait for data ready

        initBlockTransfer();
	   __delay_cycles(10);

        for(iter=0;iter<BLOCKSIZE;iter++) {
            rxWord();
            RX_DATA_BUF = rx_word;
            __delay_cycles(5);
            PRU0_PRU1_TRIGGER; // Trigger PRU1 interrupt
        }
    }
}
