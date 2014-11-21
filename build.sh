#!/bin/bash
# build file for linux OS

mkdir -p 3rdparty
git clone --depth=1 https://github.com/hoedown/hoedown 3rdparty/hoedown

qmake
make release
