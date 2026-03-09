# 🚀 飞牛 (fnOS) WebDAV 快速访问指南

> **Copaw 部署在飞牛容器中** | **快速配置指南**

---

## 📡 飞牛 WebDAV 常见访问方式

### 方式 1: Alist (最常见) ⭐

**默认配置**:
```
URL: http://192.168.0.59:5244/dav
端口：5244 (默认) 或 5005 (自定义)
认证：需要用户名密码
```

**测试命令**:
```bash
# 测试根目录
curl -I http://192.168.0.59:5005/

# 测试 /dav 路径
curl -I http://192.168.0.59:5005/dav/

# 带认证测试
curl -u admin:密码 http://192.168.0.59:5005/dav/
```

**Alist 管理后台**:
```
http://192.168.0.59:5005/@manage
默认用户名：admin
默认密码：查看 Docker 环境变量或容器日志
```

---

### 方式 2: Docker WebDAV 容器

**常见镜像**:
- `bytemark/webdav`
- `morniel/webdav`
- `liquidweb/webdav`

**测试命令**:
```bash
# 直接访问根目录
curl http://192.168.0.59:5005/

# 创建目录测试
curl -X MKCOL -u user:pass http://192.168.0.59:5005/test
```

**查看容器配置**:
```bash
# 列出所有容器
docker ps | grep -i webdav

# 查看容器环境变量
docker inspect [容器名] | grep -A 5 "Env"

# 查看容器日志
docker logs [容器名]
```

---

### 方式 3: Nginx WebDAV 模块

**配置位置**:
```
/etc/nginx/nginx.conf
或
/etc/nginx/conf.d/webdav.conf
```

**典型配置**:
```nginx
location /webdav {
    dav_methods PUT MKCOL COPY MOVE;
    autoindex on;
    auth_basic "WebDAV";
    auth_basic_user_file /etc/nginx/.davpasswd;
}
```

**访问 URL**:
```
http://192.168.0.59:5005/webdav/
```

---

## 🔐 获取认证信息的方法

### 方法 1: 查看 Docker 环境变量

```bash
# 找到 WebDAV 容器
docker ps | grep -i webdav

# 查看环境变量
docker inspect [容器名或 ID] | grep -A 20 "Env"
```

**常见环境变量**:
```bash
USERNAME=admin
PASSWORD=你的密码
# 或
DAV_USERNAME=copaw
DAV_PASSWORD=你的密码
```

### 方法 2: 查看 Docker Compose 文件

```bash
# 查找 docker-compose.yml
find / -name "docker-compose.yml" 2>/dev/null | xargs grep -l webdav

# 或查看常见位置
cat /opt/docker-compose.yml
cat /root/docker-compose.yml
cat /home/docker/docker-compose.yml
```

**典型配置**:
```yaml
version: '3'
services:
  webdav:
    image: bytemark/webdav
    environment:
      - AUTH_TYPE=Basic
      - USERNAME=copaw_upload
      - PASSWORD=YourPassword123
```

### 方法 3: 查看飞牛管理后台

1. **登录飞牛**: http://192.168.0.59:端口
2. **进入应用管理**
3. **找到 WebDAV 应用**
4. **查看设置/配置**
5. **获取用户名和密码**

### 方法 4: 重置密码

**Alist 重置密码**:
```bash
# 进入 Alist 容器
docker exec -it [alist 容器名] sh

# 重置 admin 密码
./alist admin set YourNewPassword

# 或查看随机密码
./alist admin random
```

**Docker WebDAV 重置**:
```bash
# 修改 docker-compose.yml
environment:
  - PASSWORD=NewPassword123

# 重启容器
docker-compose restart webdav
```

---

## 🚀 快速配置步骤

### 步骤 1: 确认 WebDAV 服务类型

```bash
# 测试不同路径
echo "测试根目录..."
curl -s -o /dev/null -w "%{http_code}" http://192.168.0.59:5005/

echo -e "\n测试/dav..."
curl -s -o /dev/null -w "%{http_code}" http://192.168.0.59:5005/dav/

echo -e "\n测试/webdav..."
curl -s -o /dev/null -w "%{http_code}" http://192.168.0.59:5005/webdav/
```

**响应码含义**:
- `200`: 成功访问
- `401`: 需要认证 (正常)
- `404`: 路径不存在
- `000`: 服务未运行

### 步骤 2: 创建专用账号

**推荐配置**:
```
用户名：copaw_upload
密码：WaterTower2024!
目录：/data/webdav/water_tower_project
权限：读写 (不能删除)
```

**Alist 创建用户**:
1. 访问管理后台：`http://192.168.0.59:5005/@manage`
2. 用户管理 → 添加用户
3. 填写信息并设置权限

**Docker WebDAV 创建用户**:
```bash
# 生成密码文件
htpasswd -c /data/.davpasswd copaw_upload
# 输入密码

# 重启容器
docker restart webdav
```

### 步骤 3: 测试连接

```bash
# 测试认证
curl -u copaw_upload:WaterTower2024! \
  http://192.168.0.59:5005/dav/

# 创建项目目录
curl -X MKCOL -u copaw_upload:WaterTower2024! \
  http://192.168.0.59:5005/dav/water_tower_project

# 上传测试文件
echo "Test" > test.txt
curl -T test.txt -u copaw_upload:WaterTower2024! \
  http://192.168.0.59:5005/dav/water_tower_project/test.txt

# 列出文件
curl -u copaw_upload:WaterTower2024! \
  http://192.168.0.59:5005/dav/water_tower_project/
```

### 步骤 4: 使用上传脚本

修改 `/app/working/water_tower_pcb/upload_to_webdav.sh`:

```bash
#!/bin/bash
WEBDAV_URL="http://192.168.0.59:5005/dav"  # 根据你的实际路径
USERNAME="copaw_upload"
PASSWORD="WaterTower2024!"
PROJECT_DIR="water_tower_project"
```

运行上传:
```bash
cd /app/working/water_tower_pcb
./upload_to_webdav.sh
```

---

## ❓ 常见问题

### Q1: 连接被拒绝

**解决**:
```bash
# 检查容器是否运行
docker ps | grep webdav

# 检查端口是否监听
netstat -tlnp | grep 5005

# 检查防火墙
ufw status
# 或
iptables -L -n | grep 5005
```

### Q2: 认证失败

**解决**:
1. 确认用户名密码正确
2. 检查大小写
3. 查看容器日志：`docker logs webdav`
4. 重置密码

### Q3: 无法创建目录

**解决**:
```bash
# 检查目录权限
ls -la /data/webdav/

# 修改权限
chown -R www-data:www-data /data/webdav/
chmod -R 755 /data/webdav/

# 检查磁盘空间
df -h
```

### Q4: 找不到 WebDAV 容器

**解决**:
```bash
# 列出所有容器
docker ps -a | grep -i dav

# 如果没有，需要安装
# Alist 安装:
docker run -d --restart=always -v /etc/alist:/opt/alist/data -p 5244:5244 -e PUID=0 -e PGID=0 --name="alist" xhofe/alist:latest

# 或简单 WebDAV:
docker run -d --name webdav -p 5005:80 -v /data/webdav:/var/lib/dav -e USERNAME=copaw -e PASSWORD=123456 bytemark/webdav
```

---

## 🎯 推荐配置 (你的场景)

基于你的环境 (Copaw 在飞牛容器)，推荐:

### 方案 A: 使用现有 Alist

```bash
# 1. 登录 Alist 管理后台
http://192.168.0.59:5005/@manage

# 2. 创建用户
用户名：copaw_upload
密码：[设置强密码]

# 3. 添加存储
路径：/water_tower_project
本地存储：/data/webdav/water_tower

# 4. 测试
curl -u copaw_upload:密码 http://192.168.0.59:5005/dav/water_tower_project/
```

### 方案 B: 新建专用 WebDAV

```bash
# 1. 创建新容器
docker run -d \
  --name water-tower-webdav \
  --restart=always \
  -p 5006:80 \
  -v /data/water_tower_webdav:/var/lib/dav \
  -e AUTH_TYPE=Basic \
  -e USERNAME=copaw_upload \
  -e PASSWORD=WaterTower2024! \
  bytemark/webdav

# 2. 测试
curl -u copaw_upload:WaterTower2024! http://192.168.0.59:5006/
```

---

## 📞 需要帮助？

请提供以下信息，我可以帮你配置:

1. **飞牛版本**: _______
2. **当前 WebDAV 服务**: (Alist/Docker/Nginx/其他)
3. **访问 URL**: http://192.168.0.59:5005/___
4. **是否有管理员权限**: 是/否

我可以帮你:
- ✅ 远程配置 WebDAV
- ✅ 创建专用账号
- ✅ 测试上传功能
- ✅ 生成配置文件

---

**当前状态**: ⏳ 等待 WebDAV 认证信息  
**下一步**: 提供认证信息 → 上传 Gerber 文件 → 嘉立创下单
