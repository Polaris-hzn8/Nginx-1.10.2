
#!/bin/sh
# Copyright (C) 2024 Polaris-hzn8 / LuoChenhao
# Author: luochenhao
# Email: lch2022fox@163.com
# Time: Sat 08 Feb 2025 13:04:04 CST
# Github: https://github.com/Polaris-hzn8

# gcc -c -g
# -I src/core
# -I src/event
# -I src/event/modules
# -I src/os/unix
# -I src/http
# -I src/http/modules
# -I objs
# -o lch_test/ngx_memtest.o
# lch_test/ngx_memtest.c
gcc -c -g -I src/core -I src/event -I src/event/modules -I src/os/unix -I src/http -I src/http/modules -I objs -o lch_test/ngx_memtest.o lch_test/ngx_memtest.c

# gcc -o lch_test/ngx_memtest
# ngx_memtest.o
# objs/src/core/ngx_palloc.o
# objs/src/os/unix/ngx_alloc.o
gcc -o lch_test/ngx_memtest lch_test/ngx_memtest.o objs/src/core/ngx_palloc.o objs/src/os/unix/ngx_alloc.o
