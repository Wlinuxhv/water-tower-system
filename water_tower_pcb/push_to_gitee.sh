#!/bin/bash
cd /app/working/water_tower_pcb/android_app

# Configure git to use token
git config credential.helper store

# Create credential file
echo "https://146a77e72bc5060e569d4e2f3750a2b2@gitee.com" > ~/.git-credentials
chmod 600 ~/.git-credentials

# Set up credential helper to use the file
git config credential.helper 'store --file=~/.git-credentials'

# Try to push
git push -u origin master
