/*
 * 错误码系统实现
 */

#include "error_codes.h"
#include <Wire.h>
#include <Adafruit_SSD1306.h>

// 外部 OLED 对象 (在 main.cpp 中定义)
extern Adafruit_SSD1306 display;

// 全局错误日志
Error_t g_error_log[MAX_ERROR_LOG];
uint8_t g_error_count = 0;
uint16_t g_current_error = ERR_SYS_OK;

// 错误描述表
typedef struct {
    uint16_t code;
    const char* message;
    ErrorLevel_t level;
} ErrorDesc_t;

static const ErrorDesc_t error_table[] = {
    // 系统错误
    {ERR_SYS_OK, "System OK", ERR_LEVEL_NONE},
    {ERR_SYS_INIT_FAIL, "SYS Init Fail", ERR_LEVEL_CRITICAL},
    {ERR_SYS_MEMORY_LOW, "Mem Low", ERR_LEVEL_WARNING},
    {ERR_SYS_WDT_RESET, "WDT Reset", ERR_LEVEL_ERROR},
    {ERR_SYS_POWER_LOW, "Power Low", ERR_LEVEL_CRITICAL},
    {ERR_SYS_OVERHEAT, "Overheat", ERR_LEVEL_ERROR},
    
    // 传感器错误
    {ERR_SENSOR_OK, "Sensor OK", ERR_LEVEL_NONE},
    {ERR_SENSOR_WATER_LOW, "Well Water Low", ERR_LEVEL_CRITICAL},
    {ERR_SENSOR_ADC_FAIL, "ADC Fail", ERR_LEVEL_ERROR},
    {ERR_SENSOR_OUT_OF_RANGE, "Sensor Range", ERR_LEVEL_WARNING},
    {ERR_SENSOR_DISCONNECT, "Sensor Disc", ERR_LEVEL_ERROR},
    
    // 通信错误
    {ERR_COM_OK, "Comm OK", ERR_LEVEL_NONE},
    {ERR_COM_LORA_TIMEOUT, "LoRa Timeout", ERR_LEVEL_ERROR},
    {ERR_COM_LORA_CRC, "LoRa CRC Err", ERR_LEVEL_ERROR},
    {ERR_COM_I2C_FAIL, "I2C Fail", ERR_LEVEL_ERROR},
    {ERR_COM_SPI_FAIL, "SPI Fail", ERR_LEVEL_ERROR},
    {ERR_COM_WIFI_FAIL, "WiFi Fail", ERR_LEVEL_WARNING},
    {ERR_COM_WEB_FAIL, "Web Server Err", ERR_LEVEL_WARNING},
    
    // 继电器错误
    {ERR_REL_OK, "Relay OK", ERR_LEVEL_NONE},
    {ERR_REL_STUCK_ON, "Relay Stuck On", ERR_LEVEL_CRITICAL},
    {ERR_REL_STUCK_OFF, "Relay Stuck Off", ERR_LEVEL_ERROR},
    {ERR_REL_OVERLOAD, "Relay Overload", ERR_LEVEL_CRITICAL},
    {ERR_REL_DRIVER_FAIL, "74HC595 Fail", ERR_LEVEL_ERROR},
    
    // 网络错误
    {ERR_NET_OK, "Net OK", ERR_LEVEL_NONE},
    {ERR_NET_NO_FROM, "No Slave Data", ERR_LEVEL_WARNING},
    {ERR_NET_TIMEOUT, "Net Timeout", ERR_LEVEL_ERROR},
    {ERR_NET_INVALID_DATA, "Invalid Data", ERR_LEVEL_WARNING},
    
    // 水塔错误
    {ERR_TOWER_OK, "Tower OK", ERR_LEVEL_NONE},
    {ERR_TOWER_OVERFLOW, "Tower Overflow", ERR_LEVEL_CRITICAL},
    {ERR_TOWER_DRY, "Tower Dry", ERR_LEVEL_WARNING},
    {ERR_TOWER_PUMP_FAIL, "Pump Fail", ERR_LEVEL_CRITICAL},
    {ERR_TOWER_SENSOR_FAIL, "Tower Sen Fail", ERR_LEVEL_ERROR},
};

#define ERROR_TABLE_SIZE (sizeof(error_table) / sizeof(ErrorDesc_t))

// ==================== 函数实现 ====================

/**
 * 初始化错误系统
 */
void error_init(void) {
    g_error_count = 0;
    g_current_error = ERR_SYS_OK;
    
    // 清空错误日志
    for (int i = 0; i < MAX_ERROR_LOG; i++) {
        g_error_log[i].code = ERR_SYS_OK;
        g_error_log[i].level = ERR_LEVEL_NONE;
        g_error_log[i].message = "None";
        g_error_log[i].timestamp = 0;
        g_error_log[i].tower_id = 0xFF;
    }
    
    Serial.println("✅ 错误系统初始化完成");
}

/**
 * 记录错误
 */
void error_log(uint16_t code, ErrorLevel_t level, uint8_t tower_id) {
    if (code == ERR_SYS_OK) return;  // 不记录正常状态
    
    // 更新当前错误
    if (level >= ERR_LEVEL_ERROR) {
        g_current_error = code;
    }
    
    // 添加到日志 (循环缓冲)
    uint8_t index = g_error_count % MAX_ERROR_LOG;
    
    g_error_log[index].code = code;
    g_error_log[index].level = level;
    g_error_log[index].timestamp = millis();
    g_error_log[index].tower_id = tower_id;
    g_error_log[index].message = error_get_message(code);
    
    g_error_count++;
    
    // 打印到串口
    error_print_serial(code);
    
    // 显示到 OLED
    if (level >= ERR_LEVEL_WARNING) {
        error_display_oled(code);
    }
}

/**
 * 清除错误
 */
void error_clear(uint16_t code) {
    if (code == 0) {
        // 清除所有错误
        g_current_error = ERR_SYS_OK;
        g_error_count = 0;
        for (int i = 0; i < MAX_ERROR_LOG; i++) {
            g_error_log[i].code = ERR_SYS_OK;
        }
        Serial.println("✅ 已清除所有错误");
    } else {
        // 清除指定错误
        if (g_current_error == code) {
            g_current_error = ERR_SYS_OK;
        }
        Serial.print("✅ 已清除错误：");
        Serial.println(code);
    }
}

/**
 * 获取当前错误码
 */
uint16_t error_get_current(void) {
    return g_current_error;
}

/**
 * 获取错误描述
 */
const char* error_get_message(uint16_t code) {
    for (int i = 0; i < ERROR_TABLE_SIZE; i++) {
        if (error_table[i].code == code) {
            return error_table[i].message;
        }
    }
    return "Unknown Error";
}

/**
 * 获取错误级别
 */
ErrorLevel_t error_get_level(uint16_t code) {
    for (int i = 0; i < ERROR_TABLE_SIZE; i++) {
        if (error_table[i].code == code) {
            return error_table[i].level;
        }
    }
    return ERR_LEVEL_ERROR;
}

/**
 * 在 OLED 上显示错误
 */
void error_display_oled(uint16_t code) {
    if (code == ERR_SYS_OK) return;
    
    ErrorLevel_t level = error_get_level(code);
    const char* message = error_get_message(code);
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    
    // 错误标题
    display.setTextColor(SSD1306_WHITE);
    display.println("=== ERROR ===");
    
    // 错误码
    display.print("Code: E");
    if (code < 100) display.print("00");
    else if (code < 1000) display.print("0");
    display.println(code);
    
    // 错误级别
    display.print("Level: ");
    switch (level) {
        case ERR_LEVEL_INFO:
            display.println("INFO");
            break;
        case ERR_LEVEL_WARNING:
            display.println("WARN");
            break;
        case ERR_LEVEL_ERROR:
            display.println("ERROR");
            break;
        case ERR_LEVEL_CRITICAL:
            display.println("CRIT!");
            break;
        default:
            display.println("NONE");
    }
    
    // 错误描述 (滚动显示长消息)
    display.println("-------------");
    display.setTextSize(1);
    
    // 分割长消息
    char msg_copy[32];
    strncpy(msg_copy, message, 31);
    msg_copy[31] = '\0';
    
    // 分两行显示
    if (strlen(msg_copy) > 16) {
        char line1[17], line2[17];
        strncpy(line1, msg_copy, 16);
        line1[16] = '\0';
        strncpy(line2, msg_copy + 16, 16);
        line2[16] = '\0';
        display.println(line1);
        display.println(line2);
    } else {
        display.println(msg_copy);
    }
    
    // 闪烁效果 (严重错误)
    if (level == ERR_LEVEL_CRITICAL) {
        display.invertDisplay(true);
        delay(500);
        display.invertDisplay(false);
    }
    
    display.display();
    
    // 打印到串口
    Serial.print("🚨 OLED 显示错误：");
    Serial.print(code);
    Serial.print(" - ");
    Serial.println(message);
}

/**
 * 检查是否有错误
 */
bool error_has_error(void) {
    return (g_current_error != ERR_SYS_OK);
}

/**
 * 检查是否有严重错误
 */
bool error_has_critical(void) {
    for (int i = 0; i < MAX_ERROR_LOG; i++) {
        if (g_error_log[i].level == ERR_LEVEL_CRITICAL) {
            return true;
        }
    }
    return false;
}

/**
 * 获取错误日志
 */
Error_t* error_get_log(uint8_t index) {
    if (index >= MAX_ERROR_LOG) return NULL;
    return &g_error_log[index];
}

/**
 * 打印错误信息到串口
 */
void error_print_serial(uint16_t code) {
    ErrorLevel_t level = error_get_level(code);
    const char* message = error_get_message(code);
    
    Serial.print("❌ 错误 ");
    Serial.print(code);
    Serial.print(" [");
    
    switch (level) {
        case ERR_LEVEL_INFO: Serial.print("INFO"); break;
        case ERR_LEVEL_WARNING: Serial.print("WARN"); break;
        case ERR_LEVEL_ERROR: Serial.print("ERROR"); break;
        case ERR_LEVEL_CRITICAL: Serial.print("CRIT"); break;
        default: Serial.print("NONE");
    }
    
    Serial.print("] ");
    Serial.println(message);
}

/**
 * 系统健康检查
 */
uint16_t system_health_check(void) {
    // 检查内存
    if (ESP.getFreeHeap() < 5000) {
        error_log(ERR_SYS_MEMORY_LOW, ERR_LEVEL_WARNING, 0xFF);
        return ERR_SYS_MEMORY_LOW;
    }
    
    // 检查 WiFi
    if (WiFi.status() != WL_CONNECTED) {
        // WiFi 断开不视为严重错误
        Serial.println("⚠️ WiFi 断开");
    }
    
    // 检查 I2C (OLED)
    Wire.beginTransmission(0x3C);
    if (Wire.endTransmission() != 0) {
        error_log(ERR_COM_I2C_FAIL, ERR_LEVEL_ERROR, 0xFF);
        return ERR_COM_I2C_FAIL;
    }
    
    // 所有检查通过
    return ERR_SYS_OK;
}

/**
 * 获取错误统计信息
 */
void error_get_stats(uint8_t* total, uint8_t* critical, uint8_t* error, uint8_t* warning) {
    *total = g_error_count;
    *critical = 0;
    *error = 0;
    *warning = 0;
    
    for (int i = 0; i < MAX_ERROR_LOG && i < g_error_count; i++) {
        switch (g_error_log[i].level) {
            case ERR_LEVEL_CRITICAL: (*critical)++; break;
            case ERR_LEVEL_ERROR: (*error)++; break;
            case ERR_LEVEL_WARNING: (*warning)++; break;
            default: break;
        }
    }
}

/**
 * 导出错误日志 (JSON 格式)
 */
String error_export_json(void) {
    String json = "[";
    
    for (int i = 0; i < MAX_ERROR_LOG && i < g_error_count; i++) {
        if (i > 0) json += ",";
        
        json += "{";
        json += "\"code\":" + String(g_error_log[i].code);
        json += ",\"level\":" + String(g_error_log[i].level);
        json += ",\"message\":\"" + String(g_error_log[i].message) + "\"";
        json += ",\"time\":" + String(g_error_log[i].timestamp);
        json += ",\"tower\":" + String(g_error_log[i].tower_id);
        json += "}";
    }
    
    json += "]";
    return json;
}
