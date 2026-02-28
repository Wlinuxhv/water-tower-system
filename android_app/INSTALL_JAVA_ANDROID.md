# 📋 Java 和 Android 环境安装指南

## 当前状态检查结果

❌ **Java/JDK**: 未安装  
❌ **Android SDK**: 未配置  
❌ **环境变量**: ANDROID_HOME 和 ANDROID_SDK_ROOT 未设置

---

## 🎯 推荐方案：安装 Android Studio（最简单）

Android Studio 会自动安装 JDK、Android SDK 和所有必要组件，**一站式解决**。

### 步骤 1: 下载 Android Studio

**官方下载地址**: https://developer.android.com/studio

- 点击 **Download Android Studio**
- 接受协议 → 下载 Windows 安装包（约 1.1 GB）

### 步骤 2: 安装 Android Studio

1. 双击下载的安装文件（`android-studio-2024.x.x.xx-windows.exe`）
2. 按照安装向导操作：
   - 选择安装路径（建议默认 `C:\Program Files\Android\Android Studio`）
   - 勾选 **Android Virtual Device**（可选，模拟器用）
3. 完成安装

### 步骤 3: 首次启动配置

1. 启动 Android Studio
2. 选择 **Standard** 安装类型
3. 选择 UI 主题（Dark/Light）
4. 点击 **Finish**，等待下载 SDK 组件（约 5-10 分钟，需要网络）

### 步骤 4: 验证安装

打开 PowerShell 或 CMD，运行：

```powershell
# 验证 Java
java -version
# 应显示：java version "17.x.x"

# 验证 Android SDK
echo $env:ANDROID_HOME
# 应显示：C:\Users\你的用户名\AppData\Local\Android\Sdk
```

---

## 🔧 备选方案：手动安装 JDK + Gradle

如果不想安装 Android Studio（不推荐，还需要单独配置 SDK），可以手动安装：

### 步骤 1: 安装 JDK 17

**下载地址**: https://www.oracle.com/java/technologies/downloads/#jdk17-windows

1. 选择 **Windows x64 Installer**（.msi 文件）
2. 下载并安装（需要 Oracle 账号，免费）
3. 或者使用 OpenJDK（无需账号）：
   - 下载地址：https://adoptium.net/temurin/releases/?version=17
   - 选择 **Windows x64 MSI**

### 步骤 2: 配置环境变量

1. 右键 **此电脑** → **属性** → **高级系统设置**
2. 点击 **环境变量**
3. 在 **系统变量** 中添加：
   - 变量名：`JAVA_HOME`
   - 变量值：`C:\Program Files\Java\jdk-17`（根据实际安装路径）

4. 编辑 **Path** 变量，添加：
   - `%JAVA_HOME%\bin`

### 步骤 3: 验证 Java 安装

```powershell
java -version
javac -version
```

应显示 Java 17 版本信息。

---

## 📱 使用 Android Studio 构建 APK

### 1. 打开项目

1. 启动 Android Studio
2. **File** → **Open**
3. 选择 `C:\Users\User\.openclaw\workspace\water_tower_system\android_app`
4. 点击 **OK**

### 2. 等待 Gradle 同步

- 首次打开会自动下载 Gradle 和依赖（约 3-5 分钟）
- 底部状态栏显示同步进度
- 完成后显示 **Sync Now** → 点击完成

### 3. 构建 APK

**方式 A - 图形界面：**
1. 菜单栏 **Build** → **Build Bundle(s) / APK(s)** → **Build APK(s)**
2. 等待构建完成（约 1-2 分钟）
3. 弹出通知 **APK(s) generated successfully** → 点击 **locate**

**方式 B - 命令行：**
```powershell
cd C:\Users\User\.openclaw\workspace\water_tower_system\android_app
.\gradlew.bat assembleDebug
```

### 4. APK 文件位置

```
C:\Users\User\.openclaw\workspace\water_tower_system\android_app\app\build\outputs\apk\debug\app-debug.apk
```

---

## 📦 APK 文件信息

| 项目 | 预期值 |
|------|--------|
| 文件名 | `app-debug.apk` |
| 大小 | 约 10-15 MB |
| 版本 | v1.0.0 |
| 功能 | 实时监控 + 水泵控制 + **历史水位曲线图** |

---

## 📲 安装 APK 到手机

### 方式 1: USB 传输（最简单）

1. 用 USB 线连接手机和电脑
2. 将 `app-debug.apk` 复制到手机（任意文件夹）
3. 在手机上打开文件管理器
4. 找到并点击 APK 文件
5. 如果提示"未知来源"，允许安装
6. 安装完成

### 方式 2: ADB 命令行安装

1. 在手机上开启 **开发者选项** 和 **USB 调试**
2. USB 连接手机
3. 在 PowerShell 中运行：
```powershell
adb install C:\Users\User\.openclaw\workspace\water_tower_system\android_app\app\build\outputs\apk\debug\app-debug.apk
```

### 方式 3: 二维码下载

1. 将 APK 上传到云存储（百度网盘、Google Drive、OneDrive 等）
2. 生成分享链接
3. 使用二维码生成器创建下载二维码
4. 手机扫码下载安装

---

## ✅ 验证清单（见客户前必查）

- [ ] APP 能正常打开，无崩溃
- [ ] 主界面显示水塔列表
- [ ] 点击水塔能弹出水泵控制对话框
- [ ] **历史** 页面能看到日期选择器
- [ ] 选择日期后能显示水位曲线图
- [ ] 下拉刷新能更新数据
- [ ] 设置页面能配置服务器地址

---

## 🐛 常见问题解决

### 问题 1: Gradle 下载慢或失败

**原因**: 需要访问 Google 仓库，国内网络可能不稳定

**解决方案**:
1. 使用国内镜像源
2. 在 `build.gradle` 中添加：
```gradle
repositories {
    maven { url 'https://maven.aliyun.com/repository/google' }
    maven { url 'https://maven.aliyun.com/repository/public' }
    google()
    mavenCentral()
}
```

### 问题 2: SDK 版本不匹配

**解决方案**:
- Android Studio 会提示安装缺失的 SDK
- 打开 **SDK Manager** → 安装 **Android 14.0 (API 34)**

### 问题 3: 构建时内存不足

**解决方案**:
- 在 `gradle.properties` 中添加：
```properties
org.gradle.jvmargs=-Xmx2048m -XX:MaxPermSize=512m
```

### 问题 4: 找不到 Java

**解决方案**:
- 确认 `JAVA_HOME` 环境变量已设置
- 重启 PowerShell 或 CMD 使环境变量生效
- 运行 `echo $env:JAVA_HOME` 验证

---

## ⏱️ 预计时间

| 步骤 | 时间 |
|------|------|
| 下载 Android Studio | 10-30 分钟（取决于网速） |
| 安装 Android Studio | 5-10 分钟 |
| 首次启动配置 | 5-10 分钟 |
| 打开项目并同步 | 3-5 分钟 |
| 构建 APK | 1-2 分钟 |
| **总计** | **约 25-60 分钟** |

---

## 🎯 下一步

1. **安装 Android Studio**（按上述步骤）
2. **打开项目并构建 APK**
3. **将 APK 安装到手机测试**
4. **准备客户演示**

**构建完成后，APK 文件位置会在：**
```
C:\Users\User\.openclaw\workspace\water_tower_system\android_app\app\build\outputs\apk\debug\app-debug.apk
```

---

**需要帮助随时说！** 💪
