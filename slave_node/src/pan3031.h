/*
 * PAN3031 LoRa 驱动 - STC32G 版
 */

#ifndef PAN3031_H
#define PAN3031_H

#include <STC32G.H>
#include <intrins.h>

// 引脚定义 (根据实际连接修改)
sbit PAN3031_CS   = P2^3;
sbit PAN3031_MOSI = P2^2;
sbit PAN3031_MISO = P2^1;
sbit PAN3031_SCK  = P2^0;
sbit PAN3031_IRQ  = P3^2;

// 寄存器定义
#define REG_FIFO            0x00
#define REG_OP_MODE         0x01
#define REG_FRF_MSB         0x06
#define REG_FRF_MID         0x07
#define REG_FRF_LSB         0x08
#define REG_PA_CONFIG       0x09
#define REG_LNA             0x0C
#define REG_FIFO_ADDR_PTR   0x0D
#define REG_FIFO_TX_BASE    0x0E
#define REG_FIFO_RX_BASE    0x0F
#define REG_FIFO_RX_ADDR    0x10
#define REG_IRQ_FLAGS       0x12
#define REG_RX_NB_BYTES     0x13
#define REG_PKT_SNR         0x19
#define REG_PKT_RSSI        0x1A
#define REG_MODEM_CONFIG1   0x1D
#define REG_MODEM_CONFIG2   0x1E
#define REG_PREAMBLE        0x20
#define REG_PAYLOAD_LEN     0x22
#define REG_MODEM_CONFIG3   0x26
#define REG_SYNC_WORD       0x39

// 工作模式
#define MODE_SLEEP          0x00
#define MODE_STDBY          0x01
#define MODE_FSTX           0x02
#define MODE_TX             0x03
#define MODE_FSRX           0x04
#define MODE_RXCONT         0x05
#define MODE_RXSINGLE       0x06

// 命令字定义
#define CMD_HEARTBEAT   0x01
#define CMD_QUERY       0x02
#define CMD_PUMP_CTRL   0x10
#define CMD_SET_AUTO    0x20
#define CMD_SET_MANUAL  0x21
#define CMD_ALARM       0xFF

// 函数声明
void pan3031_init(void);
void pan3031_write_reg(uint8_t addr, uint8_t value);
uint8_t pan3031_read_reg(uint8_t addr);
void pan3031_set_freq(uint32_t freq);
void pan3031_set_sf(uint8_t sf);
void pan3031_set_bw(uint32_t bw);
void pan3031_set_power(uint8_t power);
void pan3031_send(uint8_t *data, uint8_t len);
uint8_t pan3031_receive(uint8_t *data, uint8_t *len);
void pan3031_sleep(void);

// 延时函数
void delay_us(uint16_t us);
void delay_ms(uint16_t ms);
uint32_t millis(void);

#endif // PAN3031_H
