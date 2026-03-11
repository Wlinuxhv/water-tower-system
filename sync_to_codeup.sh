#!/bin/bash
# 一键同步代码到 GitHub 和 CodeUp 双仓库
# 用法：./sync_to_codeup.sh [branch_name]

set -e

# 配置
GITHUB_REMOTE="origin"
CODEUP_REMOTE="codeup"
DEFAULT_BRANCH="main"
BRANCH=${1:-$DEFAULT_BRANCH}

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "============================================================"
echo "🔄 双仓库同步脚本"
echo "============================================================"
echo "GitHub:  $GITHUB_REMOTE"
echo "CodeUp:  $CODEUP_REMOTE"
echo "分支：   $BRANCH"
echo "============================================================"
echo

# 检查远程仓库是否存在
echo "📋 检查远程仓库配置..."
if ! git remote | grep -q "^${GITHUB_REMOTE}$"; then
    echo -e "${RED}❌ 错误：GitHub 远程仓库 '$GITHUB_REMOTE' 不存在${NC}"
    exit 1
fi

if ! git remote | grep -q "^${CODEUP_REMOTE}$"; then
    echo -e "${RED}❌ 错误：CodeUp 远程仓库 '$CODEUP_REMOTE' 不存在${NC}"
    exit 1
fi
echo -e "${GREEN}✅ 远程仓库配置正确${NC}"
echo

# 获取当前分支
CURRENT_BRANCH=$(git branch --show-current)
echo "📍 当前分支：$CURRENT_BRANCH"

if [ "$CURRENT_BRANCH" != "$BRANCH" ]; then
    echo -e "${YELLOW}⚠️  当前不在目标分支 '$BRANCH' 上${NC}"
    echo "是否切换到 '$BRANCH' 分支？(y/n)"
    read -r response
    if [[ "$response" =~ ^[Yy]$ ]]; then
        git checkout "$BRANCH"
    else
        echo "继续在当前分支操作..."
        BRANCH="$CURRENT_BRANCH"
    fi
fi
echo

# 拉取最新代码
echo "📥 拉取最新代码..."
git fetch --all
echo -e "${GREEN}✅ 代码已更新${NC}"
echo

# 推送到 GitHub
echo "🚀 推送到 GitHub..."
if git push "$GITHUB_REMOTE" "$BRANCH"; then
    echo -e "${GREEN}✅ GitHub 推送成功${NC}"
else
    echo -e "${RED}❌ GitHub 推送失败${NC}"
    exit 1
fi
echo

# 推送到 CodeUp
echo "🚀 推送到 CodeUp..."
echo "提示：首次推送可能需要输入阿里云账号和密码（Token）"
if git push "$CODEUP_REMOTE" "$BRANCH"; then
    echo -e "${GREEN}✅ CodeUp 推送成功${NC}"
else
    echo -e "${RED}❌ CodeUp 推送失败${NC}"
    echo "请检查："
    echo "  1. 网络连接是否正常"
    echo "  2. Token 是否正确（密码应使用 CodeUp Token）"
    echo "  3. 仓库权限是否足够"
    exit 1
fi
echo

# 显示结果
echo "============================================================"
echo "✅ 双仓库同步完成！"
echo "============================================================"
echo "GitHub: https://github.com/Wlinuxhv/water-tower-system"
echo "CodeUp: https://codeup.aliyun.com/69b0c304f7b43e00d420c450/water-tower-system.git"
echo "============================================================"
