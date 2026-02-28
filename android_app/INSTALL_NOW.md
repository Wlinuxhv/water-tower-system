# 🚀 立即安装 Android Studio

## ✅ 下载已完成

**文件位置**: `C:\Users\User\AppData\Local\Temp\android-studio-installer.exe`  
**文件大小**: 1.1 GB

---

## 📋 安装步骤（手动）

### 1. 运行安装程序

在文件资源管理器中打开：
```
C:\Users\User\AppData\Local\Temp\android-studio-installer.exe
```

或者双击桌面上的这个文件。

### 2. 安装向导

1. **欢迎界面** → 点击 **Next**
2. **选择组件** → 保持默认（勾选 Android Studio 和 Android Virtual Device）→ **Next**
3. **选择安装位置** → 建议默认 `C:\Program Files\Android\Android Studio` → **Next**
4. **选择开始菜单文件夹** → 默认 → **Install**
5. 等待安装完成（约 5-10 分钟）
6. 勾选 **Start Android Studio** → **Finish**

### 3. 首次启动配置

1. **Import Settings** → 选择 **Do not import settings** → **OK**
2. **Welcome** → **Next**
3. **Install Type** → 选择 **Standard** → **Next**
4. **Select UI Theme** → 选择 **Dark** 或 **Light** → **Next**
5. **Verify Settings** → 点击 **Finish**
6. 等待下载 SDK 组件（约 5-10 分钟，需要网络）
7. 点击 **Finish** 完成

---

## 📱 构建 APK

### 1. 打开项目

1. 启动 Android Studio
2. **File** → **Open**
3. 选择文件夹：
   ```
   C:\Users\User\.openclaw\workspace\water_tower_system\android_app
   ```
4. 点击 **OK**

### 2. 等待 Gradle 同步

- 首次打开会自动下载 Gradle 和依赖
- 底部状态栏显示进度
- 完成后即可构建

### 3. 生成 APK

**菜单栏**: **Build** → **Build Bundle(s) / APK(s)** → **Build APK(s)**

等待 1-2 分钟，APK 生成后会弹出通知。

### 4. APK 位置

点击通知中的 **locate** 或打开文件夹：
```
C:\Users\User\.openclaw\workspace\water_tower_system\android_app\app\build\outputs\apk\debug\app-debug.apk
```

---

## 📲 安装到手机

### 方式 1: USB 传输
1. 用 USB 线连接手机
2. 将 `app-debug.apk` 复制到手机
3. 手机文件管理器打开 APK
4. 允许"未知来源"安装

### 方式 2: 直接打开
如果手机和电脑在同一 WiFi：
1. 在手机上用浏览器访问电脑共享
2. 下载 APK 并安装

---

## ⏱️ 预计时间

| 步骤 | 时间 |
|------|------|
| 安装 Android Studio | 10-15 分钟 |
| 首次启动配置 | 5-10 分钟 |
| 打开项目并同步 | 3-5 分钟 |
| 构建 APK | 1-2 分钟 |
| **总计** | **约 20-30 分钟** |

---

## ✅ 验证清单

安装完成后检查：
- [ ] Java 已安装：打开 CMD 运行 `java -version`
- [ ] Android Studio 能正常启动
- [ ] 项目能打开并同步
- [ ] APK 构建成功

---

**现在开始安装吧！安装完成后告诉我，我帮你构建 APK。** 💪
