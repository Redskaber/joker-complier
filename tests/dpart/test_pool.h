//
// Created by Kilig on 2025/4/19.
//

#ifndef JOKER_TEST_POOL_H
#define JOKER_TEST_POOL_H
#include "pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <setjmp.h>
#include <signal.h>

#define TEST_COUNT 10000000UL  // 增加到1000万次操作
#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))


// 获取高精度时间戳（单位：秒）
__attribute__((unused)) double get_high_res_time() {
#ifdef _WIN32
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
#endif
}


void test_basic_operation() {
    Pool* pool = pool_create(sizeof(int), 100);
    assert(pool != NULL);

    // 测试基本分配/释放
    int* obj = pool_alloc(pool);
    assert(obj != NULL);
    *obj = 42;
    assert(*obj == 42);
    pool_free(pool, obj);

    // 测试池空情况
    void* objs[100];
    for (int i = 0; i < 100; ++i) {
        objs[i] = pool_alloc(pool);
        assert(objs[i] != NULL);
    }
    assert(pool_alloc(pool) == NULL);

    for(int i = 0; i < 100; ++i) {
        pool_free(pool, objs[i]);
    }
    pool_destroy(&pool);
    assert(pool == NULL);
    printf("Basic test passed\n");
}


void benchmark_single_thread() {
    Pool* pool = pool_create(sizeof(int), TEST_COUNT+1);

    double start = get_high_res_time();
    for (size_t i = 0; i < TEST_COUNT; ++i) {
        int* p = pool_alloc(pool);
        *p = (int)i;
        pool_free(pool, p);
    }
    double duration = get_high_res_time() - start;

    printf("Single thread: %.2f ops/sec\n", TEST_COUNT/duration);
    pool_destroy(&pool);
    assert(pool == NULL);
}



void benchmark_batch_ops() {
    Pool* pool = pool_create(sizeof(int), 1000000);
    void* objs[100];

    // 预热运行（填充缓存）
    for (int i = 0; i < 1000; ++i) {
        size_t count = pool_alloc_batch(pool, objs, 100);
        pool_free_batch(pool, objs, count);
    }

    // 正式测试
    size_t total_alloc = 0;
    const size_t iterations = TEST_COUNT / 100;    // 100000次迭代

    double start = get_high_res_time();
    for (size_t i = 0; i < iterations; ++i) {
        size_t count = pool_alloc_batch(pool, objs, 100);
        assert(count == 100);                      // assert分配数量
        pool_free_batch(pool, objs, count);
        total_alloc += count;
    }
    double duration = get_high_res_time() - start;
    assert(total_alloc == iterations * 100);

    if (duration < 1e-9) {                          // 1ns
        printf("Batch(100) ops: [measurement error]\n");
    } else {
        double ops_per_sec = (double)(iterations * 100) / duration;     // 每次迭代100个操作
        printf("Batch(100) ops: %.2f Mops/sec\n", ops_per_sec / 1e6);
    }

    pool_destroy(&pool);
}


__attribute__((unused)) bool assert_triggered = false;
void handler(int sig) {
    (void)sig;
    assert_triggered = 1;
}
void test_edge_cases() {
    // 测试零容量
    Pool* p1 = pool_create(16, 0);
    assert(p1 == NULL && errno == EINVAL);

    // 测试无效释放
    Pool* pool = pool_create(16, 10);
//    int dummy;

    // 需要禁用abort()来捕获断言
    #ifdef NDEBUG
    printf("Assert test skipped in release build\n");
    #else
    // 使用信号处理捕获断言
//     signal(SIGABRT, handler);
//     pool_free(pool, &dummy); // 应该触发断言
//     assert(assert_triggered);
    #endif

    pool_destroy(&pool);
    assert(pool == NULL);
    printf("Edge cases test passed\n");
}


void test_memory_leak() {
    #ifdef __SANITIZE_ADDRESS__
    printf("Running with address sanitizer\n");
    #else
    printf("Compile with -fsanitize=address for leak check\n");
    #endif

    for (int i = 0; i < 1000; ++i) {
        Pool* pool = pool_create(64, 1000);
        void* obj = pool_alloc(pool);
        pool_free(pool, obj);
        pool_destroy(&pool);
        assert(pool == NULL);
    }
    printf("Memory leak test completed\n");
}

void test_stress_memory() {
    const int ITERATIONS = 1000000;
    for (int i = 0; i < ITERATIONS; ++i) {
        Pool* pool = pool_create(64, 1024);
        void* objs[1024];

        // 分配并保留对象不释放
        size_t count = pool_alloc_batch(pool, objs, 1024);
        assert(count == 1024);

        // 仅清理池不释放对象(内存泄露错误)
        // pool_destroy(&pool);

        pool_free_batch(pool, objs, 1024);
        pool_destroy(&pool);
        assert(pool == NULL);
    }
}


void test_object_lifecycle() {
    const int MAGIC_NUMBER = 0xDEADBEEF;
    Pool* pool = pool_create(sizeof(int), 100);

    // 分配并写入数据
    int* ptr = pool_alloc(pool);
    *ptr = MAGIC_NUMBER;

    // 释放后尝试读取（应崩溃）
    pool_free(pool, ptr);
    #ifndef NDEBUG
    volatile int crash_test = *ptr; // 在debug build中应触发崩溃
    (void)crash_test;
    #endif

    // 重新分配验证数据被覆盖
    int* new_ptr = pool_alloc(pool);
    assert(new_ptr == ptr); // 验证对象重用
    assert(*new_ptr != MAGIC_NUMBER); // 内存已被初始化覆盖

    pool_free(pool, new_ptr);
    pool_destroy(&pool);
    printf("Object lifecycle test passed\n");
}


void test_statistics() {
    Pool* pool = pool_create(64, 1000);

    // 分配100次
    void* ptrs[100];
    size_t count = pool_alloc_batch(pool, ptrs, 100);
    assert(count == 100);

    // 释放50个
    pool_free_batch(pool, ptrs, 100);

    pool_flush_cache(pool);

    // 验证统计
    assert(atomic_load(&pool->alloc_cnt) == 100);
    assert(atomic_load(&pool->free_cnt) == 100);
    assert(atomic_load(&pool->contention_counter) < 10); // 冲突次数应极少

    pool_destroy(&pool);
    printf("Statistics validation passed\n");
}


void test_alignment() {
    // 测试不同对象尺寸
    size_t sizes[] = {1, 8, 16, 33, 64, 127, 256};
    for(size_t i=0; i<sizeof(sizes)/sizeof(sizes[0]); i++) {
        Pool* pool = pool_create(sizes[i], 100);

        // 分配对象验证对齐
        void* ptr = pool_alloc(pool);
        uintptr_t addr = (uintptr_t)ptr;
        assert(addr % 16 == 0); // 数据区16字节对齐

        // 节点地址验证
        Node* node = (Node*)((uint8_t*)ptr - offsetof(Node, data));
        assert((uintptr_t)node % CACHE_LINE_SIZE == 0);

        pool_free(pool, ptr);
        pool_destroy(&pool);
    }
    printf("Cross-platform alignment test passed\n");
}


#endif //JOKER_TEST_POOL_H
