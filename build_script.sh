#!/usr/bin/env sh

mkdir build ; cd build || exit

cmake .. ; make

mv cache_system* ../example ; cd ..

python3 example/cache_performance_test.py
