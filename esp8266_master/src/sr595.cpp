/*
 * 74HC595 移位寄存器驱动实现
 */

#include "sr595.h"

// 全局变量
uint8_t g_relay_state = 0x00;  // 初始状态：所有继电器关闭

/**
 * 初始化 74HC595
 */
void sr595_init(void) {
    // 配置锁存引脚为输出
    pinMode(SR_LATCH_PIN, OUTPUT);
    pinMode(LORA_CS_PIN, OUTPUT);
    
    // 初始状态：锁存高电平 (保持输出稳定)
    digitalWrite(SR_LATCH_PIN, HIGH);
    digitalWrite(LORA_CS_PIN, HIGH);  // LoRa 禁用
    
    // 确保初始输出为全 0
    sr595_write(0x00);
    
    Serial.println("✅ 74HC595 初始化完成");
}

/**
 * 写入 8 位数据到 74HC595
 * 
 * 时序:
 * 1. 确保 LoRa 禁用 (CS HIGH)
 * 2. 拉低锁存引脚 (准备移位)
 * 3. 通过 shiftOut 发送 8 位数据
 * 4. 拉高锁存引脚 (锁存并输出)
 */
void sr595_write(uint8_t data) {
    // 1. 确保 LoRa 禁用 (避免 SPI 冲突)
    digitalWrite(LORA_CS_PIN, HIGH);
    
    // 2. 拉低锁存引脚，准备移位
    digitalWrite(SR_LATCH_PIN, LOW);
    
    // 3. 移位发送数据 (MSB 优先)
    // 使用 Arduino 内置 shiftOut 函数
    // 参数：数据引脚，时钟引脚，位序，数据
    shiftOut(D7, D5, MSBFIRST, data);  // MOSI, SCK
    
    // 4. 拉高锁存引脚，将移位寄存器数据锁存到输出寄存器
    digitalWrite(SR_LATCH_PIN, HIGH);
    
    // 保存状态
    g_relay_state = data;
    
    // 小延时确保稳定 (可选)
    delayMicroseconds(10);
}

/**
 * 开启指定继电器
 */
void sr595_relay_on(uint8_t relay_id) {
    if (relay_id > 7) {
        Serial.println("❌ 继电器 ID 超出范围 (0-7)");
        return;
    }
    
    // 设置对应位为 1
    g_relay_state |= (1 << relay_id);
    sr595_write(g_relay_state);
    
    Serial.print("✅ 继电器 ");
    Serial.print(relay_id);
    Serial.println(" 开启");
}

/**
 * 关闭指定继电器
 */
void sr595_relay_off(uint8_t relay_id) {
    if (relay_id > 7) {
        Serial.println("❌ 继电器 ID 超出范围 (0-7)");
        return;
    }
    
    // 清除对应位
    g_relay_state &= ~(1 << relay_id);
    sr595_write(g_relay_state);
    
    Serial.print("❌ 继电器 ");
    Serial.print(relay_id);
    Serial.println(" 关闭");
}

/**
 * 设置所有继电器状态
 * @param mask 8 位掩码 (bit0=继电器 0, bit7=继电器 7)
 */
void sr595_set_all(uint8_t mask) {
    g_relay_state = mask;
    sr595_write(mask);
    
    Serial.print("📊 继电器状态：0b");
    for (int i = 7; i >= 0; i--) {
        Serial.print((mask >> i) & 0x01);
    }
    Serial.println();
}

/**
 * 获取当前继电器状态
 */
uint8_t sr595_get_state(void) {
    return g_relay_state;
}

/**
 * 切换指定继电器状态
 */
void sr595_toggle(uint8_t relay_id) {
    if (relay_id > 7) return;
    
    // 异或切换对应位
    g_relay_state ^= (1 << relay_id);
    sr595_write(g_relay_state);
    
    Serial.print("🔄 继电器 ");
    Serial.print(relay_id);
    Serial.println(" 状态切换");
}

/**
 * 脉冲控制继电器 (用于测试)
 */
void sr595_pulse(uint8_t relay_id, uint16_t delay_ms) {
    if (relay_id > 7) return;
    
    // 开启
    sr595_relay_on(relay_id);
    delay(delay_ms);
    
    // 关闭
    sr595_relay_off(relay_id);
}

// ==================== 调试函数 ====================

/**
 * 测试所有继电器 (流水灯效果)
 */
void sr595_test_sequence(void) {
    Serial.println("🧪 开始继电器测试序列...");
    
    // 依次开启每个继电器
    for (int i = 0; i < 8; i++) {
        sr595_relay_on(i);
        delay(200);
    }
    
    // 依次关闭每个继电器
    for (int i = 0; i < 8; i++) {
        sr595_relay_off(i);
        delay(200);
    }
    
    // 全部开启
    sr595_set_all(0xFF);
    Serial.println("所有继电器开启");
    delay(500);
    
    // 全部关闭
    sr595_set_all(0x00);
    Serial.println("所有继电器关闭");
    delay(500);
    
    Serial.println("✅ 测试完成");
}

/**
 * 二进制计数测试
 */
void sr595_test_binary(void) {
    Serial.println("🧪 开始二进制计数测试...");
    
    for (uint8_t i = 0; i <= 255; i++) {
        sr595_set_all(i);
        delay(50);
    }
    
    sr595_set_all(0x00);
    Serial.println("✅ 测试完成");
}
