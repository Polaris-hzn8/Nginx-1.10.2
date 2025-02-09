
/**
* Copyright (C) 2024 Polaris-hzn8 / LuoChenhao
*
* Author: luochenhao
* Email: lch2022fox@163.com
* Time: Sat 08 Feb 2025 22:50:53 CST
* Github: https://github.com/Polaris-hzn8
* Src code may be copied only under the term's of the Apache License
* Please visit the http://www.apache.org/licenses/ Page for more detail.
*
**/

#ifndef _NGINX_MEMORY_POOL_H
#define _NGINX_MEMORY_POOL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using u_char = unsigned char;
using ngx_uint_t = unsigned long int;

typedef struct ngx_pool_s           ngx_pool_t;
typedef struct ngx_pool_cleanup_s   ngx_pool_cleanup_t;
typedef struct ngx_pool_large_s     ngx_pool_large_t;

// 内存清理回调
typedef void (*ngx_pool_cleanup_pt)(void *data);

// struct 内存池清理信息
struct ngx_pool_cleanup_s {
    ngx_pool_cleanup_pt   handler;  // 清理回调
    void                 *data;     // 待清理数据
    ngx_pool_cleanup_t   *next;     // 多个释放资源动作
};

// struct 较大的内存块管理
struct ngx_pool_large_s {
    ngx_pool_large_t     *next;
    void                 *alloc;
};

// 内存池数据(小块内存)
typedef struct {
    u_char               *last;
    u_char               *end;
    ngx_pool_s           *next;
    ngx_uint_t            failed;
} ngx_pool_data_t;

// 内存池
struct ngx_pool_s {
    ngx_pool_data_t       d;
    size_t                max;          // 大/小块内存分界
    ngx_pool_s           *current;      // 小块内存
    // ngx_chain_t          *chain;
    ngx_pool_large_t     *large;        // 大块内存
    ngx_pool_cleanup_t   *cleanup;
    // ngx_log_t            *log;
};

const int ngx_pagesize = 4096;                      //物理页面大小

#define NGX_MAX_ALLOC_FROM_POOL  (ngx_pagesize - 1) //4096区分内存块

#define NGX_DEFAULT_POOL_SIZE    (16 * 1024)        //内存池大小(默认)

#define NGX_ALIGNMENT   sizeof(unsigned long)    /* platform word */

// 将内存开辟 调整到临近a的倍数
#define ngx_align(d, a)     (((d) + (a - 1)) & ~(a - 1))
#define ngx_align_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

#define NGX_POOL_ALIGNMENT       16                 //内存池大小(最小)
#define NGX_MIN_POOL_SIZE                                                     \
    ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),            \
              NGX_POOL_ALIGNMENT)

#define ngx_memzero(buf, n)       (void) memset(buf, 0, n)
#define ngx_memset(buf, c, n)     (void) memset(buf, c, n)

class NginxMemoryPool {
public:
    // 内存池创建
    void *ngx_create_pool(size_t size);
    // 内存池申请内存 考虑内存对齐
    void *ngx_palloc(size_t size);
    // 内存池申请内存 不考虑内存对齐
    void *ngx_pnalloc(size_t size);
    // 内存池申请内存 考虑内存对比并初始化
    void *ngx_pcalloc(size_t size);
    // 释放大块内存
    void ngx_pfree(void *p);
    // 内存池重置
    void ngx_reset_pool();
    // 内存池销毁
    void ngx_destroy_pool();
    // 添加回调清理操作
    ngx_pool_cleanup_s *ngx_pool_cleanup_add(size_t size);
private:
    // 小块内存分配
    void* ngx_palloc_small(size_t size, ngx_uint_t align);
    // 大块内存分配
    void* ngx_palloc_large(size_t size);
    // 开辟新的小块内存池
    void* ngx_palloc_block(size_t size);

    ngx_pool_s* _pool;
};

#endif
