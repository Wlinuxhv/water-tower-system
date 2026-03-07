/*
 * STC8G1K08 头文件 - 简化版
 * 提供基本的寄存器定义
 */

#ifndef STC8G1K08_H
#define STC8G1K08_H

#include <8051.h>

// 特殊功能寄存器定义
sfr P0M0 = 0x94;
sfr P0M1 = 0x93;
sfr P1M0 = 0x92;
sfr P1M1 = 0x91;
sfr P2M0 = 0x96;
sfr P2M1 = 0x95;
sfr P3M0 = 0xB2;
sfr P3M1 = 0xB1;
sfr P4M0 = 0xB4;
sfr P4M1 = 0xB3;

// ADC 相关
sfr ADC_CONTR = 0xBC;
sfr ADC_RES = 0xBD;
sfr ADC_RESL = 0xBE;

// 中断相关
sfr IE = 0xA8;
sfr IE2 = 0xAF;
sfr IP = 0xB8;
sfr IP2 = 0xB7;

// 定时器相关
sfr TCON = 0x88;
sfr TMOD = 0x89;
sfr TL0 = 0x8A;
sfr TL1 = 0x8B;
sfr TH0 = 0x8C;
sfr TH1 = 0x8D;

// 串口相关
sfr SCON = 0x98;
sfr SBUF = 0x99;
sfr S2CON = 0x9A;
sfr S2BUF = 0x9B;

// 系统控制
sfr PCON = 0x87;
sfr AUXR = 0x8E;
sfr AUXR2 = 0x8F;
sfr CLK_DIV = 0x97;

// 看门狗
sfr WDT_CONTR = 0xC1;

// GPIO 位定义
sbit P00 = P0^0; sbit P01 = P0^1; sbit P02 = P0^2; sbit P03 = P0^3;
sbit P04 = P0^4; sbit P05 = P0^5; sbit P06 = P0^6; sbit P07 = P0^7;

sbit P10 = P1^0; sbit P11 = P1^1; sbit P12 = P1^2; sbit P13 = P1^3;
sbit P14 = P1^4; sbit P15 = P1^5; sbit P16 = P1^6; sbit P17 = P1^7;

sbit P20 = P2^0; sbit P21 = P2^1; sbit P22 = P2^2; sbit P23 = P2^3;
sbit P24 = P2^4; sbit P25 = P2^5; sbit P26 = P2^6; sbit P27 = P2^7;

sbit P30 = P3^0; sbit P31 = P3^1; sbit P32 = P3^2; sbit P33 = P3^3;
sbit P34 = P3^4; sbit P35 = P3^5; sbit P36 = P3^6; sbit P37 = P3^7;

sbit P40 = P4^0; sbit P41 = P4^1; sbit P42 = P4^2; sbit P43 = P4^3;
sbit P44 = P4^4; sbit P45 = P4^5; sbit P46 = P4^6; sbit P47 = P4^7;

#endif // STC8G1K08_H
