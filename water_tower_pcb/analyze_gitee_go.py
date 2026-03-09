#!/usr/bin/env python3
"""
分析 Gitee Go 流水线问题并尝试创建
"""
import requests
import json

TOKEN = "146a77e72bc5060e569d4e2f3750a2b2"
OWNER = "wlinuxhvxx"
REPO = "water-tower-task"

headers = {"Authorization": f"token {TOKEN}"}
base_url = f"https://gitee.com/api/v5/repos/{OWNER}/{REPO}"

print("=" * 70)
print("🔍 分析 Gitee Go 流水线问题")
print("=" * 70)

# 1. 检查仓库信息
print("\n📌 1. 检查仓库信息...")
url = f"{base_url}"
response = requests.get(url, headers=headers)
if response.status_code == 200:
    repo = response.json()
    print(f"✅ 仓库：{repo['full_name']}")
    print(f"   默认分支：{repo.get('default_branch', 'master')}")
    print(f"   私有：{repo.get('private')}")
else:
    print(f"❌ 错误：{response.text[:100]}")

# 2. 检查 .gitee/go.yml 是否存在
print("\n📌 2. 检查 .gitee/go.yml 配置...")
url = f"{base_url}/contents/.gitee/go.yml"
response = requests.get(url, headers=headers)
if response.status_code == 200:
    print("✅ .gitee/go.yml 存在")
    import base64
    content = base64.b64decode(response.json()['content']).decode('utf-8')
    print("   配置内容预览:")
    for line in content.split('\n')[:10]:
        print(f"   {line}")
else:
    print(f"❌ .gitee/go.yml 不存在")

# 3. 检查 Runner 状态
print("\n📌 3. 检查 Runner 状态...")
url = f"{base_url}/runners"
response = requests.get(url, headers=headers)
if response.status_code == 200:
    runners = response.json()
    if runners:
        print(f"✅ 找到 {len(runners)} 个 Runner:")
        for r in runners:
            status = "🟢 在线" if r.get('status') == 'online' else "🔴 离线"
            print(f"   - {r.get('name', 'Unknown')}: {status}")
            print(f"     OS: {r.get('os_name', 'Unknown')}")
    else:
        print("⚠️  未找到 Runner（可能需要先注册）")
else:
    print(f"❌ 无法获取 Runner 信息：{response.status_code}")

# 4. 检查最近的推送
print("\n📌 4. 检查最近的提交...")
url = f"{base_url}/commits?per_page=5"
response = requests.get(url, headers=headers)
if response.status_code == 200:
    commits = response.json()
    print(f"✅ 最近提交:")
    for c in commits[:3]:
        print(f"   - {c['sha'][:7]}: {c['commit']['message'][:50]}")
        print(f"     时间：{c['commit']['committer']['date']}")
else:
    print(f"❌ 无法获取提交记录")

# 5. 尝试手动触发流水线
print("\n📌 5. 尝试手动触发流水线...")
url = f"{base_url}/pipelines"
data = {"ref": "master"}
response = requests.post(url, json=data, headers=headers)
print(f"   状态码：{response.status_code}")
if response.status_code in [200, 201]:
    print("✅ 流水线已触发！")
    print(f"   响应：{json.dumps(response.json(), indent=2, ensure_ascii=False)[:200]}")
else:
    print(f"❌ 触发失败：{response.text[:200]}")
    print("\n💡 可能原因:")
    print("   1. Gitee Go 服务未正确激活")
    print("   2. Runner 未正确配置或未上线")
    print("   3. .gitee/go.yml 配置有误")
    print("   4. 需要在 Gitee 网页上手动开启流水线")

# 6. 检查 Webhooks
print("\n📌 6. 检查 Webhooks...")
url = f"{base_url}/hooks"
response = requests.get(url, headers=headers)
if response.status_code == 200:
    hooks = response.json()
    if hooks:
        print(f"✅ 找到 {len(hooks)} 个 Webhook")
        for h in hooks:
            print(f"   - {h.get('url', 'N/A')}")
    else:
        print("⚠️  未找到 Webhook（Gitee Go 通常不需要额外配置 webhook）")
else:
    print(f"❌ 无法获取 Webhook 信息")

print("\n" + "=" * 70)
print("💡 建议操作:")
print("  1. 在 Gitee 网页上访问：https://gitee.com/wlinuxhvxx/water-tower-task")
print("  2. 点击 '服务' → 'Gitee Go' → '流水线'")
print("  3. 确认 Runner 状态为 '在线'")
print("  4. 点击 '运行' 按钮手动触发一次构建")
print("  5. 查看日志确认问题")
print("=" * 70)
