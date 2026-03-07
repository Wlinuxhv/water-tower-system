# STC32G 水塔从机节点 - Makefile
# 使用 SDCC 编译器

# 编译器
CC = sdcc
OBJCOPY = objcopy

# 目标文件
TARGET = build/main

# 编译器标志
# STC32G 属于 8051 架构，使用 mmcs51 目标
CFLAGS = -mmcs51
CFLAGS += --model-small
CFLAGS += --opt-code-size
CFLAGS += --no-xinit-opt
CFLAGS += --std-sdcc99

# 输出格式
OUTPUT_HEX = $(TARGET).hex
OUTPUT_IHX = $(TARGET).ihx

# 默认目标
all: directories $(OUTPUT_HEX)

# 创建输出目录
directories:
	mkdir -p build

# 分别编译每个源文件
build/main.rel: src/main.c src/pan3031.h src/water_slave.h
	$(CC) $(CFLAGS) -Isrc -c src/main.c -o build/main.rel

build/pan3031.rel: src/pan3031.c src/pan3031.h
	$(CC) $(CFLAGS) -Isrc -c src/pan3031.c -o build/pan3031.rel

# 链接所有目标文件
$(TARGET).ihx: build/main.rel build/pan3031.rel
	$(CC) $(CFLAGS) build/main.rel build/pan3031.rel -o $(OUTPUT_IHX)

# 生成 HEX 文件
$(OUTPUT_HEX): $(TARGET).ihx
	$(OBJCOPY) -I ihex -O ihex $(OUTPUT_IHX) $(OUTPUT_HEX)

# 烧录 (需要 STC-ISP 工具)
flash: $(OUTPUT_HEX)
	@echo "请使用 STC-ISP 工具烧录 $(OUTPUT_HEX)"
	@echo "或使用 stcgal (Python 工具):"
	@echo "  stcgal -P stc89 -p COM3 -t $(OUTPUT_HEX)"

# 清理
clean:
	rm -rf build

# 重新编译
rebuild: clean all

# 帮助
help:
	@echo "STC32G 水塔从机节点 - 构建系统"
	@echo ""
	@echo "目标:"
	@echo "  all      - 编译项目 (默认)"
	@echo "  flash    - 烧录到芯片 (需手动操作)"
	@echo "  clean    - 清理构建文件"
	@echo "  rebuild  - 重新编译"
	@echo "  help     - 显示此帮助"
	@echo ""
	@echo "依赖:"
	@echo "  - SDCC 编译器 (http://sdcc.sourceforge.net)"
	@echo "  - stcgal (可选，用于自动烧录)"

.PHONY: all directories flash clean rebuild help
