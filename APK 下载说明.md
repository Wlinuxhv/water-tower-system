# 📱 APK 下载和编译说明

## ⚠️ 当前状态

GitHub Actions 构建需要 gradlew 文件。由于 Gradle Wrapper 文件较大且未提交到仓库，你需要选择以下方式之一获取 APK：

---

## 方式 1: 自己编译（推荐）

### 使用 Android Studio（最简单）

1. **下载并安装 Android Studio**
   - 官网：https://developer.android.com/studio
   - 按照向导安装

2. **打开项目**
   - 启动 Android Studio
   - File → Open
   - 选择 `water_tower_system/android_app` 文件夹

3. **等待 Gradle 同步**
   - 首次打开会自动下载 Gradle 和依赖
   - 可能需要 5-10 分钟（取决于网络）

4. **编译 APK**
   - 点击菜单 Build → Build Bundle(s) / APK(s) → Build APK(s)
   - 等待编译完成（约 1-2 分钟）

5. **找到 APK**
   - 位置：`android_app/app/build/outputs/apk/debug/app-debug.apk`
   - 右键点击 "show in explorer" 可以直接打开文件夹

6. **安装到手机**
   - 通过 USB 传输 APK 到手机
   - 或使用微信/QQ 发送到手机
   - 在手机上点击 APK 安装

---

### 使用命令行

如果你已经有 Android SDK：

```bash
cd water_tower_system/android_app

# Windows
gradlew.bat assembleDebug

# Linux/Mac
./gradlew assembleDebug
```

APK 输出：`app/build/outputs/apk/debug/app-debug.apk`

---

## 方式 2: 使用 GitHub Actions 构建

### 步骤

1. **Fork 仓库**
   - 打开 https://github.com/Wlinuxhv/water-tower-system
   - 点击右上角 Fork

2. **启用 Actions**
   - 在你的 Fork 仓库中
   - 点击 Actions 标签
   - 点击 "I understand my workflows, go ahead and enable them"

3. **触发构建**
   - 点击 Android CI 工作流
   - 点击 "Run workflow"
   - 选择 master 分支
   - 点击 "Run workflow"

4. **下载 APK**
   - 等待构建完成（约 10 分钟）
   - 点击最近的构建记录
   - 在 Artifacts 部分下载 `water-tower-monitor-apk`
   - 解压后得到 APK 文件

---

## 方式 3: 从 Release 下载（如果可用）

检查 Releases 页面：
https://github.com/Wlinuxhv/water-tower-system/releases

如果有 APK 附件，可以直接下载。

---

## 📋 编译要求

### Android Studio 编译

| 要求 | 说明 |
|------|------|
| 操作系统 | Windows 10/11, macOS 10.14+, Linux |
| 内存 | 至少 8GB RAM（推荐 16GB） |
| 磁盘空间 | 至少 10GB 可用空间 |
| JDK | Android Studio 自带 JDK 17 |

### 命令行编译

| 要求 | 说明 |
|------|------|
| Android SDK | API 33 (Android 13) |
| JDK | JDK 17 |
| Gradle | 8.0+（或使用 Gradle Wrapper）|

---

## 🔧 常见问题

### Q: Gradle 同步很慢怎么办？

**A:** 配置国内镜像（以阿里云为例）

编辑 `android_app/build.gradle`：

```gradle
buildscript {
    repositories {
        maven { url 'https://maven.aliyun.com/repository/google' }
        maven { url 'https://maven.aliyun.com/repository/public' }
        google()
        mavenCentral()
    }
}

allprojects {
    repositories {
        maven { url 'https://maven.aliyun.com/repository/google' }
        maven { url 'https://maven.aliyun.com/repository/public' }
        google()
        mavenCentral()
        maven { url 'https://jitpack.io' }
    }
}
```

### Q: 编译失败怎么办？

**A:** 检查以下几点：

1. **网络问题**
   - 使用国内镜像（见上）
   - 或使用代理

2. **SDK 版本**
   - 打开 SDK Manager
   - 安装 Android 13 (API 33)
   - 安装 Build-Tools 33.0.0

3. **清理缓存**
   ```bash
   # 在 android_app 目录执行
   ./gradlew clean
   ```

4. **重启 Android Studio**
   - File → Invalidate Caches / Restart

### Q: 安装时提示"未知来源"？

**A:** 这是正常的安全提示

1. 打开手机设置
2. 搜索"未知来源"或"安装未知应用"
3. 授权你用来安装 APK 的应用（如文件管理器、微信等）
4. 重新安装 APK

### Q: 安装时提示"解析包时出现问题"？

**A:** 可能原因：

1. **Android 版本太低**
   - 本 APP 要求 Android 8.0+ (API 26)
   - 检查手机系统版本

2. **APK 文件损坏**
   - 重新下载或编译

3. **架构不兼容**
   - 本 APP 支持 armeabi-v7a, arm64-v8a, x86, x86_64
   - 大多数手机都支持

---

## 📲 安装后配置

1. **打开 APP**
   - 图标：💧 水滴图标
   - 名称：水塔监控

2. **配置服务器地址**
   - 默认：`http://192.168.4.1`（ESP8266 AP 模式）
   - 或：ESP8266 在你 WiFi 中的 IP

3. **测试连接**
   - 下拉刷新
   - 应能看到水塔列表

---

## 📞 获取帮助

- **Issues**: https://github.com/Wlinuxhv/water-tower-system/issues
- **Discussions**: https://github.com/Wlinuxhv/water-tower-system/discussions

---

**更新日期**: 2026-02-28
