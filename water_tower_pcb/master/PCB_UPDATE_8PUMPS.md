# 主机 PCB 更新清单 - 支持 8 台水泵

## 当前状态
- **继电器数量**: 1 路 (设计支持 1-4 台水泵)
- **IO 口限制**: ESP8266 可用 IO 口不足
- **目标**: 支持 8 台水泵控制

## 推荐方案：PCF8574 I2C 扩展

### 新增元件

#### 1. PCF8574 I2C IO 扩展芯片
- **型号**: PCF8574APWR (TSSOP20) 或 PCF8574N (DIP16)
- **封装**: TSSOP20 或 DIP16
- **数量**: 1
- **单价**: ¥2.00
- **功能**: I2C 转 8 路 IO
- **连接**:
  - SDA → ESP8266 D1 (GPIO5)
  - SCL → ESP8266 D2 (GPIO4)
  - VCC → 3.3V
  - GND → GND
  - A0-A2 → GND (地址 0x20)
  - P0-P7 → 继电器驱动

#### 2. ULN2803 达林顿阵列 (可选)
- **型号**: ULN2803AN (DIP18) 或 ULN2803PWR (TSSOP18)
- **封装**: DIP18 或 TSSOP18
- **数量**: 1
- **单价**: ¥1.50
- **功能**: 8 路达林顿驱动，内置续流二极管
- **连接**:
  - IN1-8 → PCF8574 P0-P7
  - OUT1-8 → 继电器线圈
  - COM → 5V (续流二极管公共端)

#### 3. 继电器 (8 路)
- **型号**: HF115F-5-ZSL 或 JQC-3FF-5V
- **封装**: SMD 继电器 5V 单刀双掷
- **数量**: 8
- **单价**: ¥1.50
- **小计**: ¥12.00
- **额定**: 5V 线圈，10A 250VAC 触点

#### 4. 三极管 (如不使用 ULN2803)
- **型号**: S8050 NPN
- **封装**: SOT-23
- **数量**: 8
- **单价**: ¥0.04
- **小计**: ¥0.32

#### 5. 续流二极管 (如不使用 ULN2803)
- **型号**: 1N4148WS
- **封装**: SOD-323
- **数量**: 8
- **单价**: ¥0.03
- **小计**: ¥0.24

#### 6. 输出端子
- **型号**: TerminalBlock 16P 5mm
- **数量**: 2 (8 路 × 2 线)
- **单价**: ¥4.00
- **小计**: ¥8.00
- **功能**: 水泵 220V AC 输出

#### 7. 基极电阻 (如使用三极管)
- **型号**: 1kΩ 0805
- **数量**: 8
- **单价**: ¥0.02
- **小计**: ¥0.16

### 成本对比

| 项目 | 原设计 (1 路) | 新设计 (8 路) | 增量 |
|------|-------------|-------------|------|
| PCF8574 | ¥0 | ¥2.00 | +¥2.00 |
| ULN2803 | ¥0 | ¥1.50 | +¥1.50 |
| 继电器 | ¥1.50 | ¥12.00 | +¥10.50 |
| 三极管 | ¥0.04 | ¥0.32 | +¥0.28 |
| 二极管 | ¥0.03 | ¥0.24 | +¥0.21 |
| 端子 | ¥0.40 | ¥8.00 | +¥7.60 |
| 电阻 | ¥0.02 | ¥0.16 | +¥0.14 |
| **小计** | **¥1.99** | **¥24.22** | **+¥22.23** |
| **主机总价** | **¥145** | **¥167** | **+¥22** |

**结论**: 支持 8 台水泵，成本仅增加 **¥22**

---

## 原理图设计

### PCF8574 电路

```
         ┌─────────────────────────────────┐
         │         PCF8574APWR             │
         │                                 │
    SDA ─┤ 1  SDA                     VDD ├─ 3.3V
    SCL ─┤ 2  SCL                    GND ├─ GND
     A0 ─┤ 3  A0                      P0 ├─→ R1 ─→ Q1 ─→ 继电器 1
     A1 ─┤ 4  A1                      P1 ├─→ R2 ─→ Q2 ─→ 继电器 2
     A2 ─┤ 5  A2                      P2 ├─→ R3 ─→ Q3 ─→ 继电器 3
    INT ─┤ 6  INT                     P3 ├─→ R4 ─→ Q4 ─→ 继电器 4
         │ 7  P4                      P4 ├─→ R5 ─→ Q5 ─→ 继电器 5
         │ 8  P5                      P5 ├─→ R6 ─→ Q6 ─→ 继电器 6
    GND ─┤ 9  P6                      P6 ├─→ R7 ─→ Q7 ─→ 继电器 7
    GND ─┤ 10 P7                      P7 ├─→ R8 ─→ Q8 ─→ 继电器 8
         │ 11 GND                  NC ├─ 12
         │ 13 GND                   NC ├─ 14
         │ 15 GND                   NC ├─ 16
         │ 17 GND                   NC ├─ 18
         │ 19 GND                   NC ├─ 20
         └─────────────────────────────────┘

地址设置:
- A0, A1, A2 接地 → I2C 地址 0x20
- A0 接 VCC, A1-A2 接地 → I2C 地址 0x21
- ... (最多 8 个地址)
```

### 继电器驱动电路 (使用 ULN2803)

```
         ┌──────────────────────────────────┐
         │         ULN2803AN                │
         │                                  │
    P0 ──┤ 1  IN1                    OUT1 ├─→ 继电器 1 线圈 ─→ 5V
    P1 ──┤ 2  IN2                    OUT2 ├─→ 继电器 2 线圈 ─→ 5V
    P2 ──┤ 3  IN3                    OUT3 ├─→ 继电器 3 线圈 ─→ 5V
    P3 ──┤ 4  IN4                    OUT4 ├─→ 继电器 4 线圈 ─→ 5V
    P4 ──┤ 5  IN5                    OUT5 ├─→ 继电器 5 线圈 ─→ 5V
    P5 ──┤ 6  IN6                    OUT6 ├─→ 继电器 6 线圈 ─→ 5V
    P6 ──┤ 7  IN7                    OUT7 ├─→ 继电器 7 线圈 ─→ 5V
    P7 ──┤ 8  IN8                    OUT8 ├─→ 继电器 8 线圈 ─→ 5V
   COM ──┤ 9  COM (续流二极管公共端) ────────┘
   GND ──┤ 10 GND
         │ 11 NC
         │ ...
         │ 18 NC
         └──────────────────────────────────┘

注意:
- ULN2803 内置续流二极管，无需外接
- COM 引脚接 5V (继电器线圈电源)
- 每个 OUT 引脚可直接驱动 500mA 负载
```

### 继电器输出电路

```
继电器 1:
  线圈: 5V ──→ 继电器线圈 ──→ OUT1 (ULN2803)
  
  触点:
    COM1 ──→ 220V AC 火线输入 (L)
    NO1  ──→ 水泵 1 火线输出
    NC1  ──→ 空置
  
  零线:
    N ──→ 水泵 1 零线 (直连)

继电器 2-8: 同理

端子排:
  J8A: 水泵 1-4 输出 (8P: L1, N1, L2, N2, L3, N3, L4, N4)
  J8B: 水泵 5-8 输出 (8P: L5, N5, L6, N6, L7, N7, L8, N8)
```

---

## PCB 布局建议

### 元件布局

```
┌─────────────────────────────────────────────────────────┐
│                    主机 PCB 布局                         │
│                                                         │
│  ┌───────────┐                                          │
│  │ ESP8266   │  D1,D2 ──→ I2C 总线                     │
│  │ NodeMCU   │                                          │
│  └───────────┘                                          │
│       │                                                  │
│       │ SPI (D5-D8)                                     │
│       ↓                                                  │
│  ┌───────────┐                                          │
│  │ PAN3031   │  LoRa 433MHz                            │
│  │ LoRa      │                                          │
│  └───────────┘                                          │
│                                                         │
│  ┌───────────┐  ┌───────────┐                          │
│  │ PCF8574   │──│ ULN2803   │──→ 8 路继电器            │
│  │ I2C 扩展   │  │ 达林顿驱动 │     K1-K8               │
│  └───────────┘  └───────────┘                          │
│       ↑                                                  │
│       │ I2C (D1, D2)                                    │
│  ┌───────────┐                                          │
│  │ OLED      │  0.96" I2C                              │
│  │ 显示      │  (与 PCF8574 共用)                       │
│  └───────────┘                                          │
│                                                         │
│  ┌──────────────────────────────────────────────────┐  │
│  │  输出端子排 (16P)                                 │  │
│  │  L1 N1 L2 N2 L3 N3 L4 N4 L5 N5 L6 N6 L7 N7 L8 N8 │  │
│  └──────────────────────────────────────────────────┘  │
│                                                         │
│  ┌───────────┐  ┌───────────┐  ┌───────────────────┐  │
│  │ 5V 电源   │  │ 缺水传感器 │  │ 220V AC 输入       │  │
│  │ 输入      │  │ GPIO16    │  │ L N (市电)        │  │
│  └───────────┘  └───────────┘  └───────────────────┘  │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### 布线要点

1. **I2C 总线**:
   - SDA/SCL 走线尽量短
   - 添加 4.7kΩ上拉电阻到 3.3V
   - 远离高频信号 (LoRa, SPI)

2. **继电器部分**:
   - 继电器线圈走线加粗 (≥20mil)
   - 220V AC 部分与低压部分隔离 (间距≥3mm)
   - 添加光耦隔离 (可选，增强安全性)

3. **电源部分**:
   - 5V 电源线加粗 (≥30mil)
   - 每个 IC 添加 100nF 去耦电容
   - 继电器电源与逻辑电源分开走线

4. **热设计**:
   - 继电器下方挖空散热
   - 大电流走线开窗镀锡

---

## KiCad 修改步骤

### 步骤 1: 更新原理图

1. 打开 `water_tower_master.kicad_sch`
2. 添加 PCF8574 符号
3. 添加 ULN2803 符号
4. 添加 8 个继电器符号
5. 添加输出端子
6. 连接 I2C 总线 (SDA, SCL)
7. 连接继电器驱动电路
8. ERC 检查

### 步骤 2: 更新 PCB

1. 从原理图更新 PCB
2. 放置新元件
3. 布线 I2C 总线
4. 布线继电器驱动
5. 布线 220V AC 输出
6. 铺铜
7. DRC 检查

### 步骤 3: 生成生产文件

1. 生成 Gerber 文件
2. 生成钻孔文件
3. 生成 BOM
4. 生成坐标文件

---

## 固件更新

### 新增文件：`esp8266_master/src/pcf8574.h`

```cpp
#ifndef PCF8574_H
#define PCF8574_H

#include <Arduino.h>
#include <Wire.h>

#define PCF8574_ADDR 0x20  // I2C 地址 (A0-A2 接地)

class PCF8574 {
private:
    uint8_t i2c_addr;
    uint8_t current_state;

public:
    PCF8574(uint8_t addr = PCF8574_ADDR);
    bool begin();
    void write(uint8_t pin, bool value);
    void write8(uint8_t value);
    uint8_t read8();
    void pumpOn(uint8_t pump_id);
    void pumpOff(uint8_t pump_id);
    void allPumpsOff();
};

#endif
```

### 新增文件：`esp8266_master/src/pcf8574.cpp`

```cpp
#include "pcf8574.h"

PCF8574::PCF8574(uint8_t addr) : i2c_addr(addr), current_state(0) {}

bool PCF8574::begin() {
    Wire.begin(D1, D2);  // SDA, SCL
    Wire.beginTransmission(i2c_addr);
    return (Wire.endTransmission() == 0);
}

void PCF8574::write(uint8_t pin, bool value) {
    if (pin > 7) return;
    if (value) current_state |= (1 << pin);
    else current_state &= ~(1 << pin);
    write8(current_state);
}

void PCF8574::write8(uint8_t value) {
    current_state = value;
    Wire.beginTransmission(i2c_addr);
    Wire.write(current_state);
    Wire.endTransmission();
}

uint8_t PCF8574::read8() {
    Wire.requestFrom(i2c_addr, 1);
    return Wire.read();
}

void PCF8574::pumpOn(uint8_t pump_id) {
    write(pump_id, HIGH);
}

void PCF8574::pumpOff(uint8_t pump_id) {
    write(pump_id, LOW);
}

void PCF8574::allPumpsOff() {
    write8(0x00);
}
```

### 修改 `main.cpp`

```cpp
#include "pcf8574.h"

PCF8574 pcf8574;  // 全局对象

void setup() {
    Serial.begin(115200);
    
    // ... 现有初始化 ...
    
    // 初始化 PCF8574
    if (!pcf8574.begin()) {
        Serial.println("❌ PCF8574 初始化失败!");
        display.println("PCF8574 Error");
        display.display();
        while (1) delay(1000);
    }
    Serial.println("✅ PCF8574 初始化成功");
    
    // 关闭所有水泵
    pcf8574.allPumpsOff();
}

void control_pump(uint8_t tower_id, bool on) {
    if (tower_id >= tower_count || tower_id > 7) {
        Serial.println("❌ 无效的水塔 ID");
        return;
    }
    
    // 使用 PCF8574 控制继电器
    if (on) {
        pcf8574.pumpOn(tower_id);
        Serial.print("✅ 水塔 ");
        Serial.print(tower_id);
        Serial.println(" 开启水泵");
    } else {
        pcf8574.pumpOff(tower_id);
        Serial.print("❌ 水塔 ");
        Serial.print(tower_id);
        Serial.println(" 关闭水泵");
    }
    
    towers[tower_id].pump_on = on;
}

void process_auto_mode() {
    for (int i = 0; i < tower_count; i++) {
        // 水位低于 20% 开启水泵
        if (towers[i].water_level < 20 && !towers[i].pump_on) {
            control_pump(i, true);
        }
        // 水位高于 90% 关闭水泵
        else if (towers[i].water_level > 90 && towers[i].pump_on) {
            control_pump(i, false);
        }
    }
}

void emergency_stop() {
    Serial.println("🚨 紧急停止！关闭所有水泵");
    pcf8574.allPumpsOff();
    for (int i = 0; i < tower_count; i++) {
        towers[i].pump_on = false;
    }
}
```

---

## 测试计划

### 1. 单元测试

- [ ] PCF8574 I2C 通信测试
- [ ] 单个继电器控制测试
- [ ] 所有继电器同时控制测试

### 2. 功能测试

- [ ] 自动水位控制 (8 台水泵)
- [ ] 手动控制测试
- [ ] 缺水保护测试
- [ ] 紧急停止测试

### 3. 压力测试

- [ ] 8 台水泵同时启停
- [ ] 连续运行 72 小时
- [ ] 频繁切换测试

---

## 总结

### 方案优势
- ✅ **IO 口**: 不占用额外 IO (与 OLED 共用 I2C)
- ✅ **成本**: 仅增加 ¥22
- ✅ **扩展性**: 支持最多 64 台水泵 (8 片 PCF8574)
- ✅ **可靠性**: I2C 通信稳定
- ✅ **代码简单**: Wire 库易于使用

### 下一步
1. ✅ 确认方案 (PCF8574 + ULN2803 + 8 路继电器)
2. ⏳ 更新 KiCad 原理图和 PCB
3. ⏳ 生成 Gerber 文件
4. ⏳ 下单生产 PCB
5. ⏳ 焊接测试
6. ⏳ 烧录新固件验证

---

**版本**: v2.1
**日期**: 2025-12-21
**状态**: 设计完成，待 PCB 更新
