# Self-Hosted Runner 检测与配置指南

## 检测当前 Runner 状态

### 1. 检查 Runner 是否在线

访问 GitHub 仓库页面：
```
https://github.com/Wlinuxhv/water-tower-system/settings/actions/runners
```

查看 Runner 状态：
- 🟢 **Online** - 可以接受任务
- 🔴 **Offline** - 需要检查连接

### 2. 手动触发测试构建

运行以下命令触发测试：

```bash
# 使用 GitHub CLI
gh workflow run embedded-ci.yml

# 或访问网页
https://github.com/Wlinuxhv/water-tower-system/actions/workflows/embedded-ci.yml
```

---

## Windows Runner 环境要求

### 必需软件

#### 1. SDCC 编译器

**安装方法 1: Chocolatey (推荐)**
```powershell
# 安装 Chocolatey (如果未安装)
Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

# 安装 SDCC
choco install sdcc -y
```

**安装方法 2: 手动下载**
1. 访问：https://sourceforge.net/projects/sdcc/files/
2. 下载最新 Windows 版本
3. 解压到 `C:\Program Files\SDCC`
4. 添加 `C:\Program Files\SDCC\bin` 到 PATH 环境变量

**验证安装:**
```powershell
sdcc -v
```

预期输出:
```
SDCC : mcs51/gbz80/z80/z180/r2k/r2ka/r3ka/gbz80/ds390/pic16/pic14/TININative/ds400/ez80_90/z80/stm8 4.2.0 #13666 (MINGW64)
```

#### 2. GNU Binutils (objcopy)

**安装方法 1: Chocolatey**
```powershell
choco install binutils -y
```

**安装方法 2: MSYS2**
```powershell
choco install msys2 -y
# 然后在 MSYS2 中运行
pacman -S mingw-w64-x86_64-binutils
```

**验证安装:**
```powershell
objcopy --version
```

#### 3. Git

通常 GitHub Runner 已预装 Git。

**验证:**
```powershell
git --version
```

---

## 环境问题排查

### 问题 1: SDCC 未找到

**症状:**
```
❌ SDCC 未安装
请安装 SDCC: choco install sdcc
```

**解决:**
```powershell
# 检查 SDCC 安装路径
$sdccPaths = @(
  "C:\ProgramData\chocolatey\lib\sdcc\tools\sdcc\bin",
  "C:\Program Files\SDCC\bin",
  "C:\sdcc\bin"
)

foreach ($path in $sdccPaths) {
  if (Test-Path $path) {
    Write-Host "找到 SDCC: $path"
    $env:Path = "$path;$env:Path"
    echo "$path" >> $env:GITHUB_PATH
    break
  }
}

# 验证
sdcc -v
```

### 问题 2: objcopy 未找到

**症状:**
```
⚠️ objcopy 未安装，尝试使用 SDCC 自带工具
```

**解决:**
方案 1: 安装 binutils
```powershell
choco install binutils -y
```

方案 2: 使用 SDCC 自带的 packihx
```powershell
# SDCC 自带 packihx 工具可以转换格式
packihx output/water_slave_stc8g.ihx > output/water_slave_stc8g.hex
```

### 问题 3: 代码大小超限

**症状:**
```
⚠️ 代码大小超过 STC8G1K08 的 8KB Flash 限制!
```

**解决:**
1. 优化编译选项：
   ```
   --opt-code-size
   --model-small
   ```

2. 简化代码：
   - 移除未使用的函数
   - 使用更小的数据类型
   - 减少字符串常量

3. 检查代码大小：
   ```powershell
   $hexSize = (Get-Item "output/water_slave_stc8g.hex").Length
   Write-Host "代码大小：$hexSize / 8192 字节"
   ```

---

## 完整环境配置脚本

在 Windows Runner 上运行此脚本一次性配置环境：

```powershell
# 1. 安装 Chocolatey
Set-ExecutionPolicy Bypass -Scope Process -Force
[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

# 2. 安装必需软件
choco install sdcc binutils git -y

# 3. 验证安装
Write-Host "=== 验证安装 ==="
Write-Host "SDCC 版本:"
sdcc -v

Write-Host "`nObjcopy 版本:"
objcopy --version | Select-Object -First 1

Write-Host "`nGit 版本:"
git --version

Write-Host "`n✅ 环境配置完成!"
```

---

## 测试构建

### 方法 1: 本地测试

在 Windows Runner 上手动编译测试：

```powershell
cd C:\actions-runner\_work\water-tower-system\water-tower-system\slave_node_stc8g

# 清理
Remove-Item -Recurse -Force build, output -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path build, output | Out-Null

# 编译
sdcc -mmcs51 --model-small --opt-code-size -c src/main.c -o build/main.rel
sdcc -mmcs51 --model-small --opt-code-size -c src/pan3031.c -o build/pan3031.rel
sdcc -mmcs51 --model-small --opt-code-size -c src/sc09b.c -o build/sc09b.rel

# 链接
sdcc -mmcs51 --model-small build/*.rel -o output/water_slave_stc8g.ihx

# 生成 HEX
objcopy -I ihex -O ihx output/water_slave_stc8g.ihx output/water_slave_stc8g.hex

# 检查大小
$size = (Get-Item "output/water_slave_stc8g.hex").Length
Write-Host "HEX 大小：$size 字节 (最大 8192 字节)"

if ($size -le 8192) {
  Write-Host "✅ 编译成功!"
} else {
  Write-Host "❌ 代码过大!"
}
```

### 方法 2: GitHub Actions 测试

1. 推送代码到 GitHub
2. 访问 Actions 页面
3. 点击 "Embedded CI - STC8G Build"
4. 查看运行日志

---

## 构建产物说明

### 输出文件

| 文件 | 用途 | 大小 |
|------|------|------|
| `water_slave_stc8g.ihx` | SDCC 原始输出 | ~8KB |
| `water_slave_stc8g.hex` | Intel HEX 格式 (用于烧录) | ~8KB |
| `water_slave_stc8g.bin` | 二进制格式 | ~8KB |

### 烧录方法

#### 方法 1: STC-ISP (Windows)

1. 下载 STC-ISP: https://www.stcmcu.com/
2. 连接 STC8G 开发板
3. 选择 `water_slave_stc8g.hex`
4. 点击"下载/编程"

#### 方法 2: stcgal (跨平台)

```bash
# 安装
pip install stcgal

# 烧录
stcgal -P stc8g -p COM3 -t 115200 water_slave_stc8g.hex
```

#### 方法 3: 离线批量生产

使用 `water_slave_stc8g.bin` 配合离线编程器。

---

## 常见问题

### Q1: Runner 一直显示 Offline？

**检查:**
1. Runner 服务是否运行
2. 网络连接是否正常
3. GitHub Token 是否过期

**解决:**
```powershell
# 重启 Runner 服务
cd C:\actions-runner
.\svc.sh stop
.\svc.sh start
```

### Q2: 编译失败，报错 "unknown control character"？

**原因:** 文件编码问题（UTF-8 BOM）

**解决:**
1. 使用 VS Code 打开源文件
2. 右下角选择 "Save with Encoding"
3. 选择 "UTF-8" (无 BOM)
4. 重新提交

### Q3: 链接错误 "undefined symbol"？

**原因:** 源文件未全部编译或函数声明问题

**解决:**
```powershell
# 确保所有源文件都编译
$files = @("main.c", "pan3031.c", "sc09b.c")
foreach ($f in $files) {
  Write-Host "检查：$f"
  if (!(Test-Path "build/$f.rel")) {
    Write-Error "缺少：$f"
  }
}
```

---

## 性能优化建议

### 1. 使用缓存

```yaml
- name: 缓存 SDCC 工具链
  uses: actions/cache@v3
  with:
    path: C:\ProgramData\chocolatey\lib\sdcc
    key: ${{ runner.os }}-sdcc-v4.2.0
```

### 2. 并行编译

```powershell
# 使用后台任务并行编译
Start-Job -ScriptBlock { sdcc -mmcs51 -c src/main.c -o build/main.rel }
Start-Job -ScriptBlock { sdcc -mmcs51 -c src/pan3031.c -o build/pan3031.rel }
Start-Job -ScriptBlock { sdcc -mmcs51 -c src/sc09b.c -o build/sc09b.rel }
Wait-Job *
```

### 3. 减少检出文件

```yaml
- name: 部分检出
  uses: actions/checkout@v4
  with:
    sparse-checkout: |
      slave_node_stc8g
      esp8266_master
```

---

## 联系支持

如遇到其他问题，请：
1. 查看 GitHub Actions 日志
2. 搜索 SDCC 文档：http://sdcc.sourceforge.net/doc/
3. 联系技术支持

---

**更新日期**: 2025-12-21  
**适用版本**: Windows 10 + SDCC 4.2.0
