#!/bin/bash

# Run chmod +x git.sh once to make this executable
# Create symbolic link `ln -s git.sh git` to run as just ./git

git pull origin dev; git st
git add .; git commit
git push origin dev
cd ./cw2/

# Set vim.basic as default with:
# sudo update-alternatives --config editor
