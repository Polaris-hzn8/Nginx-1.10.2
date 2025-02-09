
/**
* Copyright (C) 2024 Polaris-hzn8 / LuoChenhao
*
* Author: luochenhao
* Email: lch2022fox@163.com
* Time: Sat 08 Feb 2025 22:51:57 CST
* Github: https://github.com/Polaris-hzn8
* Src code may be copied only under the term's of the Apache License
* Please visit the http://www.apache.org/licenses/ Page for more detail.
*
**/

#include <cstdlib>
#include <stdint.h>
#include "nginx_memory_pool.h"

void* NginxMemoryPool::ngx_create_pool(size_t size)
{
    ngx_pool_t  *p;

    p = (ngx_pool_t*)malloc(size);
    if (p == nullptr) {
        return nullptr;
    }

    p->d.last = (u_char *) p + sizeof(ngx_pool_t);
    p->d.end = (u_char *) p + size;
    p->d.next = nullptr;
    p->d.failed = 0;

    size = size - sizeof(ngx_pool_t);
    p->max = (size < NGX_MAX_ALLOC_FROM_POOL) ? size : NGX_MAX_ALLOC_FROM_POOL;

    p->current = p;
    p->large = nullptr;
    p->cleanup = nullptr;

    _pool = p;

    return p;
}

void *NginxMemoryPool::ngx_palloc(size_t size)
{
    if (size <= _pool->max) {
        return ngx_palloc_small(size, 1);
    }
    return ngx_palloc_large(size);
}

void *NginxMemoryPool::ngx_pnalloc(size_t size)
{
    if (size <= _pool->max) {
        return ngx_palloc_small(size, 0);
    }
    return ngx_palloc_large(size);
}

void *NginxMemoryPool::ngx_pcalloc(size_t size)
{
    void *p;
    p = ngx_palloc(size);
    if (p) {
        ngx_memzero(p, size);
    }
    return p;
}

void NginxMemoryPool::ngx_pfree(void *p)
{
    ngx_pool_large_t  *l;

    for (l = _pool->large; l; l = l->next) {
        if (p == l->alloc) {
            free(l->alloc);
            l->alloc = nullptr;
            return;
        }
    }
    return;
}

void NginxMemoryPool::ngx_reset_pool()
{
    ngx_pool_t        *p;
    ngx_pool_large_t  *l;

    // 大块内存释放
    for (l = _pool->large; l; l = l->next) {
        if (l->alloc) {
            free(l->alloc);
        }
    }

    // 小块内存释放
    for (p = _pool; p; p = p->d.next) {
        p->d.last = (u_char *) p + sizeof(ngx_pool_t);
        p->d.failed = 0;
    }

    _pool->current = _pool;
    _pool->large = NULL;
}

void NginxMemoryPool::ngx_destroy_pool()
{
    ngx_pool_t          *p, *n;
    ngx_pool_large_t    *l;
    ngx_pool_cleanup_t  *c;

    // 释放外部资源
    for (c = _pool->cleanup; c; c = c->next) {
        if (c->handler) {
            c->handler(c->data);
        }
    }

    // 释放大块内存
    for (l = _pool->large; l; l = l->next) {
        if (l->alloc) {
            free(l->alloc);
        }
    }

    // 释放小块内存
    for (p = _pool, n = _pool->d.next; /* void */; p = n, n = n->d.next) {
        free(p);

        if (n == nullptr) {
            break;
        }
    }
}

ngx_pool_cleanup_s *NginxMemoryPool::ngx_pool_cleanup_add(size_t size)
{
    ngx_pool_cleanup_t  *c;

    // 存储头信息（小块内存池）
    c = (ngx_pool_cleanup_t*)ngx_palloc(sizeof(ngx_pool_cleanup_t));
    if (c == nullptr) {
        return nullptr;
    }

    if (size) {
        c->data = ngx_palloc(size);
        if (c->data == nullptr) {
            return nullptr;
        }

    } else {
        c->data = nullptr;
    }

    // 回调函数成链
    c->handler = nullptr;
    c->next = _pool->cleanup;

    _pool->cleanup = c;

    return c;
}

void *NginxMemoryPool::ngx_palloc_small(size_t size, ngx_uint_t align)
{
    u_char      *m;
    ngx_pool_t  *p;

    p = _pool->current;

    do {
        // 可分配内存起始地址
        m = p->d.last;

        // 调整起始地址为4或8的整数倍
        if (align) {
            m = ngx_align_ptr(m, NGX_ALIGNMENT);
        }

        // 内存池空间的内存空间 >= 申请的内存空间（高效的内存分配）
        if ((size_t) (p->d.end - m) >= size) {
            p->d.last = m + size;
            return m;
        }

        p = p->d.next;

    } while (p);

    return ngx_palloc_block(size);
}

void *NginxMemoryPool::ngx_palloc_block(size_t size)
{
    u_char      *m;
    size_t       psize;
    ngx_pool_t  *p, *newpool;

    psize = (size_t) (_pool->d.end - (u_char *) _pool);

    m = (u_char*)malloc(psize);
    if (m == nullptr) {
        return nullptr;
    }

    newpool = (ngx_pool_t *) m;

    newpool->d.end = m + psize;
    newpool->d.next = nullptr;
    newpool->d.failed = 0;

    // 存储信息减少
    m += sizeof(ngx_pool_data_t);
    m = ngx_align_ptr(m, NGX_ALIGNMENT);
    newpool->d.last = m + size;

    // 内存块多次分配失败 直接跳过
    for (p = newpool->current; p->d.next; p = p->d.next) {
        if (p->d.failed++ > 4) {
            newpool->current = p->d.next;
        }
    }

    p->d.next = newpool;

    return m;
}

void *NginxMemoryPool::ngx_palloc_large(size_t size)
{
    void              *p;
    ngx_uint_t         n;
    ngx_pool_large_t  *large;

    p = malloc(size);
    if (p == nullptr) {
        return nullptr;
    }

    n = 0;

    // 若旧的大块内存已被释放
    for (large = _pool->large; large; large = large->next) {
        if (large->alloc == nullptr) {
            large->alloc = p;
            return p;
        }

        if (n++ > 3) {
            break;
        }
    }

    // 大块内存的头信息 使用小块内存分配的方式进行 分配
    large = (ngx_pool_large_t*)ngx_palloc_small(sizeof(ngx_pool_large_t), 1);
    if (large == nullptr) {
        free(p);
        return nullptr;
    }

    // 建立大块内存链表结点
    large->alloc = p;
    large->next = _pool->large;

    _pool->large = large;
    return p;
}



