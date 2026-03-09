# ESP8266 IO 口使用分析与扩展方案

## 当前 IO 口使用情况

### ESP8266 NodeMCU 引脚资源

| GPIO | NodeMCU | 功能 | 当前用途 | 状态 |
|------|---------|------|----------|------|
| GPIO0 | D3 | 通用 | OLED RST | 🔴 已用 |
| GPIO1 | TX0 | 串口 TX | 串口调试 | 🟡 部分占用 |
| GPIO2 | D4 | 通用 | PAN3031 IRQ | 🔴 已用 |
| GPIO3 | RX0 | 串口 RX | 串口调试 | 🟡 部分占用 |
| GPIO4 | D2 | I2C SDA | OLED SCL | 🔴 已用 |
| GPIO5 | D1 | I2C SCL | OLED SDA | 🔴 已用 |
| GPIO6-8 | - | Flash | **不可用** | ❌ 禁止使用 |
| GPIO9-10 | - | Flash | **不可用** | ❌ 禁止使用 |
| GPIO11 | - | Flash | **不可用** | ❌ 禁止使用 |
| GPIO12 | D6 | SPI MISO | PAN3031 MISO | 🔴 已用 |
| GPIO13 | D7 | SPI MOSI | PAN3031 MOSI | 🔴 已用 |
| GPIO14 | D5 | SPI SCK | PAN3031 SCK | 🔴 已用 |
| GPIO15 | D8 | SPI CS | PAN3031 CS | 🔴 已用 |
| GPIO16 | D0 | 通用 | 缺水传感器 + 继电器 | 🔴 已用 |
| ADC | A0 | 模拟输入 | 未使用 | 🟢 空闲 |

### 已用 IO 口统计

**固定占用**:
- OLED 显示：3 个 (D1, D2, D3)
- PAN3031 LoRa: 5 个 (D4, D5, D6, D7, D8)
- 缺水传感器：1 个 (D0)
- **小计**: 9 个

**可用 IO 口**:
- **GPIO1 (TX0)**: 可用于输出（但会影响串口调试）
- **GPIO3 (RX0)**: 可用于输入（但会影响串口调试）
- **ADC (A0)**: 模拟输入，不能用于数字输出
- **实际可用数字输出**: **0-1 个**

### 结论

❌ **当前设计无法直接控制 8 台水泵**

剩余可用 IO 口：**0-1 个**（需要控制 8 台水泵）

---

## 扩展方案

### 方案 1: 3-8 译码器 (74HC138) ⭐ 推荐

**原理**:
```
ESP8266 3 个 IO 口 → 74HC138 → 8 路输出 → 8 个继电器
```

**硬件连接**:
```
ESP8266          74HC138         继电器
───────          ───────         ────────
GPIO? (D?) ────> A (Pin 1)
GPIO? (D?) ────> B (Pin 2)
GPIO? (D?) ────> C (Pin 3)
                 
                 Y0 (Pin 15) ──> 继电器 1
                 Y1 (Pin 14) ──> 继电器 2
                 Y2 (Pin 13) ──> 继电器 3
                 Y3 (Pin 12) ──> 继电器 4
                 Y4 (Pin 11) ──> 继电器 5
                 Y5 (Pin 10) ──> 继电器 6
                 Y6 (Pin 9)  ──> 继电器 7
                 Y7 (Pin 7)  ──> 继电器 8
```

**IO 口需求**:
- 数据线：3 个 (A, B, C)
- 使能端：3 个 (G1, G2A, G2B) - 可固定接高/低电平
- **总计**: **3 个 IO 口**

**优点**:
- ✅ 仅需 3 个 IO 口控制 8 路
- ✅ 成本低 (74HC138: ¥0.5)
- ✅ 电路简单
- ✅ 响应速度快

**缺点**:
- ⚠️ 同一时间只能控制 1 个继电器
- ⚠️ 需要软件扫描控制

**代码示例**:
```cpp
#define DEC_A D9   // GPIO11 (需要复用)
#define DEC_B D10  // GPIO12 (需要复用)
#define DEC_C D11  // GPIO13 (需要复用)

void setup_decoder() {
    pinMode(DEC_A, OUTPUT);
    pinMode(DEC_B, OUTPUT);
    pinMode(DEC_C, OUTPUT);
}

void select_relay(uint8_t relay_id) {
    // relay_id: 0-7
    digitalWrite(DEC_A, relay_id & 0x01);
    digitalWrite(DEC_B, (relay_id >> 1) & 0x01);
    digitalWrite(DEC_C, (relay_id >> 2) & 0x01);
}

void pump_control(uint8_t tower_id, bool on) {
    select_relay(tower_id);
    delay(10);  // 等待译码器稳定
    // 控制公共继电器引脚
    digitalWrite(PUMP_COMMON, on ? HIGH : LOW);
}
```

**问题**: 需要复用 SPI 引脚 (D5-D8)，会影响 LoRa 通信

---

### 方案 2: I2C IO 扩展芯片 (PCF8574) ⭐⭐ 强烈推荐

**原理**:
```
ESP8266 I2C → PCF8574 → 8 路 IO → 8 个继电器
```

**硬件连接**:
```
ESP8266          PCF8574         继电器
───────          ───────         ────────
D1 (GPIO5) ────> SDA
D2 (GPIO4) ────> SCL
3.3V ──────────> VCC
GND ───────────> GND

                 P0 ──> 继电器 1
                 P1 ──> 继电器 2
                 P2 ──> 继电器 3
                 P3 ──> 继电器 4
                 P4 ──> 继电器 5
                 P5 ──> 继电器 6
                 P6 ──> 继电器 7
                 P7 ──> 继电器 8
```

**IO 口需求**:
- I2C 总线：2 个 (SDA, SCL) - **与 OLED 共用**
- **总计**: **0 个额外 IO 口**

**优点**:
- ✅ 不占用额外 IO 口 (与 OLED 共用 I2C)
- ✅ 可同时控制多个继电器
- ✅ 成本低 (PCF8574: ¥2)
- ✅ 支持级联 (最多 8 片 = 64 路)
- ✅ 代码简单 (Wire 库)

**缺点**:
- ⚠️ I2C 速度较慢 (但对继电器够用)

**代码示例**:
```cpp
#include <Wire.h>
#include <PCF8574.h>

PCF8574 pcf8574(0x20);  // I2C 地址 0x20

void setup() {
    Wire.begin(D1, D2);  // SDA, SCL
    pcf8574.begin();
}

void pump_control(uint8_t tower_id, bool on) {
    if (tower_id > 7) return;
    pcf8574.write(tower_id, on ? HIGH : LOW);
}

// 批量控制
void control_multiple(uint8_t mask) {
    // mask: 8 位二进制，每位控制一个继电器
    pcf8574.write8(mask);
}
```

**接线调整**:
- OLED 和 PCF8574 共用 I2C 总线 (D1, D2)
- 上拉电阻：SDA/SCL 各接 4.7kΩ到 3.3V

---

### 方案 3: SPI 移位寄存器 (74HC595)

**原理**:
```
ESP8266 SPI → 74HC595 → 8 路输出 → 8 个继电器
```

**硬件连接**:
```
ESP8266          74HC595         继电器
───────          ───────         ────────
D5 (SCK)  ────> SH_CP (Pin 11)
D7 (MOSI) ────> DS (Pin 14)
D8 (CS)   ────> ST_CP (Pin 12)

                 Q0 ──> 继电器 1
                 Q1 ──> 继电器 2
                 ...
                 Q7 ──> 继电器 8
```

**IO 口需求**:
- SPI 总线：3 个 (SCK, MOSI, CS) - **与 LoRa 共用**
- **总计**: **0 个额外 IO 口**

**优点**:
- ✅ 不占用额外 IO 口 (与 LoRa 共用 SPI)
- ✅ 可同时控制多个继电器
- ✅ 成本极低 (74HC595: ¥0.3)
- ✅ 支持级联

**缺点**:
- ⚠️ 需要与 LoRa 分时复用 SPI 总线
- ⚠️ 代码稍复杂

**代码示例**:
```cpp
#define SR_DATA D7   // MOSI
#define SR_CLK  D5   // SCK
#define SRLatch D8   // CS

void setup_sr() {
    pinMode(SR_DATA, OUTPUT);
    pinMode(SR_CLK, OUTPUT);
    pinMode(SRLatch, OUTPUT);
}

void write_sr(uint8_t data) {
    digitalWrite(SRLatch, LOW);
    shiftOut(SR_DATA, SR_CLK, MSBFIRST, data);
    digitalWrite(SRLatch, HIGH);
}

void pump_control(uint8_t tower_id, bool on) {
    uint8_t state = read_current_state();
    if (on) state |= (1 << tower_id);
    else state &= ~(1 << tower_id);
    write_sr(state);
}
```

---

### 方案 4: 专用继电器扩展板

**推荐模块**:
1. **I2C 继电器板** (PCF8574 + 继电器)
   - 8 路继电器
   - I2C 接口
   - 价格：¥15-20

2. **SPI 继电器板** (74HC595 + 继电器)
   - 8 路继电器
   - SPI 接口
   - 价格：¥12-18

**优点**:
- ✅ 即插即用
- ✅ 包含驱动电路
- ✅ 隔离保护好

**缺点**:
- ⚠️ 成本较高

---

## 推荐方案对比

| 方案 | IO 占用 | 成本 | 复杂度 | 推荐度 |
|------|---------|------|--------|--------|
| 3-8 译码器 (74HC138) | 3 个 | ¥0.5 | 低 | ⭐⭐⭐ |
| I2C 扩展 (PCF8574) | 0 个 (共用) | ¥2 | 低 | ⭐⭐⭐⭐⭐ |
| SPI 移位寄存器 (74HC595) | 0 个 (共用) | ¥0.3 | 中 | ⭐⭐⭐⭐ |
| 成品继电器板 | 0 个 (共用) | ¥15-20 | 极低 | ⭐⭐⭐⭐ |

---

## 最佳方案：PCF8574 I2C 扩展

### 推荐理由
1. **不占用额外 IO 口** - 与 OLED 共用 I2C
2. **可同时控制多个继电器** - 无需扫描
3. **成本低** - PCF8574 仅需 ¥2
4. **电路简单** - 只需 2 根线
5. **可扩展** - 最多支持 64 路

### 硬件设计变更

#### 主机 PCB 新增元件
```
U5: PCF8574APWR (TSSOP20) 或 PCF8574N (DIP16)
J8: 8 路继电器输出端子
Q1-Q8: 8 个三极管 (S8050) 或 ULN2803 达林顿阵列
D1-D8: 8 个续流二极管 (1N4148)
K1-K8: 8 个继电器 (5V 10A)
```

#### 电路连接
```
ESP8266        PCF8574        ULN2803       继电器
───────        ───────        ───────       ────────
D1 (SDA) ────> SDA ─────────> IN1 ──────> 继电器 1
D2 (SCL) ────> SCL            IN2 ──────> 继电器 2
               P0             IN3 ──────> 继电器 3
               P1             IN4 ──────> 继电器 4
               P2             IN5 ──────> 继电器 5
               P3             IN6 ──────> 继电器 6
               P4             IN7 ──────> 继电器 7
               P5             COM ─────> 继电器公共端
               P6
               P7
```

### 软件更新

#### 新增文件：`esp8266_master/src/pcf8574.cpp`
```cpp
#include <Wire.h>
#include "pcf8574.h"

PCF8574 pcf8574(0x20);  // I2C 地址：0x20 (A0-A2 接地)

bool pcf8574_init() {
    Wire.begin(D1, D2);  // SDA, SCL
    return pcf8574.begin();
}

void pcf8574_pump_control(uint8_t tower_id, bool on) {
    if (tower_id > 7) return;
    pcf8574.write(tower_id, on ? HIGH : LOW);
}

void pcf8574_set_all(uint8_t mask) {
    pcf8574.write8(mask);
}

uint8_t pcf8574_get_state() {
    return pcf8574.read8();
}
```

#### 修改 `main.cpp`
```cpp
#include "pcf8574.h"

void setup() {
    // ... 现有初始化 ...
    
    // 初始化 PCF8574
    if (!pcf8574_init()) {
        Serial.println("PCF8574 初始化失败!");
    }
}

void control_pump(uint8_t tower_id, bool on) {
    if (tower_id >= tower_count) return;
    
    // 使用 PCF8574 控制继电器
    pcf8574_pump_control(tower_id, on);
    towers[tower_id].pump_on = on;
    
    Serial.print("水塔 ");
    Serial.print(tower_id);
    Serial.println(on ? " 开启水泵" : " 关闭水泵");
}
```

### BOM 更新

#### 新增元件
| 位号 | 元件 | 型号 | 数量 | 单价 | 小计 |
|------|------|------|------|------|------|
| U5 | IO 扩展 | PCF8574APWR | 1 | ¥2.00 | ¥2.00 |
| U6 | 驱动 | ULN2803 | 1 | ¥1.50 | ¥1.50 |
| K1-K8 | 继电器 | 5V 10A | 8 | ¥1.50 | ¥12.00 |
| Q1-Q8 | 三极管 | S8050 | 8 | ¥0.04 | ¥0.32 |
| D1-D8 | 二极管 | 1N4148 | 8 | ¥0.03 | ¥0.24 |
| J8 | 端子 | 16P 5mm | 1 | ¥2.00 | ¥2.00 |
| R1-R8 | 电阻 | 1kΩ | 8 | ¥0.02 | ¥0.16 |
| **新增合计** | | | | | **¥18.22** |

**总成本**: ¥145 (原主机) + ¥18.22 = **¥163.22**

---

## 实施步骤

### 步骤 1: 软件验证 (立即)
1. 购买 PCF8574 模块 (¥5 包邮)
2. 面包板接线测试
3. 烧录新固件验证

### 步骤 2: PCB 设计 (1 周)
1. 更新主机 PCB 原理图
2. 添加 PCF8574 和 8 路继电器
3. 重新布局布线
4. 生成 Gerber 文件

### 步骤 3: 生产测试 (2 周)
1. SMT 贴片 PCB
2. 焊接测试
3. 功能验证
4. 现场部署

---

## 临时方案

在 PCB 更新前，可以使用:
1. **PCF8574 模块** + **8 路继电器模块**
2. 飞线连接到 ESP8266 (D1, D2)
3. 软件已支持，可立即测试

**成本**: PCF8574 模块 (¥5) + 8 路继电器模块 (¥15) = **¥20**

---

## 总结

### 当前状态
- ❌ 可用 IO 口：0-1 个
- ❌ 无法直接控制 8 台水泵

### 推荐方案
- ✅ **PCF8574 I2C 扩展** (0 额外 IO 口，成本¥18)
- ✅ 支持 8 台水泵同时控制
- ✅ 电路简单，代码易实现

### 备选方案
- 74HC595 SPI 移位寄存器 (成本¥0.3，需复用 SPI)
- 成品 8 路继电器板 (成本¥15-20，即插即用)

### 下一步
1. 确认方案 (推荐 PCF8574)
2. 购买模块测试
3. 更新 PCB 设计
4. 批量生产

---

**版本**: v2.1
**日期**: 2025-12-21
**作者**: AI Assistant
