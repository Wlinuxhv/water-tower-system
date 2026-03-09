# Windows Self-Hosted Runner 环境检测

## 检测脚本

在 Windows Runner 上运行以下 PowerShell 脚本检测环境：

```powershell
Write-Host "=== Windows Runner 环境检测 ===" -ForegroundColor Cyan
Write-Host ""

# 1. 检查 SDCC
Write-Host "1. 检查 SDCC 编译器" -ForegroundColor Yellow
$sdccPath = "C:\Program Files\SDCC"
$sdccBin = "$sdccPath\bin"
$sdccExe = "$sdccBin\sdcc.exe"

if (Test-Path $sdccPath) {
    Write-Host "   ✅ SDCC 目录存在：$sdccPath" -ForegroundColor Green
    
    if (Test-Path $sdccExe) {
        Write-Host "   ✅ sdcc.exe 存在" -ForegroundColor Green
        $version = & $sdccExe -v 2>&1 | Select-String "SDCC" | Select-Object -First 1
        Write-Host "   版本：$version" -ForegroundColor Gray
        
        # 添加到 PATH 测试
        $env:Path = "$sdccBin;$env:Path"
        try {
            $testOutput = & sdcc -v 2>&1
            Write-Host "   ✅ SDCC 可执行" -ForegroundColor Green
        } catch {
            Write-Host "   ❌ SDCC 执行失败" -ForegroundColor Red
        }
    } else {
        Write-Host "   ❌ sdcc.exe 不存在" -ForegroundColor Red
    }
} else {
    Write-Host "   ❌ SDCC 目录不存在" -ForegroundColor Red
    Write-Host "   预期路径：$sdccPath" -ForegroundColor Gray
}

Write-Host ""

# 2. 检查其他工具
Write-Host "2. 检查其他工具" -ForegroundColor Yellow

# objcopy
if (Get-Command objcopy -ErrorAction SilentlyContinue) {
    Write-Host "   ✅ objcopy 已安装" -ForegroundColor Green
    objcopy --version | Select-Object -First 1 | ForEach-Object { Write-Host "   $_" -ForegroundColor Gray }
} else {
    Write-Host "   ⚠️ objcopy 未安装 (可选，SDCC 自带 packihx)" -ForegroundColor DarkYellow
}

# packihx
$packihxExe = "$sdccBin\packihx.exe"
if (Test-Path $packihxExe) {
    Write-Host "   ✅ packihx 已安装 (SDCC 自带)" -ForegroundColor Green
} else {
    Write-Host "   ⚠️ packihx 未找到" -ForegroundColor DarkYellow
}

# Git
if (Get-Command git -ErrorAction SilentlyContinue) {
    Write-Host "   ✅ Git 已安装" -ForegroundColor Green
    git --version | ForEach-Object { Write-Host "   $_" -ForegroundColor Gray }
} else {
    Write-Host "   ❌ Git 未安装" -ForegroundColor Red
}

Write-Host ""

# 3. 检查源文件
Write-Host "3. 检查项目源文件" -ForegroundColor Yellow

$projectRoot = "C:\actions-runner\_work\water-tower-system\water-tower-system"
if (Test-Path $projectRoot) {
    Write-Host "   ✅ 项目目录存在" -ForegroundColor Green
    
    $sourceFiles = @(
        "slave_node_stc8g/src/main.c",
        "slave_node_stc8g/src/pan3031.c",
        "slave_node_stc8g/src/sc09b.c"
    )
    
    foreach ($file in $sourceFiles) {
        $fullPath = Join-Path $projectRoot $file
        if (Test-Path $fullPath) {
            $size = (Get-Item $fullPath).Length
            Write-Host "   ✅ $file ($size 字节)" -ForegroundColor Green
        } else {
            Write-Host "   ❌ $file (不存在)" -ForegroundColor Red
        }
    }
} else {
    Write-Host "   ⚠️ 项目目录不存在 (将在 checkout 后创建)" -ForegroundColor DarkYellow
    Write-Host "   预期路径：$projectRoot" -ForegroundColor Gray
}

Write-Host ""

# 4. 系统信息
Write-Host "4. 系统信息" -ForegroundColor Yellow
Write-Host "   OS: Windows 10" -ForegroundColor Gray
Write-Host "   Arch: $env:PROCESSOR_ARCHITECTURE" -ForegroundColor Gray
Write-Host "   CPUs: $env:NUMBER_OF_PROCESSORS" -ForegroundColor Gray
$mem = [math]::Round((Get-CimInstance Win32_OperatingSystem).TotalVisibleMemorySize/1MB, 2)
Write-Host "   Memory: $mem GB" -ForegroundColor Gray

Write-Host ""
Write-Host "=== 检测完成 ===" -ForegroundColor Cyan
```

---

## 手动测试编译

在 Runner 上手动运行一次完整编译测试：

```powershell
# 设置工作目录
Set-Location "C:\actions-runner\_work\water-tower-system\water-tower-system"

# 添加 SDCC 到 PATH
$sdccBin = "C:\Program Files\SDCC\bin"
$env:Path = "$sdccBin;$env:Path"

Write-Host "=== 手动测试编译 ===" -ForegroundColor Cyan

# 创建输出目录
$buildDir = "slave_node_stc8g/build"
$outDir = "slave_node_stc8g/output"

if (Test-Path $buildDir) { Remove-Item -Recurse -Force $buildDir }
if (Test-Path $outDir) { Remove-Item -Recurse -Force $outDir }
New-Item -ItemType Directory -Path $buildDir, $outDir | Out-Null

# 编译参数
$cflags = @(
    "-mmcs51",
    "--model-small",
    "--opt-code-size",
    "--no-xinit-opt",
    "--std-sdcc99"
)

# 编译源文件
Write-Host "`n编译源文件..." -ForegroundColor Yellow

$files = @("main.c", "pan3031.c", "sc09b.c")
$objects = @()

foreach ($file in $files) {
    Write-Host "  编译：$file" -NoNewline
    $obj = "$buildDir/$($file -replace '\.c$', '.rel')"
    
    $args = $cflags + @("-c", "slave_node_stc8g/src/$file", "-o", $obj)
    $output = & sdcc $args 2>&1
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host " ✅" -ForegroundColor Green
        $objects += $obj
    } else {
        Write-Host " ❌" -ForegroundColor Red
        Write-Host $output
        break
    }
}

# 链接
if ($objects.Count -eq 3) {
    Write-Host "`n链接..." -ForegroundColor Yellow
    $ihx = "$outDir/water_slave_stc8g.ihx"
    
    $linkArgs = $cflags + $objects + @("-o", $ihx)
    $output = & sdcc $linkArgs 2>&1
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  ✅ 链接成功" -ForegroundColor Green
    } else {
        Write-Host "  ❌ 链接失败" -ForegroundColor Red
        Write-Host $output
    }
    
    # 生成 HEX
    Write-Host "`n生成 HEX 文件..." -ForegroundColor Yellow
    $hex = "$outDir/water_slave_stc8g.hex"
    
    & "$sdccBin\packihx.exe" $ihx > $hex
    
    if (Test-Path $hex) {
        $size = (Get-Item $hex).Length
        Write-Host "  ✅ HEX 生成成功 ($size 字节)" -ForegroundColor Green
        
        # 验证大小
        if ($size -le 8192) {
            $percent = [math]::Round(($size / 8192) * 100, 2)
            Write-Host "  ✅ 大小验证通过 ($percent% / 8KB)" -ForegroundColor Green
        } else {
            Write-Host "  ⚠️ 代码超过 8KB!" -ForegroundColor DarkYellow
        }
    }
    
    # 显示输出文件
    Write-Host "`n输出文件:" -ForegroundColor Yellow
    Get-ChildItem $outDir | Format-Table Name, Length, LastWriteTime -AutoSize
}

Write-Host "`n=== 测试完成 ===" -ForegroundColor Cyan
```

---

## 预期输出

### 成功检测示例

```
=== Windows Runner 环境检测 ===

1. 检查 SDCC 编译器
   ✅ SDCC 目录存在：C:\Program Files\SDCC
   ✅ sdcc.exe 存在
   版本：SDCC : mcs51 4.2.0 #13666 (MINGW64)
   ✅ SDCC 可执行

2. 检查其他工具
   ⚠️ objcopy 未安装 (可选，SDCC 自带 packihx)
   ✅ packihx 已安装 (SDCC 自带)
   ✅ Git 已安装
   版本：git version 2.40.0.windows.1

3. 检查项目源文件
   ✅ 项目目录存在
   ✅ slave_node_stc8g/src/main.c (5443 字节)
   ✅ slave_node_stc8g/src/pan3031.c (3623 字节)
   ✅ slave_node_stc8g/src/sc09b.c (5082 字节)

4. 系统信息
   OS: Windows 10
   Arch: AMD64
   CPUs: 4
   Memory: 8.00 GB

=== 检测完成 ===
```

### 手动编译测试输出

```
=== 手动测试编译 ===

编译源文件...
  编译：main.c ✅
  编译：pan3031.c ✅
  编译：sc09b.c ✅

链接...
  ✅ 链接成功

生成 HEX 文件...
  ✅ HEX 生成成功 (7845 字节)
  ✅ 大小验证通过 (95.88% / 8KB)

输出文件:
Name                        Length LastWriteTime
----                        ------ -------------
water_slave_stc8g.hex       7845 2025-12-21 10:30:00
water_slave_stc8g.ihx       7845 2025-12-21 10:30:00

=== 测试完成 ===
```

---

## 触发 GitHub Actions 测试

### 方法 1: GitHub CLI

```bash
# 触发 workflow
gh workflow run embedded-ci.yml

# 查看运行状态
gh run list
gh run watch
```

### 方法 2: GitHub 网页

1. 访问：https://github.com/Wlinuxhv/water-tower-system/actions
2. 选择 "Embedded CI - STC8G Windows Build"
3. 点击 "Run workflow"
4. 选择 master 分支
5. 点击 "Run workflow"

### 方法 3: 推送代码触发

```bash
git commit --allow-empty -m "test: 触发 STC8G 构建测试"
git push
```

---

## 查看构建日志

构建开始后，访问：
https://github.com/Wlinuxhv/water-tower-system/actions

点击最新的运行记录，查看：
1. **check-windows-runner** - Runner 环境检测
2. **stc8g-windows** - STC8G 编译过程
3. **build-summary** - 构建总结

---

## 常见问题

### Q1: Runner 显示 Offline？

**检查:**
```powershell
# 查看 Runner 服务
Get-Service -Name "actions.runner.*"

# 重启服务
cd C:\actions-runner
.\svc.sh stop
.\svc.sh start
```

### Q2: SDCC 路径不对？

**修改 workflow 中的路径:**
```powershell
# 在 check-windows-runner job 中修改
$sdccPath = "C:\Program Files\SDCC"  # 改为实际路径
```

### Q3: 编译报错 "unknown control character"？

**原因:** 文件编码问题

**解决:**
1. 用 VS Code 打开源文件
2. 右下角选择 "Save with Encoding"
3. 选择 "UTF-8" (无 BOM)
4. 重新提交

### Q4: 代码大小超限？

**优化编译选项:**
```powershell
$cflags = @(
    "-mmcs51",
    "--model-small",
    "--opt-code-size",  # 优化代码大小
    "--no-xinit-opt",
    "--std-sdcc99"
)
```

---

## 联系支持

如遇到其他问题：
1. 查看 GitHub Actions 完整日志
2. 检查 SDCC 文档：http://sdcc.sourceforge.net/doc/
3. 查看 Runner 日志：`C:\actions-runner\_diag\`

---

**更新日期**: 2025-12-21  
**适用环境**: Windows 10 + SDCC 4.2.0 @ C:\Program Files\SDCC
