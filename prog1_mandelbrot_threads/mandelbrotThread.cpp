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


//
// workerThreadStart --
//
// Thread entrypoint.
// 每个线程应该做的事情，这里目前没有实现
void workerThreadStart(WorkerArgs * const args) {

    // TODO FOR CS149 STUDENTS: Implement the body of the worker
    // thread here. Each thread should make a call to mandelbrotSerial()
    // to compute a part of the output image.  For example, in a
    // program that uses two threads, thread 0 could compute the top
    // half of the image and thread 1 could compute the bottom half.

    // 1. 修改起始代码，使用两个处理器并行化曼德布罗特集的生成。具体来说，在线程 0 中计算图像的上半部分，
    // 在线程 1 中计算图像的下半部分。这种问题分解类型被称为空间分解，因为图像的不同空间区域由不同的处理器计算。
    if(0 == args->threadId) {

        mandelbrotSerial(args->x0, args->y0, args->x1, args->y1, 
            args->width, args->height, 0, args->height/2,
            args->maxIterations, args->output);

    }
    else {

        mandelbrotSerial(args->x0, args->y0, args->x1, args->y1, 
            args->width, args->height, args->height/2, args->height/2,
            args->maxIterations, args->output);

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

