# STC8G1K08 水塔从机 - Makefile
# 使用 SDCC 编译器

# 编译器配置
CC = sdcc
MCU = stc8g1k08
CFLAGS = --model-small --opt-code-size --no-xinit-opt
CFLAGS += --int-long-reent --float-reent

# 目录结构
SRC_DIR = src
INC_DIR = inc
BUILD_DIR = build
OUT_DIR = output

# 源文件
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/pan3031.c

# 头文件
INCS = -I$(INC_DIR)

# 输出文件
TARGET = $(OUT_DIR)/water_slave_stc8g

# 默认目标
all: dirs $(TARGET).hex $(TARGET).bin

# 创建目录
dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OUT_DIR)

# 编译
$(BUILD_DIR)/main.rel: $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

$(BUILD_DIR)/pan3031.rel: $(SRC_DIR)/pan3031.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

# 链接
$(TARGET).ihx: $(BUILD_DIR)/main.rel $(BUILD_DIR)/pan3031.rel
	$(CC) $(CFLAGS) $^ -o $@

# 生成 HEX 文件
$(TARGET).hex: $(TARGET).ihx
	objcopy -I ihex -O ihex $< $@
	@echo "HEX 文件生成完成：$@"
	@ls -lh $@

# 生成 BIN 文件
$(TARGET).bin: $(TARGET).ihx
	objcopy -I ihex -O binary $< $@
	@echo "BIN 文件生成完成：$@"

# 烧录 (需要 stcgal 工具)
flash: $(TARGET).hex
	@echo "开始烧录到 STC8G1K08..."
	@echo "请使用 STC-ISP 工具或 stcgal 烧录 $<"
	# stcgal -P stc8g -p COM3 -t 115200 $<

# 清理
clean:
	rm -rf $(BUILD_DIR) $(OUT_DIR)

# 查看大小
size: $(TARGET).ihx
	@echo "代码大小统计:"
	@packihx $< | wc -c
	@echo "字节 (最大 8192 字节)"

.PHONY: all dirs flash clean size
