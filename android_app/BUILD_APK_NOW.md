# 🚀 首次启动 Android Studio 并构建 APK

## ✅ 环境已就绪

- ✅ Android Studio 已安装
- ✅ Java (JBR 21) 已安装
- ⚠️ Android SDK 需要首次配置

---

## 📋 步骤 1: 首次启动 Android Studio（必须！）

**Android Studio 需要首次运行来下载 SDK 组件**

### 操作：

1. **打开 Android Studio**
   - 双击桌面快捷方式
   - 或运行：`C:\Program Files\Android\Android Studio\bin\studio64.exe`

2. **首次启动向导**
   - 选择 **Do not import settings** → **OK**
   - **Next** → **Standard** → **Next**
   - 选择主题 → **Next**
   - **Finish**（会自动下载 SDK，约 5-10 分钟）

3. **等待 SDK 下载完成**
   - 底部会显示下载进度
   - 完成后点击 **Finish**

---

## 📋 步骤 2: 打开项目

1. **File** → **Open**
2. 选择文件夹：
   ```
   C:\Users\User\.openclaw\workspace\water_tower_system\android_app
   ```
3. 点击 **OK**

---

## 📋 步骤 3: 构建 APK

1. 等待 Gradle 同步完成（底部状态栏显示）
2. **Build** → **Build Bundle(s) / APK(s)** → **Build APK(s)**
3. 等待 1-2 分钟
4. 弹出通知后点击 **locate**

---

## 📦 APK 位置

```
C:\Users\User\.openclaw\workspace\water_tower_system\android_app\app\build\outputs\apk\debug\app-debug.apk
```

---

## ⏱️ 预计时间

| 步骤 | 时间 |
|------|------|
| 首次启动 + SDK 下载 | 5-10 分钟 |
| 打开项目 + Gradle 同步 | 3-5 分钟 |
| 构建 APK | 1-2 分钟 |
| **总计** | **约 10-20 分钟** |

---

## ✅ 完成后

APK 文件约 10-15 MB，可以安装到手机测试了！

**现在去打开 Android Studio 吧！** 🚀
