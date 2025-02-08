
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_PALLOC_H_INCLUDED_
#define _NGX_PALLOC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * NGX_MAX_ALLOC_FROM_POOL should be (ngx_pagesize - 1), i.e. 4095 on x86.
 * On Windows NT it decreases a number of locked pages in a kernel.
 */
#define NGX_MAX_ALLOC_FROM_POOL  (ngx_pagesize - 1) //4096区分内存块

#define NGX_DEFAULT_POOL_SIZE    (16 * 1024)        //内存池大小(默认)

#define NGX_POOL_ALIGNMENT       16                 //内存池大小(最小)
#define NGX_MIN_POOL_SIZE                                                     \
    ngx_align((sizeof(ngx_pool_t) + 2 * sizeof(ngx_pool_large_t)),            \
              NGX_POOL_ALIGNMENT)

/////////////////////////////////////////////////////////////////////////////////////////
// 内存池的清理
// 较大内存块的管理以及文件清理

// 内存清理回调
typedef void (*ngx_pool_cleanup_pt)(void *data);

// struct 内存池清理信息
typedef struct ngx_pool_cleanup_s  ngx_pool_cleanup_t;
struct ngx_pool_cleanup_s {
    ngx_pool_cleanup_pt   handler;  // 清理回调
    void                 *data;     // 待清理数据
    ngx_pool_cleanup_t   *next;     // 多个释放资源动作
};

// struct 较大的内存块管理
typedef struct ngx_pool_large_s  ngx_pool_large_t;
struct ngx_pool_large_s {
    ngx_pool_large_t     *next;
    void                 *alloc;
};


typedef struct {
    u_char               *last;
    u_char               *end;
    ngx_pool_t           *next;
    ngx_uint_t            failed;
} ngx_pool_data_t;


struct ngx_pool_s {
    ngx_pool_data_t       d;
    size_t                max;
    ngx_pool_t           *current;
    ngx_chain_t          *chain;
    ngx_pool_large_t     *large;
    ngx_pool_cleanup_t   *cleanup;
    ngx_log_t            *log;
};


typedef struct {
    ngx_fd_t              fd;
    u_char               *name;
    ngx_log_t            *log;
} ngx_pool_cleanup_file_t;

/////////////////////////////////////////////////////////////////////////////////////////
// Nginx内存管理
void *ngx_alloc(size_t size, ngx_log_t *log);
void *ngx_calloc(size_t size, ngx_log_t *log);

// 创建内存池
ngx_pool_t *ngx_create_pool(size_t size, ngx_log_t *log);
// 销毁内存池
void ngx_destroy_pool(ngx_pool_t *pool);
// 重置内存池
void ngx_reset_pool(ngx_pool_t *pool);

// 内存分配
void *ngx_palloc(ngx_pool_t *pool, size_t size);    // 内存对齐
void *ngx_pnalloc(ngx_pool_t *pool, size_t size);   // 内存对齐不考虑
void *ngx_pcalloc(ngx_pool_t *pool, size_t size);   // 初始化
void *ngx_pmemalign(ngx_pool_t *pool, size_t size, size_t alignment);
// 内存释放
ngx_int_t ngx_pfree(ngx_pool_t *pool, void *p);

// 内存清理
ngx_pool_cleanup_t *ngx_pool_cleanup_add(ngx_pool_t *p, size_t size);
void ngx_pool_run_cleanup_file(ngx_pool_t *p, ngx_fd_t fd);
void ngx_pool_cleanup_file(void *data);
void ngx_pool_delete_file(void *data);


#endif /* _NGX_PALLOC_H_INCLUDED_ */
