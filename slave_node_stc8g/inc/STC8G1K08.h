/*
 * STC8G1K08 头文件 - 简化版
 * 提供基本的寄存器定义
 */

#ifndef STC8G1K08_H
#define STC8G1K08_H

// 特殊功能寄存器定义 (必须在 include 之前)
__sfr __at(0x94) P0M0;
__sfr __at(0x93) P0M1;
__sfr __at(0x92) P1M0;
__sfr __at(0x91) P1M1;
__sfr __at(0x96) P2M0;
__sfr __at(0x95) P2M1;
__sfr __at(0xB2) P3M0;
__sfr __at(0xB1) P3M1;
__sfr __at(0xB4) P4M0;
__sfr __at(0xB3) P4M1;

// ADC 相关
__sfr __at(0xBC) ADC_CONTR;
__sfr __at(0xBD) ADC_RES;
__sfr __at(0xBE) ADC_RESL;

// 中断相关
__sfr __at(0xA8) IE;
__sfr __at(0xAF) IE2;
__sfr __at(0xB8) IP;
__sfr __at(0xB7) IP2;

// 定时器相关
__sfr __at(0x88) TCON;
__sfr __at(0x89) TMOD;
__sfr __at(0x8A) TL0;
__sfr __at(0x8B) TL1;
__sfr __at(0x8C) TH0;
__sfr __at(0x8D) TH1;

// 串口相关
__sfr __at(0x98) SCON;
__sfr __at(0x99) SBUF;
__sfr __at(0x9A) S2CON;
__sfr __at(0x9B) S2BUF;

// 系统控制
__sfr __at(0x87) PCON;
__sfr __at(0x8E) AUXR;
__sfr __at(0x8F) AUXR2;
__sfr __at(0x97) CLK_DIV;

// 看门狗
__sfr __at(0xC1) WDT_CONTR;

#include <8051.h>

// GPIO 位定义
__sbit __at(0x80) P00;
__sbit __at(0x81) P01;
__sbit __at(0x82) P02;
__sbit __at(0x83) P03;
__sbit __at(0x84) P04;
__sbit __at(0x85) P05;
__sbit __at(0x86) P06;
__sbit __at(0x87) P07;

__sbit __at(0x90) P10;
__sbit __at(0x91) P11;
__sbit __at(0x92) P12;
__sbit __at(0x93) P13;
__sbit __at(0x94) P14;
__sbit __at(0x95) P15;
__sbit __at(0x96) P16;
__sbit __at(0x97) P17;

__sbit __at(0xA0) P20;
__sbit __at(0xA1) P21;
__sbit __at(0xA2) P22;
__sbit __at(0xA3) P23;
__sbit __at(0xA4) P24;
__sbit __at(0xA5) P25;
__sbit __at(0xA6) P26;
__sbit __at(0xA7) P27;

__sbit __at(0xB0) P30;
__sbit __at(0xB1) P31;
__sbit __at(0xB2) P32;
__sbit __at(0xB3) P33;
__sbit __at(0xB4) P34;
__sbit __at(0xB5) P35;
__sbit __at(0xB6) P36;
__sbit __at(0xB7) P37;

__sbit __at(0xC0) P40;
__sbit __at(0xC1) P41;
__sbit __at(0xC2) P42;
__sbit __at(0xC3) P43;
__sbit __at(0xC4) P44;
__sbit __at(0xC5) P45;
__sbit __at(0xC6) P46;
__sbit __at(0xC7) P47;

#endif // STC8G1K08_H
