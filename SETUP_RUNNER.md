# 🚀 配置 GitHub Self-Hosted Runner

## ✅ Runner 已下载

位置：`C:\actions-runner`

---

## 📋 步骤 1: 获取 Registration Token

### 在 GitHub 上操作：

1. 打开：https://github.com/Wlinuxhv/water-tower-system/settings/actions/runners/new
2. 选择 **Windows**
3. 复制 **Run** 部分中的 token（类似 `AQN...` 的长字符串）

**或者用这个直接链接**：
```
https://github.com/Wlinuxhv/water-tower-system/settings/actions/runners/new
```

---

## 📋 步骤 2: 配置 Runner

### 在 PowerShell 中运行（管理员）：

```powershell
cd C:\actions-runner
.\config.cmd --url https://github.com/Wlinuxhv/water-tower-system --token YOUR_TOKEN_HERE --name windows-runner --runnergroup default --unattended
```

**替换 `YOUR_TOKEN_HERE` 为刚才复制的 token**

### 配置选项：
- `--unattended`: 无需交互，自动配置
- `--name windows-runner`: runner 名称
- `--runnergroup default`: 使用默认组

---

## 📋 步骤 3: 启动 Runner

```powershell
cd C:\actions-runner
.\run.cmd
```

**保持这个窗口运行**，Runner 会监听 GitHub 的构建任务。

---

## 📋 步骤 4: 修改 Workflow

更新 `.github/workflows/android-ci.yml`：

```yaml
jobs:
  build:
    runs-on: self-hosted  # 改为 self-hosted
```

---

## 📋 步骤 5: 推送并触发构建

```powershell
cd C:\Users\User\.openclaw\workspace\water_tower_system
git add .
git commit -m "Add history feature + self-hosted runner config"
git push
```

或者手动触发：
1. 打开：https://github.com/Wlinuxhv/water-tower-system/actions
2. 点击 **Android CI** workflow
3. 点击 **Run workflow**
4. 选择分支 → **Run workflow**

---

## 📦 构建完成后

1. 在 Actions 页面找到刚完成的构建
2. 点击构建记录
3. 在 **Artifacts** 部分下载 APK
4. 文件名：`water-tower-monitor-debug.zip`

---

## ⏱️ 预计时间

| 步骤 | 时间 |
|------|------|
| 配置 Runner | 2 分钟 |
| 首次构建（下载依赖） | 5-10 分钟 |
| 后续构建 | 2-3 分钟 |

---

## 🎯 快速开始

**现在做**：
1. 打开 https://github.com/Wlinuxhv/water-tower-system/settings/actions/runners/new
2. 复制 token
3. 运行配置命令
4. 启动 runner
5. 告诉我，我帮你修改 workflow 并推送

---

**需要帮忙随时说！** 💪
