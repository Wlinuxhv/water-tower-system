#!/usr/bin/env python3
import os
import base64
import requests
import json

TOKEN = "146a77e72bc5060e569d4e2f3750a2b2"
OWNER = "wlinuxhvxx"
REPO = "water-tower-task"
BRANCH = "master"

headers = {
    "Authorization": f"token {TOKEN}",
    "Content-Type": "application/json"
}

base_url = f"https://gitee.com/api/v5/repos/{OWNER}/{REPO}/contents"

# Files to upload (skip .git directory and local.properties)
files_to_upload = [
    ".gitee/go.yml",
    ".gitignore",
    "README.md",
    "UPLOAD_GUIDE.md",
    "build.gradle",
    "settings.gradle",
    "gradle.properties",
    "gradle/wrapper/gradle-wrapper.properties",
    "app/build.gradle",
    "app/proguard-rules.pro",
    "app/src/main/AndroidManifest.xml",
    "app/src/main/java/com/watertower/task/MainActivity.java",
    "app/src/main/res/layout/activity_main.xml",
    "app/src/main/res/values/colors.xml",
    "app/src/main/res/values/strings.xml",
    "app/src/main/res/values/themes.xml",
]

base_dir = "/app/working/water_tower_pcb/android_app"

print(f"开始上传文件到 Gitee: {OWNER}/{REPO}")
print("=" * 60)

for file_path in files_to_upload:
    full_path = os.path.join(base_dir, file_path)
    
    if not os.path.exists(full_path):
        print(f"❌ 文件不存在：{file_path}")
        continue
    
    # Read file content
    with open(full_path, 'rb') as f:
        content = f.read()
    
    content_base64 = base64.b64encode(content).decode('utf-8')
    
    # Prepare API request for creating files
    # Use POST with files parameter
    api_url = f"{base_url}"
    
    data = {
        "content": content_base64,
        "message": f"Upload {file_path}",
        "branch": BRANCH,
        "path": file_path
    }
    
    try:
        # First check if file exists
        check_url = f"{base_url}/{file_path}"
        check_response = requests.get(check_url, headers=headers)
        
        if check_response.status_code == 200:
            # File exists, need to update with SHA
            sha = check_response.json().get("sha", "")
            data["sha"] = sha
            response = requests.put(check_url, json=data, headers=headers)
        else:
            # File doesn't exist, create it using POST
            response = requests.post(api_url, json=data, headers=headers)
        
        if response.status_code in [200, 201]:
            print(f"✅ {file_path}")
        else:
            print(f"❌ {file_path}: {response.status_code}")
            if "message" in response.json():
                print(f"   {response.json()['message']}")
    except Exception as e:
        print(f"❌ {file_path}: Error - {str(e)}")

print("=" * 60)
print("上传完成！")
