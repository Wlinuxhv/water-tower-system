# 📱 Android APP 构建指南

## 方法 1: 使用 Android Studio（推荐，最简单）

### 步骤 1: 安装 Android Studio

1. 下载地址：https://developer.android.com/studio
2. 安装后首次启动会自动下载 SDK 和必要组件

### 步骤 2: 打开项目

1. 启动 Android Studio
2. **File** → **Open**
3. 选择 `android_app` 文件夹
4. 等待 Gradle 同步完成（首次可能需要几分钟）

### 步骤 3: 构建 APK

**方式 A - 直接运行到手机：**
1. 用 USB 连接 Android 手机（需开启 USB 调试）
2. 点击绿色运行按钮 ▶️
3. APP 会自动安装到手机

**方式 B - 生成 APK 文件：**
1. **Build** → **Build Bundle(s) / APK(s)** → **Build APK(s)**
2. 完成后点击 **locate** 找到 APK 文件
3. APK 位置：`app/build/outputs/apk/debug/app-debug.apk`

---

## 方法 2: 使用命令行（需要 Java + Gradle）

### 前置要求

1. **安装 JDK 17**
   - 下载：https://www.oracle.com/java/technologies/downloads/#jdk17-windows
   - 安装后设置环境变量 `JAVA_HOME`

2. **验证安装**
   ```bash
   java -version
   # 应显示 Java 17
   ```

### 构建步骤

```bash
cd C:\Users\User\.openclaw\workspace\water_tower_system\android_app

# 首次运行会自动下载 Gradle
gradlew.bat assembleDebug

# 生成的 APK 位置：
# app/build/outputs/apk/debug/app-debug.apk
```

---

## 方法 3: 在线构建（无需安装）

如果不想安装 Android Studio，可以使用在线构建服务：

### 使用 GitHub Actions（如果已上传到 GitHub）

1. 推送代码到 GitHub
2. 在 Actions 标签页触发 Android CI
3. 下载生成的 APK 文件

### 使用 Codemagic

1. 访问：https://codemagic.io/
2. 连接 GitHub 仓库
3. 自动构建并下载 APK

---

## 📲 安装 APK 到手机

### 方式 1: USB 传输

1. 将 `app-debug.apk` 复制到手机
2. 在手机上打开文件管理器
3. 点击 APK 文件安装
4. 允许"未知来源"安装

### 方式 2: ADB 安装

```bash
adb install app/build/outputs/apk/debug/app-debug.apk
```

### 方式 3: 二维码下载

1. 将 APK 上传到云存储（百度网盘、Google Drive 等）
2. 生成下载链接二维码
3. 手机扫码下载安装

---

## ⚙️ 首次使用配置

1. 打开 APP
2. 进入 **设置**
3. 配置服务器地址：
   - AP 模式：`http://192.168.4.1`
   - 路由模式：`http://ESP8266 的 IP 地址`
4. 点击 **测试连接** 验证
5. 返回主界面下拉刷新

---

## 🐛 常见问题

### 问题：Gradle 同步失败

**解决：**
- 检查网络连接（需要访问 Google 仓库）
- 在 `build.gradle` 中添加国内镜像源

### 问题：SDK 版本不匹配

**解决：**
- Android Studio 会自动提示安装缺失的 SDK
- 按照提示安装 SDK 34 即可

### 问题：构建速度慢

**解决：**
- 使用国内镜像源（阿里云、腾讯云）
- 增加 Gradle 内存：`org.gradle.jvmargs=-Xmx2048m`

---

## 📦 APK 文件大小

- Debug 版本：约 8-12 MB
- Release 版本：约 5-8 MB（经过压缩和优化）

---

**构建完成后，带着 APP 去找客户验证吧！** 🚀
