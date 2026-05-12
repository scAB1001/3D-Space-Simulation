#!/bin/bash

git pull origin dev; git st
git add .; git commit
git push origin dev
cd ./cw2/
