#!/bin/bash

# Run chmod +x git.sh once to make this executable
# Create symbolic link `ln -s git.sh git` to run as just ./git

git pull origin andreas; git st
git add .; git ci; git push origin andreas
cd ./cw2/

# Set vim.basic as default with:
# sudo update-alternatives --config editor
