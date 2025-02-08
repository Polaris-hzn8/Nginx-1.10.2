
/**
* Copyright (C) 2024 Polaris-hzn8 / LuoChenhao
*
* Author: luochenhao
* Email: lch2022fox@163.com
* Time: Sat 08 Feb 2025 09:36:24 CST
* Github: https://github.com/Polaris-hzn8
* Src code may be copied only under the term's of the Apache License
* Please visit the http://www.apache.org/licenses/ Page for more detail.
*
**/

#include <nginx.h>
#include <ngx_core.h>
#include <ngx_config.h>
#include <ngx_palloc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ngx_log_error_core(ngx_uint_t level, ngx_log_t *log, ngx_err_t err, const char *fmt, ...) {

}

// 内存池测试
typedef struct {
    char *ptr;
	FILE *file;
} ngx_memtest_data_t;

void ngx_memtest_free(char *p) {
	printf("ngx_memtest_free free ptr memory!");
	free(p);
}

void ngx_memtest_fclose(FILE *file) {
	printf("ngx_memtest_fclose close file.");
	fclose(file);
}

void main()
{
	// 内存池创建
	// 512 - sizeof(ngx_pool_t) 对比 4095
	ngx_pool_t* pool = ngx_create_pool(512, NULL);
	if (pool == NULL) {
		printf("ngx_create_pool failed...");
		return;
	}

	// 分配小块内存
	void* ptr_s = ngx_palloc(pool, 128);
	if (ptr_s == NULL) {
		printf("ngx_palloc 128 bytes failed...");
		return;
	}

	// 分配大块内存
	ngx_memtest_data_t* ptr_l = ngx_palloc(pool, 512);
	if (ptr_l == NULL) {
		printf("ngx_palloc 512 bytes failed...");
		return;
	}

	// 大块内存外部资源分配
	ptr_l->ptr = malloc(12);
	strcpy(ptr_l->ptr, "hello world");
	ptr_l->file = fopen("ngx_memtest_data.txt", "rw");

	// 大块内存外部资源释放
	ngx_pool_cleanup_t *cl1 = ngx_pool_cleanup_add(pool, sizeof(char*));
	cl1->handler = ngx_memtest_free;
	cl1->data = ptr_l->ptr;
	ngx_pool_cleanup_t *cl2 = ngx_pool_cleanup_add(pool, sizeof(FILE*));
	cl2->handler = ngx_memtest_fclose;
	cl2->data = ptr_l->file;

	// 内存池销毁
	ngx_destroy_pool(pool);/* 回调所有内存释放函数 释放大块内存 释放小块内存 */

	return;
}