#!/bin/bash

dir="$(realpath $(dirname $0))"

CC=clang CXX=clang++ meson setup builddir --reconfigure
cd builddir

CC=clang CXX=clang++ meson compile -j12

./libdbusmenugtk4

cd ..