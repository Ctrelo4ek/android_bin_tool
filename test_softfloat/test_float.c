#include <stdio.h>
#include <time.h>

// 测试浮点运算的次数
#define NUM_ITERATIONS 100000000

void measure_float_performance() {
    float a = 1.2345f;
    float b = 2.3456f;
    float result;
    clock_t start_time, end_time;
    double total_time;

    start_time = clock();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        result = a * b + a - b;
    }
    end_time = clock();

    total_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Float performance test completed in %f seconds\n", total_time);
}

void measure_double_performance() {
    double a = 1.23456789;
    double b = 2.34567890;
    double result;
    clock_t start_time, end_time;
    double total_time;

    start_time = clock();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        result = a * b + a - b;
    }
    end_time = clock();

    total_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Double performance test completed in %f seconds\n", total_time);
}

int main() {
    printf("Starting float performance test...\n");
    measure_float_performance();
    
    printf("Starting double performance test...\n");
    measure_double_performance();

    return 0;
}