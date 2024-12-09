#!/usr/bin/env sh

mkdir build ; cd build || exit

cmake .. ; make

mv cache_system* ../example ; cd ..
