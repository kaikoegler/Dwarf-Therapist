#!/bin/sh
set -e
brew update
for pkg in qt cmake ninja; do
    brew outdated $pkg || brew install $pkg
done
