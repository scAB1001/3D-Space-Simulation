#!/bin/bash

# Create symbolic link `ln -s git.sh git` to run as just ./git

git st
cd ~/github-projects/uni/comp3811/
git add .; git ci; git push origin dev
cd ./cw1/

# Set vim.basic as default with:
# sudo update-alternatives --config editor
