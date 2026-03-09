#!/usr/bin/env python3
"""
手动触发 Gitee Go 构建
"""
import requests
import json

TOKEN = "146a77e72bc5060e569d4e2f3750a2b2"
OWNER = "wlinuxhvxx"
REPO = "water-tower-task"
BRANCH = "master"

print("=" * 60)
print("🚀 尝试触发 Gitee Go 构建")
print("=" * 60)

# 方法 1: 通过创建 Pipeline
print("\n📌 方法 1: 尝试创建 Pipeline...")
url = f"https://gitee.com/api/v5/repos/{OWNER}/{REPO}/pipelines"
headers = {"Authorization": f"token {TOKEN}"}
data = {"ref": BRANCH}

try:
    response = requests.post(url, json=data, headers=headers)
    print(f"状态码：{response.status_code}")
    if response.status_code in [200, 201]:
        print("✅ 构建已触发！")
        print(json.dumps(response.json(), indent=2, ensure_ascii=False))
    else:
        print(f"❌ 响应：{response.text[:200]}")
except Exception as e:
    print(f"❌ 错误：{e}")

# 方法 2: 检查 Runner 状态
print("\n📌 方法 2: 检查 Runner 状态...")
url = f"https://gitee.com/api/v5/repos/{OWNER}/{REPO}/runners"
try:
    response = requests.get(url, headers=headers)
    print(f"状态码：{response.status_code}")
    if response.status_code == 200:
        runners = response.json()
        if runners:
            print(f"✅ 找到 {len(runners)} 个 Runner:")
            for r in runners:
                status = "🟢 在线" if r.get('status', '') == 'online' else "🔴 离线"
                print(f"  - {r.get('name', 'Unknown')}: {status}")
                print(f"    ID: {r.get('id')}")
        else:
            print("⚠️  未找到 Runner")
    else:
        print(f"❌ 响应：{response.text[:200]}")
except Exception as e:
    print(f"❌ 错误：{e}")

# 方法 3: 检查仓库 Webhooks
print("\n📌 方法 3: 检查 Webhooks...")
url = f"https://gitee.com/api/v5/repos/{OWNER}/{REPO}/hooks"
try:
    response = requests.get(url, headers=headers)
    print(f"状态码：{response.status_code}")
    if response.status_code == 200:
        hooks = response.json()
        if hooks:
            print(f"✅ 找到 {len(hooks)} 个 Webhook")
        else:
            print("⚠️  未找到 Webhook (可能需要手动触发)")
    else:
        print(f"❌ 响应：{response.text[:200]}")
except Exception as e:
    print(f"❌ 错误：{e}")

print("\n" + "=" * 60)
print("💡 如果 API 触发失败，请在 Gitee App 中手动操作:")
print("  1. 打开仓库 → 服务 → Gitee Go → 流水线")
print("  2. 点击 '运行' 按钮")
print("  3. 选择 master 分支")
print("  4. 确认运行")
print("=" * 60)
