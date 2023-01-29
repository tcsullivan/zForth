#include "i2c.h"

#include <msp430g2553.h>

#include <stddef.h>

static I2C_Mode MasterMode = IDLE_MODE;
static uint16_t TransmitRegAddr;
static uint8_t *Buffer;
static volatile uint8_t RXByteCtr;
static volatile uint8_t TXByteCtr;

void i2c_init(void)
{
    P1SEL |= BIT6 | BIT7;
    P1SEL2 |= BIT6 | BIT7;

    UCB0CTL1 |= UCSWRST;                      // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB0BR0 = 10/*160*/;                            // fSCL = SMCLK/160 = ~100kHz
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
    UCB0I2CIE |= UCNACKIE;
}

static void I2C_Master_BeginTransfer(uint8_t dev_addr, uint16_t reg_addr)
{
    MasterMode = TX_REG_ADDRESS_MODE;
    TransmitRegAddr = reg_addr;
    UCB0I2CSA = dev_addr;

    IFG2 &= ~(UCB0TXIFG | UCB0RXIFG); // Clear any pending interrupts
    IE2 &= ~UCB0RXIE;
    IE2 |= UCB0TXIE;
    UCB0CTL1 |= UCTR + UCTXSTT;
}

I2C_Mode I2C_Master_ReadReg(uint8_t dev_addr, uint16_t reg_addr, uint8_t *reg_data, uint8_t count)
{
    Buffer = reg_data;
    TXByteCtr = 0;
    RXByteCtr = count;

    I2C_Master_BeginTransfer(dev_addr, reg_addr);

    while (RXByteCtr != 0)
        __no_operation();

    return MasterMode;

}

I2C_Mode I2C_Master_WriteReg(uint8_t dev_addr, uint16_t reg_addr, uint8_t *reg_data, uint8_t count)
{
    Buffer = reg_data;
    TXByteCtr = count;
    RXByteCtr = 0;

    I2C_Master_BeginTransfer(dev_addr, reg_addr);

    while (TXByteCtr != 0)
        __no_operation();

    return MasterMode;
}

// I2C Interrupt For Received and Transmitted Data
__interrupt_vec(USCIAB0TX_VECTOR)
static void USCIAB0TX_ISR(void)
{
  if (IFG2 & UCB0RXIFG)                 // Receive Data Interrupt
  {
      uint8_t rx_val = UCB0RXBUF;

      if (RXByteCtr)
      {
          *Buffer++ = rx_val;
          RXByteCtr--;
      }

      if (RXByteCtr == 1)
      {
          UCB0CTL1 |= UCTXSTP;
      }
      else if (RXByteCtr == 0)
      {
          IE2 &= ~UCB0RXIE;
          MasterMode = IDLE_MODE;
      }
  }
  else if (IFG2 & UCB0TXIFG)            // Transmit Data Interrupt
  {
      switch (MasterMode)
      {
          case TX_REG_ADDRESS_MODE:
              UCB0TXBUF = TransmitRegAddr >> 8;
              MasterMode = TX_REG_ADDRESS_MODE2;
              break;

          case TX_REG_ADDRESS_MODE2:
              UCB0TXBUF = TransmitRegAddr & 0xFF;
              if (RXByteCtr)
                  MasterMode = SWITCH_TO_RX_MODE;   // Need to start receiving now
              else
                  MasterMode = TX_DATA_MODE;        // Continue to transmision with the data in Transmit Buffer
              break;

          case SWITCH_TO_RX_MODE:
              IE2 |= UCB0RXIE;              // Enable RX interrupt
              IE2 &= ~UCB0TXIE;             // Disable TX interrupt
              UCB0CTL1 &= ~UCTR;            // Switch to receiver
              MasterMode = RX_DATA_MODE;    // State state is to receive data
              UCB0CTL1 |= UCTXSTT;          // Send repeated start
              if (RXByteCtr == 1)
              {
                  //Must send stop since this is the N-1 byte
                  while((UCB0CTL1 & UCTXSTT));
                  UCB0CTL1 |= UCTXSTP;      // Send stop condition
              }
              break;

          case TX_DATA_MODE:
              if (TXByteCtr)
              {
                  UCB0TXBUF = *Buffer++;
                  TXByteCtr--;
              }
              else
              {
                  //Done with transmission
                  UCB0CTL1 |= UCTXSTP;     // Send stop condition
                  MasterMode = IDLE_MODE;
                  IE2 &= ~UCB0TXIE;                       // disable TX interrupt
              }
              break;

          default:
              __no_operation();
              break;
      }
  }
}

// I2C Interrupt For Start, Restart, Nack, Stop
// TODO is this necessary?
__interrupt_vec(USCIAB0RX_VECTOR)
static void USCIAB0RX_ISR(void)
{
    if (UCB0STAT & UCNACKIFG)
        UCB0STAT &= ~UCNACKIFG;
    if (UCB0STAT & UCSTPIFG)
        UCB0STAT &= ~(UCSTTIFG | UCSTPIFG | UCNACKIFG);
    if (UCB0STAT & UCSTTIFG)
        UCB0STAT &= ~(UCSTTIFG);
}

