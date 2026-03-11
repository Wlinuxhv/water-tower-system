# CodeUp 双仓库同步配置指南

## 📋 概述

本配置实现 GitHub（主仓库）和阿里云 CodeUp（镜像仓库）的双向同步。

- **GitHub**: https://github.com/Wlinuxhv/water-tower-system (主用)
- **CodeUp**: https://codeup.aliyun.com/69b0c304f7b43e00d420c450/water-tower-system.git (国内镜像)

---

## 🔐 Token 存储

CodeUp Token 已加密存储到：`/app/working/copaw_tokens.db`

- **加密方式**: Fernet AES-128-CBC + PBKDF2-SHA256 (480000 次迭代)
- **Salt**: 256-bit 随机盐
- **Token 名称**: `codeup`

---

## 📦 Git 远程仓库配置

当前已配置两个远程仓库：

```bash
cd /app/working/water-tower-system
git remote -v
```

输出：
```
codeup  https://codeup.aliyun.com/69b0c304f7b43e00d420c450/water-tower-system.git (fetch)
codeup  https://codeup.aliyun.com/69b0c304f7b43e00d420c450/water-tower-system.git (push)
origin  https://github.com/Wlinuxhv/water-tower-system.git (fetch)
origin  https://github.com/Wlinuxhv/water-tower-system.git (push)
```

---

## 🚀 使用方法

### 方法 1: 手动推送（首次使用）

```bash
cd /app/working/water-tower-system

# 推送到 GitHub
git push origin main

# 推送到 CodeUp（首次需要输入 Token）
git push codeup main
```

首次推送时会提示：
```
Username for 'https://codeup.aliyun.com': <你的阿里云账号名>
Password for 'https://<你的阿里云账号名>@codeup.aliyun.com': <CodeUp Token>
```

**Token**: `pt-ndoy9KTrNnUKsNuWES0hvUAT_38482e29-f8c9-4e3f-bba7-7657f9c0ec4d`

---

### 方法 2: 使用同步脚本（推荐）

```bash
cd /app/working/water-tower-system
./sync_to_codeup.sh
```

或指定分支：
```bash
./sync_to_codeup.sh develop
```

脚本会自动：
1. 检查远程仓库配置
2. 拉取最新代码
3. 推送到 GitHub
4. 推送到 CodeUp

---

### 方法 3: 配置自动同步（可选）

启用 Git Hook，每次推送到 GitHub 时自动同步到 CodeUp：

```bash
cd /app/working/water-tower-system
cp .git/hooks/post-push.auto-sync .git/hooks/post-push
chmod +x .git/hooks/post-push
```

启用后，执行 `git push` 会自动同步到 CodeUp。

---

## ⚠️ 常见问题

### 1. CodeUp 推送失败

**原因**: Token 错误或权限不足

**解决**:
```bash
# 清除保存的凭据
git credential-cache exit
# 或
rm -rf ~/.git-credentials

# 重新推送（会提示输入 Token）
git push codeup main
```

---

### 2. 网络连接超时

**原因**: 国内访问 CodeUp 网络问题

**解决**:
- 检查网络连接
- 使用阿里云内网（如果在阿里云 ECS）
- 稍后重试

---

### 3. 分支不存在

**原因**: CodeUp 仓库还没有该分支

**解决**:
```bash
# 强制推送创建分支
git push codeup <branch_name> -u
```

---

## 📊 同步策略

### 推荐工作流

```bash
# 1. 本地开发
git add .
git commit -m "你的提交信息"

# 2. 推送到双平台
git push origin main        # 先推 GitHub
./sync_to_codeup.sh        # 再同步 CodeUp

# 或直接使用 Git Hook 自动同步
git push origin main       # 自动同步到 CodeUp
```

---

### 分支管理

| 分支 | GitHub | CodeUp | 说明 |
|------|--------|--------|------|
| `main` | ✅ 主分支 | ✅ 镜像 | 生产环境代码 |
| `develop` | ✅ 开发分支 | ✅ 镜像 | 日常开发 |
| `feature/*` | ✅ 特性分支 | ⚠️ 按需同步 | 临时分支 |

---

## 🔍 验证同步

### 检查 GitHub

访问：https://github.com/Wlinuxhv/water-tower-system

### 检查 CodeUp

访问：https://codeup.aliyun.com/69b0c304f7b43e00d420c450/water-tower-system.git

### 查看提交历史

```bash
# GitHub 最新提交
git log origin/main --oneline -5

# CodeUp 最新提交
git log codeup/main --oneline -5

# 应该看到相同的提交记录
```

---

## 📝 注意事项

1. **GitHub 是主仓库** - 所有开发以 GitHub 为准
2. **CodeUp 是镜像** - 仅用于国内加速访问和数据备份
3. **CI/CD 仍在 GitHub** - 不迁移到云效 Flow
4. **Token 安全** - 不要将 Token 提交到代码仓库
5. **定期同步** - 确保两个仓库代码一致

---

## 🛠️ 维护命令

```bash
# 查看远程仓库
git remote -v

# 更新远程仓库 URL
git remote set-url codeup <新地址>

# 删除远程仓库
git remote remove codeup

# 重新添加远程仓库
git remote add codeup https://codeup.aliyun.com/69b0c304f7b43e00d420c450/water-tower-system.git

# 测试连接
git ls-remote codeup

# 查看所有分支
git branch -a

# 查看 CodeUp 分支
git branch -r --list 'codeup/*'
```

---

## 📞 支持

如有问题，请检查：
1. Token 是否正确
2. 网络连接是否正常
3. 仓库权限是否足够
4. Git 版本是否兼容

---

**最后更新**: 2024 年
**配置状态**: ✅ 已完成
