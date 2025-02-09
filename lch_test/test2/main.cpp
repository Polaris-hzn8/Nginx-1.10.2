/**
* Copyright (C) 2024 Polaris-hzn8 / LuoChenhao
*
* Author: luochenhao
* Email: lch2022fox@163.com
* Time: Sun 09 Feb 2025 11:39:13 CST
* Github: https://github.com/Polaris-hzn8
* Src code may be copied only under the term's of the Apache License
* Please visit the http://www.apache.org/licenses/ Page for more detail.
*
**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nginx_memory_pool.h"

// 内存池测试
typedef struct {
    char *ptr;
	FILE *file;
} ngx_memtest_data_t;

void ngx_memtest_free(void *arg) {
	char *p = (char*)arg;
	printf("ngx_memtest_free free ptr memory!\n");
	free(p);
}

void ngx_memtest_fclose(void *arg) {
	FILE* file = (FILE*)arg;
	printf("ngx_memtest_fclose close file.\n");
	fclose(file);
}

int main()
{
    NginxMemoryPool ngxMemPool;
	// 内存池创建
	// 512 - sizeof(ngx_pool_t) 对比 4095
	ngx_pool_t* pool = (ngx_pool_t*)ngxMemPool.ngx_create_pool(512);
	if (pool == nullptr) {
		printf("ngx_create_pool failed...");
		return -1;
	}

	// 分配小块内存
	void* ptr_s = ngxMemPool.ngx_palloc(128);
	if (ptr_s == nullptr) {
		printf("ngx_palloc 128 bytes failed...");
		return -1;
	}

	// 分配大块内存
	ngx_memtest_data_t* ptr_l = (ngx_memtest_data_t*)ngxMemPool.ngx_palloc(512);
	if (ptr_l == nullptr) {
		printf("ngx_palloc 512 bytes failed...");
		return -1;
	}

	// 大块内存外部资源分配
	ptr_l->ptr = (char*)malloc(12);
	strcpy(ptr_l->ptr, "hello world");
	ptr_l->file = fopen("ngx_memtest_data.txt", "w");

	// 大块内存外部资源释放
	ngx_pool_cleanup_t *cl1 = (ngx_pool_cleanup_t*)ngxMemPool.ngx_pool_cleanup_add(sizeof(char*));
	cl1->handler = ngx_memtest_free;
	cl1->data = ptr_l->ptr;
	ngx_pool_cleanup_t *cl2 = (ngx_pool_cleanup_t*)ngxMemPool.ngx_pool_cleanup_add(sizeof(FILE*));
	cl2->handler = ngx_memtest_fclose;
	cl2->data = ptr_l->file;

	// 内存池销毁
	ngxMemPool.ngx_destroy_pool();/* 回调所有内存释放函数 释放大块内存 释放小块内存 */

	return 0;
}
