#ifndef VLCA_I2C_H
#define VLCA_I2C_H

#include <stdint.h>

typedef enum I2C_ModeEnum {
    IDLE_MODE,
    NACK_MODE,
    TX_REG_ADDRESS_MODE,
    TX_REG_ADDRESS_MODE2,
    TX_DATA_MODE,
    RX_DATA_MODE,
    SWITCH_TO_RX_MODE,
    SWITHC_TO_TX_MODE,
    TIMEOUT_MODE
} I2C_Mode;

void i2c_init(void);

I2C_Mode I2C_Master_WriteReg(uint8_t dev_addr, uint16_t reg_addr, uint8_t *reg_data, uint8_t count);
I2C_Mode I2C_Master_ReadReg(uint8_t dev_addr, uint16_t reg_addr, uint8_t *reg_data, uint8_t count);

#endif // VLCA_I2C_H

