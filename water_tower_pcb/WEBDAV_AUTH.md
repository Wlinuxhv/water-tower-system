# 📝 WebDAV 认证配置说明

> **问题**: Copaw 部署在飞牛 (fnOS) 容器中，WebDAV 需要认证  
> **解决方案**: 创建专用账号或配置匿名访问

---

## 🔐 方案 1: 创建专用账号 (推荐)

### 在飞牛 fnOS 中创建账号

1. **登录飞牛管理后台**
   - 地址：http://你的飞牛 IP:端口
   - 使用管理员账号登录

2. **创建新用户**
   ```
   用户名：copaw_upload
   密码：[设置一个强密码]
   权限：WebDAV 读写权限
   目录：/data/webdav/water_tower_project
   ```

3. **配置 WebDAV 权限**
   - 打开 Docker 容器管理
   - 找到 Copaw 容器
   - 编辑 WebDAV 配置
   - 添加用户 `copaw_upload`

4. **测试连接**
   ```bash
   # 测试认证
   curl -u copaw_upload:你的密码 http://192.168.0.59:5005/
   
   # 创建目录
   curl -X MKCOL -u copaw_upload:你的密码 \
     http://192.168.0.59:5005/water_tower_project
   ```

---

## 🔓 方案 2: 配置匿名访问 (仅限内网)

### 修改 WebDAV 配置

**如果使用 Alist (常见 WebDAV 方案)**:

1. **进入 Alist 管理后台**
   ```
   http://192.168.0.59:5005/@manage
   ```

2. **添加匿名用户**
   - 用户管理 → 添加用户
   - 用户名：`anonymous`
   - 密码：留空
   - 权限：只读或读写

3. **配置存储策略**
   - 存储 → 添加存储
   - 挂载路径：`/water_tower_project`
   - 驱动：本地存储
   - 根文件夹路径：`/data/webdav/water_tower`
   - 匿名访问：启用

### 如果使用其他 WebDAV 服务

**Nginx + WebDAV 模块**:
```nginx
location /webdav {
    dav_methods PUT MKCOL COPY MOVE;
    create_full_put_path on;
    dav_access user:rw group:r all:r;
    auth_basic off;  # 关闭认证 (仅内网)
    # 或者使用 basic auth
    # auth_basic "WebDAV";
    # auth_basic_user_file /etc/nginx/.davpasswd;
}
```

**Docker 运行参数**:
```bash
docker run -d \
  -p 5005:80 \
  -v /data/webdav:/data \
  -e ANONYMOUS=true \
  --name webdav
  bytemark/webdav
```

---

## 🚀 方案 3: 使用现有账号

如果你已经有飞牛管理员账号：

```bash
# 使用管理员账号上传
cd /app/working/water_tower_pcb
./upload_to_webdav.sh admin 你的管理员密码
```

---

## 📋 上传脚本使用说明

### 修改上传脚本

编辑 `/app/working/water_tower_pcb/upload_to_webdav.sh`:

```bash
#!/bin/bash
# 修改这些变量
WEBDAV_URL="http://192.168.0.59:5005"
USERNAME="你的用户名"     # 例如：copaw_upload
PASSWORD="你的密码"       # 例如：YourPassword123
```

### 运行上传

```bash
cd /app/working/water_tower_pcb
chmod +x upload_to_webdav.sh
./upload_to_webdav.sh
```

### 手动上传命令

```bash
# 1. 创建项目目录
curl -X MKCOL -u username:password \
  http://192.168.0.59:5005/water_tower_project

# 2. 上传 Gerber 文件
curl -T gerber/water_tower_master_gerber.zip \
  -u username:password \
  http://192.168.0.59:5005/water_tower_project/master_gerber.zip

# 3. 上传从机 Gerber
curl -T gerber/water_tower_slave_stc8g-F_Cu.gtl \
  -u username:password \
  http://192.168.0.59:5005/water_tower_project/slave/

# 4. 上传文档
curl -T README.md \
  -u username:password \
  http://192.168.0.59:5005/water_tower_project/
```

---

## ✅ 验证上传

### 方法 1: 浏览器访问

```
http://192.168.0.59:5005/water_tower_project/
```

输入用户名密码，查看文件列表。

### 方法 2: curl 命令

```bash
curl -u username:password \
  http://192.168.0.59:5005/water_tower_project/
```

### 方法 3: 挂载 WebDAV

**Linux**:
```bash
# 安装 davfs2
sudo apt-get install davfs2

# 挂载
sudo mount -t davfs http://192.168.0.59:5005/water_tower_project /mnt/webdav
```

**Windows**:
1. 打开"此电脑"
2. 右键 → "添加网络位置"
3. 输入：`http://192.168.0.59:5005/water_tower_project/`
4. 输入用户名密码

---

## 🔧 常见问题

### Q1: 提示 "Not Authorized"

**解决**:
- 检查用户名密码是否正确
- 确认 WebDAV 服务是否运行
- 查看 Docker 容器日志

```bash
# 查看容器日志
docker logs [webdav 容器名或 ID]

# 重启容器
docker restart [容器名]
```

### Q2: 无法创建目录 (MKCOL 失败)

**解决**:
- 检查用户是否有写权限
- 检查磁盘空间
- 检查文件夹所有者

```bash
# 检查权限
ls -la /data/webdav/

# 修改权限
chown -R www-data:www-data /data/webdav/
chmod -R 755 /data/webdav/
```

### Q3: 连接超时

**解决**:
- 检查防火墙设置
- 确认端口 5005 是否开放
- 检查网络连通性

```bash
# 测试端口
telnet 192.168.0.59 5005

# 或
nc -zv 192.168.0.59 5005
```

---

## 📞 飞牛 fnOS 特定配置

### Docker 容器配置

如果你使用 Docker 部署 WebDAV:

**docker-compose.yml**:
```yaml
version: '3'
services:
  webdav:
    image: bytemark/webdav:latest
    container_name: webdav
    ports:
      - "5005:80"
    volumes:
      - /data/webdav:/var/lib/dav
    environment:
      - AUTH_TYPE=Basic
      - USERNAME=copaw_upload
      - PASSWORD=YourPassword123
      - ANONYMOUS_METHODS=GET
    restart: unless-stopped
```

**启动命令**:
```bash
docker-compose up -d
```

### 访问日志

```bash
# 查看实时日志
docker logs -f webdav

# 查看访问日志
docker exec webdav cat /var/log/apache2/access.log
```

---

## 🎯 推荐配置

**对于你的场景 (Copaw 在飞牛容器)**:

1. **创建专用账号**: `copaw_upload`
2. **设置强密码**: 至少 12 位，包含大小写和数字
3. **限制目录**: 只能访问 `/water_tower_project`
4. **启用日志**: 记录所有上传操作
5. **定期备份**: 自动备份 WebDAV 数据

**配置示例**:
```bash
# 用户名：copaw_upload
# 密码：WaterTower2024!
# 目录：/data/webdav/water_tower_project
# 权限：读写，不能删除

# 测试
curl -u copaw_upload:WaterTower2024! \
  -X MKCOL \
  http://192.168.0.59:5005/water_tower_project
```

---

## 📝 下一步

请告诉我：
1. 你使用的是什么 WebDAV 服务？(Alist/Nginx/Docker 等)
2. 是否需要我帮你创建具体的配置脚本？
3. 或者你更倾向于哪种认证方案？

我可以帮你：
- ✅ 生成具体的配置文件
- ✅ 创建自动化部署脚本
- ✅ 配置匿名访问 (如果安全)
- ✅ 设置用户权限管理

---

**当前文件位置**: `/app/working/water_tower_pcb/`  
**Gerber 文件**: 已生成，等待上传  
**WebDAV 地址**: `http://192.168.0.59:5005/`
