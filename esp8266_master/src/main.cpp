/*
 * ESP8266 水塔监控主机 - 完整版
 * 
 * 功能:
 * 1. PAN3031 LoRa 通信 (一主多从)
 * 2. Web 服务器 (REST API)
 * 3. 自动水位控制
 * 4. 历史记录存储
 * 5. OLED 实时显示
 * 6. WiFi 连接手机 APP
 * 7. 缺水保护
 * 
 * 硬件:
 * - ESP8266 NodeMCU
 * - PAN3031 LoRa (SPI)
 * - OLED 0.96" I2C
 * - 缺水传感器 (GPIO)
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "pan3031.h"
#include "water_system.h"

// ==================== 引脚定义 ====================
// OLED (I2C)
#define OLED_SDA    D1  // GPIO5
#define OLED_SCL    D2  // GPIO4
#define OLED_RST    D3  // GPIO0

// PAN3031 (SPI)
#define PAN3031_CS   D5  // GPIO14
#define PAN3031_MOSI D7  // GPIO13
#define PAN3031_MISO D6  // GPIO12
#define PAN3031_SCK  D8  // GPIO15
#define PAN3031_IRQ  D4  // GPIO2

// 缺水传感器
#define WATER_LOW_SENSOR D0  // GPIO16

// 继电器控制 (可选，主机也可以控制水泵)
#define RELAY_1 D8   // GPIO15
#define RELAY_2 D9   // GPIO3

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
    Serial.println("\n=== 水塔监控主机启动 ===");
    
    // 初始化各模块
    setup_oled();
    setup_pan3031();
    setup_wifi();
    setup_server();
    
    // 显示欢迎界面
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("水塔监控系统");
    display.println("主机 v2.0");
    display.print("WiFi: ");
    display.println(sys_status.wifi_connected ? "OK" : "NG");
    display.print("LoRa: 434MHz SF7");
    display.display();
    
    Serial.println("系统初始化完成");
    Serial.print("IP 地址：");
    Serial.println(WiFi.localIP());
}

// ==================== 主循环 ====================
void loop() {
    server.handleClient();
    
    // PAN3031 通信处理
    handle_network_comm();
    
    // 自动控制逻辑
    if (sys_status.mode == MODE_AUTO) {
        process_auto_mode();
    }
    
    // 缺水检测
    check_well_water();
    
    // 历史记录保存 (每小时)
    save_history();
    
    // OLED 显示更新
    update_oled_display();
    
    delay(100);
}

// ==================== WiFi 设置 ====================
void setup_wifi() {
    Serial.print("连接 WiFi: ");
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
        Serial.println("\nWiFi 连接成功!");
        Serial.print("IP 地址：");
        Serial.println(WiFi.localIP());
    } else {
        sys_status.wifi_connected = false;
        Serial.println("\nWiFi 连接失败");
        // 创建 AP 热点
        WiFi.softAP("WaterTower-AP", "12345678");
        Serial.print("AP 模式 IP: ");
        Serial.println(WiFi.softAPIP());
    }
}

// ==================== OLED 设置 ====================
void setup_oled() {
    Wire.begin(OLED_SDA, OLED_SCL);
    
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED 初始化失败!");
        return;
    }
    
    display.clearDisplay();
    display.setTextColor(WHITE);
    Serial.println("OLED 初始化成功");
}

// ==================== PAN3031 设置 ====================
void setup_pan3031() {
    pan3031_init(PAN3031_CS, PAN3031_MOSI, PAN3031_MISO, PAN3031_SCK, PAN3031_IRQ);
    pan3031_set_freq(434000000);  // 434MHz
    pan3031_set_sf(7);            // SF7
    pan3031_set_bw(125000);       // 125kHz
    pan3031_set_power(20);        // 20dBm
    Serial.println("PAN3031 初始化成功");
}

// ==================== Web 服务器设置 ====================
void setup_server() {
    // API 路由
    server.on("/api/status", HTTP_GET, []() {
        String json = "{";
        json += "\"mode\":" + String(sys_status.mode) + ",";
        json += "\"wifi\":" + String(sys_status.wifi_connected) + ",";
        json += "\"well_water\":" + String(sys_status.well_water_ok) + ",";
        json += "\"towers\":[";
        for (int i = 0; i < tower_count; i++) {
            if (i > 0) json += ",";
            json += "{\"id\":" + String(towers[i].id);
            json += ",\"level\":" + String(towers[i].water_level);
            json += ",\"pump\":" + String(towers[i].pump_on);
            json += ",\"online\":" + String(towers[i].online);
            json += ",\"alarm\":" + String(towers[i].low_water_alarm);
            json += "}";
        }
        json += "]}";
        server.send(200, "application/json", json);
    });
    
    // 控制水泵
    server.on("/api/pump", HTTP_POST, []() {
        if (server.hasArg("tower") && server.hasArg("action")) {
            uint8_t tower_id = server.arg("tower").toInt();
            bool action = (server.arg("action") == "on");
            control_pump(tower_id, action);
            server.send(200, "text/plain", "OK");
        } else {
            server.send(400, "text/plain", "Invalid params");
        }
    });
    
    // 切换模式
    server.on("/api/mode", HTTP_POST, []() {
        if (server.hasArg("mode")) {
            sys_status.mode = (SystemMode)server.arg("mode").toInt();
            server.send(200, "text/plain", "OK");
        } else {
            server.send(400, "text/plain", "Invalid params");
        }
    });
    
    // 获取历史记录
    server.on("/api/history", HTTP_GET, []() {
        if (server.hasArg("towerId")) {
            uint8_t tower_id = server.arg("towerId").toInt();
            send_history_json(tower_id);
        } else {
            server.send(400, "text/plain", "Missing towerId");
        }
    });
    
    // 获取所有水塔列表
    server.on("/api/towers", HTTP_GET, []() {
        String json = "[";
        for (int i = 0; i < tower_count; i++) {
            if (i > 0) json += ",";
            json += "{\"id\":" + String(towers[i].id);
            json += ",\"name\":\"水塔" + String(i+1) + "\"";
            json += ",\"level\":" + String(towers[i].water_level);
            json += ",\"pump\":" + String(towers[i].pump_on);
            json += ",\"online\":" + String(towers[i].online);
            json += "}";
        }
        json += "]";
        server.send(200, "application/json", json);
    });
    
    server.begin();
    Serial.println("Web 服务器启动");
}

// ==================== PAN3031 通信处理 ====================
void handle_network_comm() {
    // 接收从机数据
    uint8_t rx_data[32];
    uint8_t rx_len;
    
    if (pan3031_receive(rx_data, &rx_len)) {
        // 解析数据包
        if (rx_len >= 5) {
            uint8_t src_id = rx_data[0];
            uint8_t cmd = rx_data[1];
            uint8_t data_len = rx_data[2];
            
            Serial.print("收到从机 ");
            Serial.print(src_id);
            Serial.print(" 数据：");
            Serial.println(cmd);
            
            if (cmd == CMD_HEARTBEAT && data_len >= 2) {
                // 更新水塔数据
                uint8_t idx = find_tower(src_id);
                if (idx < MAX_TOWERS) {
                    towers[idx].water_level = rx_data[3];
                    towers[idx].well_water_ok = (rx_data[4] & 0x02) >> 1;
                    towers[idx].pump_on = rx_data[4] & 0x01;
                    towers[idx].online = true;
                    towers[idx].last_update = millis();
                    
                    Serial.print("水塔 ");
                    Serial.print(src_id);
                    Serial.print(" 水位：");
                    Serial.print(towers[idx].water_level);
                    Serial.println("%");
                }
            }
        }
    }
    
    // 检查离线从机 (30 秒无心跳)
    uint32_t now = millis();
    for (int i = 0; i < tower_count; i++) {
        if (now - towers[i].last_update > 30000) {
            towers[i].online = false;
        }
    }
}

// ==================== 自动模式处理 ====================
void process_auto_mode() {
    static uint32_t last_check = 0;
    
    // 每 5 秒检查一次
    if (millis() - last_check < 5000) return;
    last_check = millis();
    
    // 检查缺水保护
    if (!sys_status.well_water_ok) {
        // 关闭所有水泵
        for (int i = 0; i < tower_count; i++) {
            if (towers[i].pump_on) {
                control_pump(i, false);
            }
        }
        Serial.println("井水缺水，关闭所有水泵!");
        return;
    }
    
    // 根据水位自动控制
    for (int i = 0; i < tower_count; i++) {
        if (!towers[i].online) continue;
        
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

// ==================== 缺水检测 ====================
void check_well_water() {
    bool water_ok = digitalRead(WATER_LOW_SENSOR);
    if (water_ok != sys_status.well_water_ok) {
        sys_status.well_water_ok = water_ok;
        Serial.print("井水状态：");
        Serial.println(water_ok ? "正常" : "缺水!");
        
        if (!water_ok) {
            // 缺水报警
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("⚠️ 井水缺水!");
            display.println("所有水泵已关闭");
            display.display();
        }
    }
}

// ==================== 水泵控制 ====================
void control_pump(uint8_t tower_id, bool on) {
    if (tower_id >= tower_count) return;
    
    // 发送控制命令到从机
    uint8_t tx_data[6];
    tx_data[0] = 0x00;  // 主机地址
    tx_data[1] = towers[tower_id].id;  // 目标从机
    tx_data[2] = CMD_PUMP_CTRL;
    tx_data[3] = 1;
    tx_data[4] = on ? 1 : 0;
    
    pan3031_send(tx_data, 5);
    towers[tower_id].pump_on = on;
    
    Serial.print("水塔 ");
    Serial.print(tower_id);
    Serial.print(" 水泵：");
    Serial.println(on ? "开启" : "关闭");
}

// ==================== OLED 显示更新 ====================
void update_oled_display() {
    static uint32_t last_update = 0;
    if (millis() - last_update < 1000) return;
    last_update = millis();
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    
    // 状态栏
    display.print("模式：");
    display.print(sys_status.mode == MODE_AUTO ? "自动" : "手动");
    display.print(" | 井水：");
    display.println(sys_status.well_water_ok ? "OK" : "缺水!");
    
    // 各水塔水位 (最多显示 4 个)
    for (int i = 0; i < tower_count && i < 4; i++) {
        display.print("T");
        display.print(i + 1);
        display.print(":");
        
        // 水位条
        uint8_t level = towers[i].water_level;
        uint8_t bars = level / 10;
        for (int j = 0; j < 10; j++) {
            display.print(j < bars ? "|" : ".");
        }
        display.print(level);
        display.print("%");
        display.println(towers[i].pump_on ? " [泵]" : "");
    }
    
    display.display();
}

// ==================== 历史记录保存 ====================
void save_history() {
    uint32_t now = millis() / 1000;
    
    // 每小时保存一次
    if (now - sys_status.last_save < 3600) return;
    sys_status.last_save = now;
    
    for (int i = 0; i < tower_count; i++) {
        uint8_t idx = towers[i].history_index;
        towers[i].history[idx].timestamp = now;
        towers[i].history[idx].water_level = towers[i].water_level;
        towers[i].history[idx].pump_status = towers[i].pump_on;
        
        // 循环缓冲区
        towers[i].history_index = (idx + 1) % HISTORY_SIZE;
    }
    
    Serial.println("历史记录已保存");
}

// ==================== 获取历史记录 API ====================
void send_history_json(uint8_t tower_id) {
    uint8_t idx = find_tower(tower_id);
    if (idx >= MAX_TOWERS) {
        server.send(404, "text/plain", "Tower not found");
        return;
    }
    
    String json = "[";
    bool first = true;
    
    // 从最旧的数据开始
    uint8_t start_idx = towers[idx].history_index;
    for (int i = 0; i < HISTORY_SIZE; i++) {
        uint8_t h_idx = (start_idx + i) % HISTORY_SIZE;
        if (towers[idx].history[h_idx].timestamp > 0) {
            if (!first) json += ",";
            first = false;
            
            json += "{";
            json += "\"towerId\":" + String(tower_id);
            json += ",\"timestamp\":" + String(towers[idx].history[h_idx].timestamp * 1000UL);
            json += ",\"waterLevel\":" + String(towers[idx].history[h_idx].water_level);
            json += ",\"pumpStatus\":" + String(towers[idx].history[h_idx].pump_status);
            json += "}";
        }
    }
    
    json += "]";
    server.send(200, "application/json", json);
}

// ==================== 辅助函数 ====================
uint8_t find_tower(uint8_t id) {
    for (int i = 0; i < tower_count; i++) {
        if (towers[i].id == id) return i;
    }
    // 添加新水塔
    if (tower_count < MAX_TOWERS) {
        towers[tower_count].id = id;
        towers[tower_count].online = false;
        towers[tower_count].water_level = 0;
        return tower_count++;
    }
    return MAX_TOWERS;
}
