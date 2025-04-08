#include <stdio.h>
#include <thread>

#include "CycleTimer.h"

typedef struct {
    float x0, x1;
    float y0, y1;
    unsigned int width;
    unsigned int height;
    int maxIterations;
    int* output;
    int threadId;
    int numThreads;
} WorkerArgs;


extern void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int numRows,
    int maxIterations,
    int output[]);

static inline int mandel(float c_re, float c_im, int count)
{
    float z_re = c_re, z_im = c_im;
    int i;
    for (i = 0; i < count; ++i) {

        if (z_re * z_re + z_im * z_im > 4.f)
            break;

        float new_re = z_re*z_re - z_im*z_im;
        float new_im = 2.f * z_re * z_im;
        z_re = c_re + new_re;
        z_im = c_im + new_im;
    }

    return i;
}

//
// workerThreadStart --
//
// Thread entrypoint.
// 每个线程应该做的事情，这里目前没有实现
void workerThreadStart(WorkerArgs * const args) {

    // 只要把任务切得细一点，交叉分配就可以了
    float dx = (args->x1 - args->x0) / args->width;
    float dy = (args->y1 - args->y0) / args->height;

    for (int j = args->threadId; j < args->height; j += args->numThreads) {
        for (int i = 0; i < args->width; ++i) {
            float x = args->x0 + i * dx;
            float y = args->y0 + j * dy;

            int index = (j * args->width + i);

            args->output[index] = mandel(x, y, args->maxIterations);
        }
    }

    printf("Hello world from thread %d\n", args->threadId);

}

//
// MandelbrotThread --
//
// Multi-threaded implementation of mandelbrot set image generation.
// Threads of execution are created by spawning std::threads.
void mandelbrotThread(
    int numThreads,
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations, int output[])
{
    static constexpr int MAX_THREADS = 32;

    // 查看使用线程数是否超过最大限制
    if (numThreads > MAX_THREADS)
    {
        fprintf(stderr, "Error: Max allowed threads is %d\n", MAX_THREADS);
        exit(1);
    }

    // 创建线程对象，但目前还没有线程实例
    // Creates thread objects that do not yet represent a thread.
    std::thread workers[MAX_THREADS];
    WorkerArgs args[MAX_THREADS];

    // 初始化所有线程的参数
    for (int i=0; i<numThreads; i++) {
      
        // TODO FOR CS149 STUDENTS: You may or may not wish to modify
        // the per-thread arguments here.  The code below copies the
        // same arguments for each thread
        args[i].x0 = x0;
        args[i].y0 = y0;
        args[i].x1 = x1;
        args[i].y1 = y1;
        args[i].width = width;
        args[i].height = height;
        args[i].maxIterations = maxIterations;
        args[i].numThreads = numThreads;
        args[i].output = output;
      
        args[i].threadId = i;
    }

    // 启动所有线程，除了线程0，每个线程都使用 args[i] 参数去执行函数 workerThreadStart
    // Spawn the worker threads.  Note that only numThreads-1 std::threads
    // are created and the main application thread is used as a worker
    // as well.
    for (int i=1; i<numThreads; i++) {
        workers[i] = std::thread(workerThreadStart, &args[i]);
    }
    
    // 线程0也执行 workerThreadStart
    workerThreadStart(&args[0]);

    // 等待所有线程结束
    // join worker threads
    for (int i=1; i<numThreads; i++) {
        workers[i].join();
    }
}

