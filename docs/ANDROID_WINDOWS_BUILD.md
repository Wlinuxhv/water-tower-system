# Android APK Windows 构建指南

## 配置说明

已创建新的 GitHub Actions workflow：`android-ci-windows.yml`

**特点**:
- ✅ 使用 Windows 10 self-hosted runner
- ✅ 自动检测 Java/Android SDK/Gradle
- ✅ 详细的环境检查和日志
- ✅ 自动上传 APK 产物 (保留 30 天)

---

## 触发构建

### 方法 1: GitHub CLI (推荐)

```bash
# 触发 Android 构建
gh workflow run android-ci-windows.yml

# 实时监控构建日志
gh run watch

# 查看运行列表
gh run list

# 查看最新运行的详细信息
gh run view --log
```

### 方法 2: GitHub 网页

1. 访问：https://github.com/Wlinuxhv/water-tower-system/actions
2. 选择 **"Android CI - Windows Build"**
3. 点击 **"Run workflow"** 按钮
4. 选择分支：**master**
5. 点击 **"Run workflow"**

### 方法 3: 推送代码触发

```bash
# 修改 android_app 目录下的任意文件
git commit --allow-empty -m "test: 触发 Android 构建测试"
git push
```

---

## 预期输出

### 1. Runner 环境检测

```
=== Runner 信息 ===
Runner OS: Windows
Runner Name: windows-runner
Runner Arch: X64
Number of CPUs: 4
Total Memory: 8.00 GB
OS Name: Windows 10 Pro
OS Version: 10.0.19045

=== 检查 Java 环境 ===
✅ JAVA_HOME: C:\Program Files\Java\jdk-17
Java 版本：version "17.0.9" 2023-10-17
✅ java 命令可用
version "17.0.9" 2023-10-17 LTS

=== 检查 Android SDK ===
✅ Android SDK: C:\Android\android-sdk
已安装平台：android-33, android-34
已安装 Build Tools: 33.0.2, 34.0.0
✅ adb 可用
Android Debug Bridge version 1.0.41

=== 检查 Gradle ===
✅ Gradle 已安装
Gradle 8.2
✅ Gradle Wrapper 存在

=== 检查项目结构 ===
✅ android_app/build.gradle
✅ android_app/app/build.gradle
✅ android_app/settings.gradle
✅ android_app/gradlew.bat
✅ android_app/gradle/wrapper/gradle-wrapper.properties

=== app/build.gradle 配置 ===
    compileSdk 34
    targetSdk 34
    minSdk 24
```

### 2. APK 编译过程

```
=== 开始编译 Debug APK ===
Gradle 版本:
Gradle 8.2

项目配置:
  compileSdk 34
  targetSdk 34
  minSdk 24

编译 APK...
> Task :app:preBuild UP-TO-DATE
> Task :app:preDebugBuild UP-TO-DATE
> Task :app:mergeDebugShaders
> Task :app:compileDebugShaders NO-SOURCE
> Task :app:generateDebugAssets UP-TO-DATE
> Task :app:mergeDebugAssets
> Task :app:processDebugResources
> Task :app:compileDebugKotlin
> Task :app:compileDebugJavaWithJavac
> Task :app:processDebugJavaRes
> Task :app:mergeDebugJavaResource
> Task :app:dexBuilderDebug
> Task :app:mergeDebugGlobalSynthetics
> Task :app:mergeExtDexDebug
> Task :app:mergeDebugNativeLibs NO-SOURCE
> Task :app:mergeLibDexDebug
> Task :app:mergeProjectDexDebug
> Task :app:packageDebug
> Task :app:createDebugApkListingFileRedirect
> Task :app:assembleDebug

BUILD SUCCESSFUL in 1m 23s

✅ 编译成功!

=== 查找 APK 文件 ===
✅ 找到 APK: D:\a\water-tower-system\water-tower-system\android_app\app\build\outputs\apk\debug\app-debug.apk
文件大小：27.27 MB
最后修改：2025-12-21 11:30:45
已复制 APK 到：app\build\outputs\apk\debug\water-tower-monitor-debug.apk

=== 构建信息 ===
APK 路径：D:\a\water-tower-system\water-tower-system\android_app\app\build\outputs\apk\debug\water-tower-monitor-debug.apk
APK 名称：water-tower-monitor-debug.apk
APK 大小：27.27 MB
Runner: windows-runner
构建时间：2025-12-21 11:30:45

✅ 上传成功：water-tower-monitor-debug-windows
```

### 3. 构建总结

```
## 📱 Android APK 构建总结

### Runner 信息
- 名称：windows-runner
- OS: Windows 10
- Java: version "17.0.9" 2023-10-17 LTS
- Android SDK: C:\Android\android-sdk

### 构建状态
| 项目 | 状态 | 产物 |
|------|------|------|
| Android APK | ✅ 成功 | water-tower-monitor-debug.apk |

### 下载 APK
- [water-tower-monitor-debug-windows](../../actions/runs/xxx/artifacts)
```

---

## 下载 APK

### 方法 1: GitHub 网页

1. 访问：https://github.com/Wlinuxhv/water-tower-system/actions
2. 点击最新的运行记录
3. 滚动到底部 **"Artifacts"**
4. 点击 **`water-tower-monitor-debug-windows`**
5. 下载 APK 文件 (约 27MB)

### 方法 2: GitHub CLI

```bash
# 列出所有运行
gh run list

# 下载最新运行的 artifact
gh run download --name water-tower-monitor-debug-windows

# 或下载指定运行的 artifact
gh run download <run-id> --name water-tower-monitor-debug-windows
```

### 方法 3: 直接链接

构建成功后，artifact 保留 30 天，可通过以下链接下载：
```
https://github.com/Wlinuxhv/water-tower-system/actions/runs/{RUN_ID}/artifacts/{ARTIFACT_ID}
```

---

## 环境要求

### Windows Runner 必需软件

#### 1. Java JDK 17

**安装**:
```powershell
# 使用 Chocolatey
choco install openjdk17 -y

# 或下载：https://adoptium.net/
```

**验证**:
```powershell
java -version
# 预期输出：version "17.0.x"
```

#### 2. Android SDK

**安装**:
1. 下载 Android Command Line Tools: https://developer.android.com/studio#command-tools
2. 解压到 `C:\Android\android-sdk`
3. 安装必需组件：
```powershell
cd C:\Android\android-sdk\cmdline-tools\latest\bin

# 接受许可证
sdkmanager --licenses

# 安装平台工具
sdkmanager "platform-tools" "platforms;android-34" "build-tools;34.0.0"
```

**验证**:
```powershell
adb --version
sdkmanager --version
```

#### 3. Gradle (可选，项目使用 Gradle Wrapper)

**安装**:
```powershell
choco install gradle -y
```

---

## 故障排查

### 问题 1: JAVA_HOME 未设置

**症状**: `⚠️ JAVA_HOME 未设置`

**解决**:
```powershell
# 设置 JAVA_HOME
$javaHome = "C:\Program Files\Java\jdk-17"
[System.Environment]::SetEnvironmentVariable("JAVA_HOME", $javaHome, "Machine")

# 刷新当前会话
$env:JAVA_HOME = $javaHome
```

### 问题 2: Android SDK 未找到

**症状**: `⚠️ 未找到 Android SDK`

**解决**:
```powershell
# 设置 ANDROID_HOME
$sdkPath = "C:\Android\android-sdk"
[System.Environment]::SetEnvironmentVariable("ANDROID_HOME", $sdkPath, "Machine")
[System.Environment]::SetEnvironmentVariable("ANDROID_SDK_ROOT", $sdkPath, "Machine")

# 添加到 PATH
$env:Path = "$sdkPath\platform-tools;$sdkPath\tools;$env:Path"
```

### 问题 3: 编译失败 - SDK 未安装

**症状**: `SDK location not found`

**解决**:
1. 创建 `local.properties` 文件：
```properties
sdk.dir=C\:\\Android\\android-sdk
```

2. 或在 workflow 中自动设置（已包含）

### 问题 4: Gradle 下载超时

**症状**: `Download failed` 或超时

**解决**:
- 使用国内镜像（已在 build.gradle 配置）
- 增加超时时间
- 检查网络连接

### 问题 5: 内存不足

**症状**: `OutOfMemoryError` 或 `GC overhead limit exceeded`

**解决**:
在 `gradle.properties` 中增加内存：
```properties
org.gradle.jvmargs=-Xmx2048m -XX:MaxPermSize=512m
```

---

## 手动测试 (在 Runner 上)

在 Windows Runner 上手动测试编译：

```powershell
# 1. 设置环境变量
$env:JAVA_HOME = "C:\Program Files\Java\jdk-17"
$env:ANDROID_HOME = "C:\Android\android-sdk"
$env:Path = "$env:ANDROID_HOME\platform-tools;$env:ANDROID_HOME\tools;$env:Path"

# 2. 进入项目目录
cd C:\actions-runner\_work\water-tower-system\water-tower-system\android_app

# 3. 清理
.\gradlew.bat clean

# 4. 编译 Debug APK
.\gradlew.bat assembleDebug --no-daemon

# 5. 查找 APK
Get-ChildItem -Recurse -Filter "*.apk" -Path "app\build\outputs\apk\debug" |
  Format-Table Name, Length, LastWriteTime -AutoSize

Write-Host "✅ 编译完成!"
```

---

## 构建产物

### APK 信息

| 属性 | 值 |
|------|-----|
| 文件名 | water-tower-monitor-debug.apk |
| 大小 | ~27 MB |
| 路径 | android_app/app/build/outputs/apk/debug/ |
| 保留时间 | 30 天 |

### 兼容性

- **Android 版本**: 7.0+ (API 24)
- **目标版本**: Android 14 (API 34)
- **架构**: arm64-v8a, armeabi-v7a, x86, x86_64

---

## 查看构建日志

### GitHub 网页

1. 访问：https://github.com/Wlinuxhv/water-tower-system/actions
2. 点击运行记录
3. 查看各步骤日志：
   - ✅ check-windows-runner
   - ✅ build-apk-windows
   - ✅ build-summary

### GitHub CLI

```bash
# 查看最新运行日志
gh run view --log

# 查看特定 job 日志
gh run view <run-id> --job <job-id> --log

# 实时查看日志
gh run watch
```

---

## 自动化建议

### 1. 定时构建

在 workflow 中添加：
```yaml
schedule:
  - cron: '0 2 * * *'  # 每天 2:00 UTC
```

### 2. 发布 Release 时自动构建

```yaml
on:
  release:
    types: [published]
```

### 3. 自动签名

配置签名密钥后，自动构建 Release 版本。

---

## 联系支持

遇到问题？
1. 查看完整日志：GitHub Actions
2. 检查环境：运行检测脚本
3. 查看文档：`docs/WINDOWS_RUNNER_CHECK.md`

---

**更新日期**: 2025-12-21  
**适用环境**: Windows 10 + JDK 17 + Android SDK
