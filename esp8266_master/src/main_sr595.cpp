/*
 * ESP8266 水塔监控主机 - 74HC595 继电器控制版
 * 
 * 功能:
 * 1. PAN3031 LoRa 通信 (一主多从)
 * 2. 74HC595 SPI 复用控制 8 路继电器
 * 3. Web 服务器 (REST API)
 * 4. 自动水位控制
 * 5. 历史记录存储
 * 6. OLED 实时显示
 * 7. WiFi 连接手机 APP
 * 8. 缺水保护
 * 
 * 硬件:
 * - ESP8266 NodeMCU
 * - PAN3031 LoRa (SPI)
 * - 74HC595 移位寄存器 (SPI 复用)
 * - 8 路继电器模块
 * - OLED 0.96" I2C
 * - 缺水传感器 (GPIO)
 * 
 * SPI 复用说明:
 * - SCK (D5): LoRa 和 74HC595 共用
 * - MOSI (D7): LoRa 和 74HC595 共用
 * - CS1 (D8): LoRa 片选
 * - CS2 (D4): 74HC595 锁存
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "pan3031.h"
#include "water_system.h"
#include "sr595.h"  // 74HC595 驱动

// ==================== 引脚定义 ====================
// OLED (I2C)
#define OLED_SDA    D1  // GPIO5
#define OLED_SCL    D2  // GPIO4
#define OLED_RST    D3  // GPIO0

// PAN3031 LoRa (SPI)
#define PAN3031_CS   D8  // GPIO15 - 片选
#define PAN3031_MOSI D7  // GPIO13
#define PAN3031_MISO D6  // GPIO12
#define PAN3031_SCK  D5  // GPIO14
#define PAN3031_IRQ  -1  // 禁用 (D4 用于 74HC595 锁存)

// 74HC595 (SPI 复用)
// SCK 和 MOSI 与 LoRa 共用
// 锁存引脚使用 D4 (原 LoRa IRQ)
#define SR595_LATCH  D4  // GPIO2 - 74HC595 锁存

// 缺水传感器
#define WATER_LOW_SENSOR D0  // GPIO16

// ==================== WiFi 配置 ====================
const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASS = "YourWiFiPassword";

// ==================== 全局变量 ====================
ESP8266WebServer server(80);
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RST);

TowerData towers[MAX_TOWERS];
uint8_t tower_count = 0;

SystemStatus sys_status = {
    .mode = MODE_AUTO,
    .wifi_connected = false,
    .well_water_ok = true,
    .last_heartbeat = 0,
    .last_save = 0
};

// ==================== 函数声明 ====================
void setup_wifi();
void setup_oled();
void setup_pan3031();
void setup_sr595();
void setup_server();
void update_oled_display();
void handle_network_comm();
void check_well_water();
void control_pump(uint8_t tower_id, bool on);
void process_auto_mode();
void save_history();
void send_history_json(uint8_t tower_id);
uint8_t find_tower(uint8_t id);

// ==================== 初始化 ====================
void setup() {
    Serial.begin(115200);
    Serial.println("\n=== 水塔监控主机启动 v2.1 (74HC595) ===");
    
    // 初始化各模块
    setup_oled();
    setup_pan3031();
    setup_sr595();  // 新增：74HC595 初始化
    setup_wifi();
    setup_server();
    
    // 显示欢迎界面
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("水塔监控系统");
    display.println("主机 v2.1");
    display.print("WiFi: ");
    display.println(sys_status.wifi_connected ? "OK" : "NG");
    display.print("LoRa: 434MHz SF7");
    display.println("Relay: 74HC595 x8");
    display.display();
    
    Serial.println("系统初始化完成");
    Serial.print("支持最多 ");
    Serial.print(MAX_TOWERS);
    Serial.println(" 个水塔");
}

// ==================== 模块初始化 ====================

void setup_oled() {
    Wire.begin(OLED_SDA, OLED_SCL);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.setTextColor(SSD1306_WHITE);
}

void setup_pan3031() {
    pan3031_init();
    pan3031_set_freq(434000000UL);
    pan3031_set_sf(7);
    pan3031_set_bw(125000UL);
    pan3031_set_power(20);
    Serial.println("✅ PAN3031 LoRa 初始化完成");
}

void setup_sr595() {
    // 初始化 74HC595
    sr595_init();
    
    // 测试继电器
    Serial.println("🧪 测试继电器...");
    sr595_set_all(0xFF);  // 全部开启
    delay(200);
    sr595_set_all(0x00);  // 全部关闭
}

void setup_wifi() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 30) {
        delay(500);
        Serial.print(".");
        timeout++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        sys_status.wifi_connected = true;
        Serial.println("\n✅ WiFi connected");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        sys_status.wifi_connected = false;
        Serial.println("\n❌ WiFi timeout");
    }
}

void setup_server() {
    // 系统状态
    server.on("/api/status", HTTP_GET, []() {
        String json = "{";
        json += "\"wifi\":" + String(sys_status.wifi_connected ? "true" : "false");
        json += ",\"mode\":\"" + String(sys_status.mode == MODE_AUTO ? "AUTO" : "MANUAL") + "\"";
        json += ",\"well_water\":" + String(sys_status.well_water_ok ? "true" : "false");
        json += ",\"towers\":" + String(tower_count);
        json += "}";
        server.send(200, "application/json", json);
    });
    
    // 获取水塔列表
    server.on("/api/towers", HTTP_GET, []() {
        String json = "[";
        for (int i = 0; i < tower_count; i++) {
            if (i > 0) json += ",";
            json += "{";
            json += "\"id\":" + String(towers[i].id);
            json += ",\"level\":" + String(towers[i].water_level);
            json += ",\"pump\":" + String(towers[i].pump_on ? "true" : "false");
            json += "}";
        }
        json += "]";
        server.send(200, "application/json", json);
    });
    
    // 控制水泵
    server.on("/api/pump", HTTP_POST, []() {
        uint8_t tower_id = server.arg("id").toInt();
        bool action = server.arg("action") == "on";
        control_pump(tower_id, action);
        server.send(200, "text/plain", "OK");
    });
    
    // 模式切换
    server.on("/api/mode", HTTP_POST, []() {
        String mode = server.arg("mode");
        sys_status.mode = (mode == "AUTO") ? MODE_AUTO : MODE_MANUAL;
        server.send(200, "text/plain", "OK");
    });
    
    server.begin();
    Serial.println("✅ Web 服务器启动");
}

// ==================== 水泵控制 (使用 74HC595) ====================

/**
 * 控制水泵
 * 使用 74HC595 控制继电器
 */
void control_pump(uint8_t tower_id, bool on) {
    if (tower_id >= tower_count || tower_id > 7) {
        Serial.println("❌ 无效的水塔 ID");
        return;
    }
    
    // 使用 74HC595 控制继电器
    if (on) {
        sr595_relay_on(tower_id);
        towers[tower_id].pump_on = true;
        Serial.print("✅ 水塔 ");
        Serial.print(tower_id);
        Serial.println(" 开启水泵");
    } else {
        sr595_relay_off(tower_id);
        towers[tower_id].pump_on = false;
        Serial.print("❌ 水塔 ");
        Serial.print(tower_id);
        Serial.println(" 关闭水泵");
    }
    
    // 更新 OLED 显示
    update_oled_display();
}

/**
 * 紧急停止 - 关闭所有水泵
 */
void emergency_stop() {
    Serial.println("🚨 紧急停止！关闭所有水泵");
    sr595_set_all(0x00);  // 使用 74HC595 关闭所有继电器
    
    for (int i = 0; i < tower_count; i++) {
        towers[i].pump_on = false;
    }
    
    update_oled_display();
}

// ==================== 自动控制逻辑 ====================

void process_auto_mode() {
    if (sys_status.mode != MODE_AUTO) return;
    
    // 检查井水缺水
    check_well_water();
    if (!sys_status.well_water_ok) {
        emergency_stop();
        return;
    }
    
    // 自动水位控制
    for (int i = 0; i < tower_count; i++) {
        // 水位低于 20% 开启水泵
        if (towers[i].water_level < 20 && !towers[i].pump_on) {
            control_pump(i, true);
            Serial.print("水塔 ");
            Serial.print(i);
            Serial.println(" 水位低，开启水泵");
        }
        // 水位高于 90% 关闭水泵
        else if (towers[i].water_level > 90 && towers[i].pump_on) {
            control_pump(i, false);
            Serial.print("水塔 ");
            Serial.print(i);
            Serial.println(" 水位高，关闭水泵");
        }
    }
}

// ==================== OLED 显示 ====================

void update_oled_display() {
    display.clearDisplay();
    display.setCursor(0, 0);
    
    // 状态栏
    display.print("Mode:");
    display.print(sys_status.mode == MODE_AUTO ? "AUTO" : "MANUAL");
    display.print(" | Well:");
    display.println(sys_status.well_water_ok ? "OK" : "LOW");
    
    // 水塔状态
    for (int i = 0; i < tower_count && i < 4; i++) {
        display.print("T");
        display.print(i);
        display.print(":");
        display.print(towers[i].water_level);
        display.print("% ");
        display.println(towers[i].pump_on ? "[PUMP]" : "    ");
    }
    
    display.display();
}

// ==================== 主循环 ====================

void loop() {
    // 处理 Web 服务器
    server.handleClient();
    
    // 处理 LoRa 通信
    handle_network_comm();
    
    // 自动控制
    if (sys_status.mode == MODE_AUTO) {
        process_auto_mode();
    }
    
    // 更新显示
    update_oled_display();
    
    delay(100);
}

// ==================== LoRa 通信处理 ====================

void handle_network_comm() {
    uint8_t rx_data[8];
    uint8_t len = pan3031_receive(rx_data, 8);
    
    if (len < 4) return;
    
    // 处理传感器数据
    if (rx_data[1] == CMD_SENSOR_DATA) {
        uint8_t tower_id = rx_data[0];
        uint8_t water_level = rx_data[3];
        uint8_t well_ok = rx_data[4];
        
        // 查找或添加水塔
        int idx = find_tower(tower_id);
        if (idx < 0 && tower_count < MAX_TOWERS) {
            idx = tower_count++;
            towers[idx].id = tower_id;
            towers[idx].water_level = 0;
            towers[idx].pump_on = false;
        }
        
        if (idx >= 0) {
            towers[idx].water_level = water_level;
            sys_status.well_water_ok = well_ok;
        }
    }
}

void check_well_water() {
    sys_status.well_water_ok = (digitalRead(WATER_LOW_SENSOR) == HIGH);
}

uint8_t find_tower(uint8_t id) {
    for (int i = 0; i < tower_count; i++) {
        if (towers[i].id == id) return i;
    }
    return -1;
}

void save_history() {
    // TODO: 保存到 EEPROM
}

void send_history_json(uint8_t tower_id) {
    // TODO: 发送历史记录
}
