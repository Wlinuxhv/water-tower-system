#!/bin/bash
# WebDAV 上传脚本 - 水塔监控系统 PCB 文件
# 用法：./upload_to_webdav.sh [用户名] [密码]

WEBDAV_URL="http://192.168.0.59:5005"
PROJECT_DIR="water_tower_project"
LOCAL_DIR="/app/working/water_tower_pcb"

# 默认认证 (请根据实际情况修改)
USERNAME=${1:-admin}
PASSWORD=${2:-admin}

echo "========================================"
echo "水塔监控系统 PCB 文件上传到 WebDAV"
echo "========================================"
echo ""
echo "WebDAV 地址：$WEBDAV_URL"
echo "项目目录：$PROJECT_DIR"
echo "本地目录：$LOCAL_DIR"
echo "用户名：$USERNAME"
echo ""

# 创建远程项目目录
echo "步骤 1: 创建远程项目目录..."
curl -X MKCOL --user "$USERNAME:$PASSWORD" "$WEBDAV_URL/$PROJECT_DIR" 2>/dev/null
if [ $? -eq 0 ]; then
    echo "✓ 目录创建成功"
else
    echo "⚠ 目录可能已存在或认证失败"
fi

# 创建子目录
for subdir in master slave gerber docs; do
    curl -X MKCOL --user "$USERNAME:$PASSWORD" "$WEBDAV_URL/$PROJECT_DIR/$subdir" 2>/dev/null
done
echo "✓ 子目录创建完成"
echo ""

# 上传 Gerber 文件
echo "步骤 2: 上传 Gerber 文件..."
cd "$LOCAL_DIR/gerber"
for file in *.gtl *.gbl *.gko *.gto *.gts *.gbp *.drl *.zip; do
    if [ -f "$file" ]; then
        echo "  上传：$file"
        curl -T "$file" --user "$USERNAME:$PASSWORD" "$WEBDAV_URL/$PROJECT_DIR/gerber/$file" 2>/dev/null
    fi
done
echo "✓ Gerber 文件上传完成"
echo ""

# 上传主机文件
echo "步骤 3: 上传主机 (Master) 文件..."
cd "$LOCAL_DIR/master"
for file in *.kicad_pcb *.csv; do
    if [ -f "$file" ]; then
        echo "  上传：$file"
        curl -T "$file" --user "$USERNAME:$PASSWORD" "$WEBDAV_URL/$PROJECT_DIR/master/$file" 2>/dev/null
    fi
done
echo "✓ 主机文件上传完成"
echo ""

# 上传从机文件
echo "步骤 4: 上传从机 (Slave) 文件..."
cd "$LOCAL_DIR/slave"
for file in *.kicad_pcb *.csv; do
    if [ -f "$file" ]; then
        echo "  上传：$file"
        curl -T "$file" --user "$USERNAME:$PASSWORD" "$WEBDAV_URL/$PROJECT_DIR/slave/$file" 2>/dev/null
    fi
done
echo "✓ 从机文件上传完成"
echo ""

# 上传文档
echo "步骤 5: 上传文档..."
cd "$LOCAL_DIR"
if [ -f "README.md" ]; then
    curl -T "README.md" --user "$USERNAME:$PASSWORD" "$WEBDAV_URL/$PROJECT_DIR/README.md" 2>/dev/null
    echo "  上传：README.md"
fi
echo "✓ 文档上传完成"
echo ""

# 验证上传
echo "步骤 6: 验证上传结果..."
echo "远程文件列表:"
curl --user "$USERNAME:$PASSWORD" "$WEBDAV_URL/$PROJECT_DIR/" 2>/dev/null | grep -oP '(?<=>)[^<]+' || echo "无法获取文件列表 (可能是认证问题)"
echo ""

echo "========================================"
echo "上传完成！"
echo "========================================"
echo ""
echo "嘉立创下单指引:"
echo "1. 访问：https://www.jlcpcb.com"
echo "2. 点击 '立即下单'"
echo "3. 上传 gerber/water_tower_master_gerber.zip"
echo "4. 确认 PCB 参数 (100x80mm, 2 层，1.6mm)"
echo "5. 选择 SMT 贴片服务 (可选)"
echo "6. 下单支付"
echo ""
echo "预计费用：¥270-400 (含运费)"
echo "预计交期：5-8 天"
echo ""
