#!/bin/bash

set -ex
cd "$(dirname "$0")"

function compile() {
  "$CLANG" \
    -std=c++11 \
    -o bf \
    bf.cc \
    $("$LLVM_CONFIG" --cppflags --ldflags --libs all)
  ./bf
}

case "$1" in
  --osx)
    brew install llvm33
    prefix="$(brew --prefix)/opt/llvm33/bin/"
    export CLANG="clang++"
    export LLVM_CONFIG="$prefix/llvm-config-3.3"
    compile
    ;;
  --vagrant-guest)
    pkg="clang+llvm-3.3-amd64-Ubuntu-12.04.2"
    if [[ ! -d "$HOME/$pkg" ]] ; then
      file="${pkg}.tar.gz"
      url="http://llvm.org/releases/3.3/$file"
      wget -c -O "$HOME/$file" "$url"
      tar -x -C "$HOME" -f "$HOME/$file"
    fi
    export CLANG="$HOME/$pkg/bin/clang++"
    export LLVM_CONFIG="$HOME/$pkg/bin/llvm-config"
    compile
    ;;
  --vagrant-host)
    vagrant up
    vagrant ssh -c '$HOME/project/build.sh --vagrant-guest'
    ;;
esac

