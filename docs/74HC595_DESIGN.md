# 74HC595 SPI 复用方案 - 8 路继电器控制

## 方案概述

使用 **74HC595 移位寄存器** 复用 ESP8266 的 SPI 总线，控制 8 路继电器。

### 核心优势

| 优势 | 说明 |
|------|------|
| ✅ **SPI 复用** | 与 LoRa 共用 SCK/MOSI，仅需 1 个额外 IO (锁存) |
| ✅ **同时控制** | 8 路继电器可独立同时控制 |
| ✅ **成本低** | 74HC595 ¥0.3 + 继电器模块 ¥15 = ¥15.3 |
| ✅ **电路简单** | 只需 3 根信号线 |
| ✅ **易扩展** | 支持级联，最多 64 路 (8 片级联) |

---

## 硬件设计

### ESP8266 引脚分配

| ESP8266 | 功能 | 用途 | 说明 |
|---------|------|------|------|
| D5 (GPIO14) | SPI SCK | LoRa + 74HC595 | 共用时钟 |
| D7 (GPIO13) | SPI MOSI | LoRa + 74HC595 | 共用数据 |
| D8 (GPIO15) | SPI CS | LoRa 片选 | 独立 |
| D4 (GPIO2) | GPIO | 74HC595 锁存 | 独立 |
| D1/D2 | I2C | OLED | 独立 |
| D0 | GPIO | 缺水传感器 | 独立 |

### 74HC595 引脚连接

```
ESP8266          74HC595         8 路继电器模块
───────          ───────         ────────────────
D5 (SCK)  ─────> 11  SH_CP
D7 (MOSI) ─────> 14  DS
D4 (GPIO2) ────> 12  ST_CP (锁存)
                 
5V ─────────────> 16  VCC
GND ────────────> 8   GND

74HC595 输出:
  15  Q0 ──────> 继电器 IN1
  1   Q1 ──────> 继电器 IN2
  2   Q2 ──────> 继电器 IN3
  3   Q3 ──────> 继电器 IN4
  4   Q4 ──────> 继电器 IN5
  5   Q5 ──────> 继电器 IN6
  6   Q6 ──────> 继电器 IN7
  7   Q7 ──────> 继电器 IN8

其他引脚:
  13  OE ──────> GND (输出使能)
  10  MR ──────> VCC (复位禁用)
  9   Q7' ─────> NC (级联用，本次不用)
```

### 去耦电容

```
74HC595 VCC (Pin 16) ──[100nF]── GND (Pin 8)
```

### 电路图

```
                    ┌─────────────────────────────────┐
                    │         74HC595N                │
                    │                                 │
    ESP D5 (SCK) ───┤ 11  SH_CP              VCC ├─ 16 ──> 5V
    ESP D7 (MOSI) ──┤ 14  DS                Q0 ├─ 15 ──> IN1
    ESP D4 (GPIO2) ─┤ 12  ST_CP             Q1 ├─ 1  ──> IN2
                    │ 13  OE ───┬── GND     Q2 ├─ 2  ──> IN3
                    │ 10  MR ───┼── VCC     Q3 ├─ 3  ──> IN4
                    │ 8   GND ──┴── GND     Q4 ├─ 4  ──> IN5
                    │ 9   Q7' (NC)          Q5 ├─ 5  ──> IN6
                    │ 7   Q6                  Q6 ├─ 6  ──> IN7
                    │ 6   Q5                  Q7 ├─ 7  ──> IN8
                    └─────────────────────────────────┘
                    
    去耦：16 脚 (VCC) ──[100nF]── 8 脚 (GND)
```

---

## SPI 复用原理

### 总线共享

```
ESP8266 (SPI Master)
       │
       ├── SCK ──┬──> LoRa SCK
       │         └──> 74HC595 SH_CP
       │
       ├── MOSI ─┬──> LoRa MOSI
       │         └──> 74HC595 DS
       │
       ├── CS1 (D8) ──> LoRa CS (片选)
       │
       └── CS2 (D4) ──> 74HC595 ST_CP (锁存)
```

### 工作时序

#### 1. 控制 LoRa (不影响继电器)

```cpp
digitalWrite(LORA_CS, LOW);    // 选中 LoRa
// SPI 通信...
digitalWrite(LORA_CS, HIGH);   // 禁用 LoRa
// 74HC595 锁存保持 HIGH，继电器状态不变
```

#### 2. 控制 74HC595 (不影响 LoRa)

```cpp
digitalWrite(LORA_CS, HIGH);   // 确保 LoRa 禁用
digitalWrite(SR_LATCH, LOW);   // 74HC595 准备移位
shiftOut(D7, D5, MSBFIRST, data);  // 发送数据
digitalWrite(SR_LATCH, HIGH);  // 74HC595 锁存输出
// LoRa CS 保持 HIGH，LoRa 不响应
```

### 关键：互不干扰

- **LoRa 通信时**: 74HC595 锁存引脚保持 HIGH，移位寄存器变化不影响输出
- **控制继电器时**: LoRa CS 保持 HIGH，LoRa 不响应 SPI 总线

---

## 软件设计

### 核心函数

```cpp
// 初始化
void sr595_init() {
    pinMode(SR_LATCH, OUTPUT);
    pinMode(LORA_CS, OUTPUT);
    digitalWrite(SR_LATCH, HIGH);
    digitalWrite(LORA_CS, HIGH);
    sr595_write(0x00);  // 初始关闭所有继电器
}

// 写入数据
void sr595_write(uint8_t data) {
    digitalWrite(LORA_CS, HIGH);   // 禁用 LoRa
    digitalWrite(SR_LATCH, LOW);   // 准备移位
    shiftOut(D7, D5, MSBFIRST, data);  // 发送
    digitalWrite(SR_LATCH, HIGH);  // 锁存输出
}

// 控制单个继电器
void sr595_relay_on(uint8_t id) {
    g_relay_state |= (1 << id);
    sr595_write(g_relay_state);
}

void sr595_relay_off(uint8_t id) {
    g_relay_state &= ~(1 << id);
    sr595_write(g_relay_state);
}

// 批量控制
void sr595_set_all(uint8_t mask) {
    sr595_write(mask);
}
```

### 集成到水泵控制

```cpp
void control_pump(uint8_t tower_id, bool on) {
    if (on) {
        sr595_relay_on(tower_id);
        towers[tower_id].pump_on = true;
    } else {
        sr595_relay_off(tower_id);
        towers[tower_id].pump_on = false;
    }
}

// 紧急停止
void emergency_stop() {
    sr595_set_all(0x00);  // 关闭所有继电器
}
```

---

## 成本分析

| 元件 | 数量 | 单价 | 小计 |
|------|------|------|------|
| 74HC595N | 1 | ¥0.30 | ¥0.30 |
| 8 路继电器模块 | 1 | ¥15.00 | ¥15.00 |
| 排针/端子 | 1 | ¥2.00 | ¥2.00 |
| 100nF 电容 | 1 | ¥0.03 | ¥0.03 |
| **总计** | | | **¥17.33** |

**对比**:
- PCF8574 方案：¥22
- **74HC595 方案：¥17.33 (节省 ¥4.67)**

---

## PCB 设计

### 元件布局

```
┌─────────────────────────────────────────────────────┐
│              主机 PCB 布局                           │
│                                                     │
│  ┌───────────┐                                      │
│  │ ESP8266   │  D4,D5,D7,D8 ──┐                    │
│  └───────────┘                │                    │
│                               ↓                    │
│  ┌───────────┐         ┌─────────────┐            │
│  │ PAN3031   │<───────>│  74HC595    │            │
│  │ LoRa      │  SPI    │  移位寄存器 │            │
│  └───────────┘         └──────┬──────┘            │
│                               │                    │
│  ┌───────────┐                ↓                    │
│  │ OLED      │         ┌─────────────┐            │
│  │ I2C 显示   │         │ 8 路继电器   │            │
│  └───────────┘         │ 模块        │            │
│                        └──────┬──────┘            │
│                               │                    │
│                        ┌──────┴──────┐            │
│                        │ 输出端子排   │            │
│                        │ 8x2 Pin     │            │
│                        └─────────────┘            │
│                                                     │
└─────────────────────────────────────────────────────┘
```

### 布线要点

1. **SPI 总线**:
   - SCK/MOSI 走线等长
   - 靠近 74HC595 放置
   - 远离高频信号

2. **继电器输出**:
   - 输出走线加粗 (≥20mil)
   - 220V AC 部分隔离 (间距≥3mm)
   - 添加光耦隔离 (可选)

3. **电源**:
   - 5V 电源线加粗 (≥30mil)
   - 74HC595 附近加 100nF 去耦电容

---

## 测试计划

### 1. 单元测试

```cpp
void test_sr595() {
    // 流水灯测试
    for (int i = 0; i < 8; i++) {
        sr595_relay_on(i);
        delay(200);
    }
    
    // 全部关闭
    sr595_set_all(0x00);
    delay(500);
    
    // 全部开启
    sr595_set_all(0xFF);
    delay(500);
    
    // 全部关闭
    sr595_set_all(0x00);
}
```

### 2. SPI 复用测试

```cpp
void test_spi_multiplex() {
    // 1. LoRa 通信
    pan3031_send(tx_data, 5);
    
    // 2. 控制继电器 (不影响 LoRa)
    sr595_relay_on(0);
    
    // 3. 再次 LoRa 通信
    pan3031_receive(rx_data, 8);
    
    // 验证继电器状态保持不变
    assert(sr595_get_state() == 0x01);
}
```

### 3. 集成测试

- [ ] 8 路继电器独立控制
- [ ] 同时控制多个继电器
- [ ] LoRa 通信不受影响
- [ ] 自动控制逻辑正常
- [ ] 紧急停止功能正常

---

## 文件清单

### 新增文件

1. **`esp8266_master/src/sr595.h`** - 头文件
2. **`esp8266_master/src/sr595.cpp`** - 驱动实现
3. **`esp8266_master/src/main_sr595.cpp`** - 主程序 (74HC595 版)
4. **`docs/74HC595_DESIGN.md`** - 本文档

### 修改文件

1. **`esp8266_master/src/main.cpp`** - 引脚定义更新
   - PAN3031_CS: D5 → D8
   - PAN3031_IRQ: 禁用 (D4 用于锁存)

---

## 使用示例

### 基本控制

```cpp
#include "sr595.h"

void setup() {
    sr595_init();
}

void loop() {
    // 开启 1 号水泵
    sr595_relay_on(0);
    delay(1000);
    
    // 关闭 1 号水泵
    sr595_relay_off(0);
    delay(1000);
    
    // 同时开启 1,3,5,7 号水泵
    sr595_set_all(0b01010101);
    delay(2000);
    
    // 全部关闭
    sr595_set_all(0x00);
    delay(2000);
}
```

### 自动控制

```cpp
void auto_control() {
    // 水位低时开启水泵
    if (water_level < 20) {
        sr595_relay_on(pump_id);
    }
    // 水位高时关闭水泵
    else if (water_level > 90) {
        sr595_relay_off(pump_id);
    }
}

// 缺水保护
void well_water_protection() {
    if (!well_water_ok) {
        sr595_set_all(0x00);  // 关闭所有水泵
    }
}
```

---

## 常见问题

### Q1: 为什么不用 PCF8574?

**A**: 74HC595 更便宜 (¥0.3 vs ¥2)，且 SPI 复用更简单。

### Q2: SPI 复用会冲突吗？

**A**: 不会。通过独立的 CS/锁存引脚控制，确保同一时间只有一个设备响应。

### Q3: 可以级联更多路吗？

**A**: 可以。Q7' 引脚连接下一片 DS，最多支持 8 片级联 (64 路)。

### Q4: 继电器模块需要额外驱动吗？

**A**: 不需要。市面 8 路继电器模块自带三极管驱动，直接接 74HC595 输出即可。

---

## 总结

### 方案优势
- ✅ **成本最低**: ¥17.33 (比 PCF8574 省¥4.67)
- ✅ **IO 占用少**: 仅 1 个额外 IO (锁存)
- ✅ **同时控制**: 8 路独立控制
- ✅ **SPI 复用**: 与 LoRa 完美共存
- ✅ **易于扩展**: 支持级联至 64 路

### 适用场景
- 需要控制 8 台以内水泵
- ESP8266 IO 口紧张
- 成本敏感项目
- 需要同时控制多个继电器

### 下一步
1. ✅ 软件设计完成
2. ✅ 文档编写完成
3. ⏳ 购买 74HC595 + 继电器模块测试
4. ⏳ 更新 PCB 设计
5. ⏳ 批量生产

---

**版本**: v2.1
**日期**: 2025-12-21
**状态**: 设计完成，待测试
