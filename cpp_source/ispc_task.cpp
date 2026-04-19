#include <cstdint>
#include <cstdlib>
#include <assert.h>
#include <climits>
#include <ppl.h>
#include <vector>

#include <thread>
#include <iostream>

extern "C" void* ISPCAlloc(void** handlePtr, int64_t size, int32_t alignment);
extern "C" void ISPCLaunch(void** handlePtr, void* f, void* data, int count0, int count1, int count2);
extern "C" void ISPCSync(void* handle);

using TaskFuncPtr = void (*)(void* data, int threadIndex, int threadCount,
                                            int taskIndex, int taskCount,
                                            int taskIndex0, int taskIndex1, int taskIndex2,
                                            int taskCount0, int taskCount1, int taskCount2);

static void* alloc_aligned_memory(int64_t size, int32_t alignment)
{
    return _aligned_malloc(size, alignment);
}

static void  free_aligned_memory(void *ptr) 
{
    assert(ptr != nullptr);
   return _aligned_free(ptr);
}

struct TaskRecord 
{
    TaskRecord() = default;
    TaskRecord(const TaskRecord&) = delete;
    TaskRecord& operator=(const TaskRecord&) = delete;
    TaskRecord(TaskRecord&&) = delete;
    TaskRecord& operator=(TaskRecord&&) = delete;

    std::vector<void*> mem_ptrs;

    void* alloc(int64_t size, int32_t alignment)
    {
        void* ptr = alloc_aligned_memory(size, alignment);
        if (ptr != nullptr) {
            mem_ptrs.push_back(ptr);
        }
        return ptr;
    }

    void free() 
    {
        for (auto& ptr : mem_ptrs)
        {
            if (ptr != nullptr) {
                free_aligned_memory(ptr);
            }
        }
        mem_ptrs.clear();
    }
    
    ~TaskRecord()
    {
        free();
    }
};


void* ISPCAlloc(void** handlePtr, int64_t size, int32_t alignment) 
{
    TaskRecord* task_record_ptr = nullptr;
    if (*handlePtr == nullptr) 
    {
        task_record_ptr = new TaskRecord();
        *handlePtr = (void*)task_record_ptr;
    }
    else {
        task_record_ptr = (TaskRecord*)(*handlePtr);
    }

    return task_record_ptr->alloc(size, alignment);
}

void ISPCLaunch(void** handlePtr, void* f, void* data, int count0, int count1, int count2) 
{
    TaskRecord* task_record_ptr = nullptr;
    if (*handlePtr == nullptr)
    {
        task_record_ptr = new TaskRecord();
        *handlePtr = (void*)task_record_ptr;
    }
    else {
        task_record_ptr = (TaskRecord*)(*handlePtr);
    }

    const int task_count_0 = count0;
    const int task_count_1 = count1;
    const int task_count_2 = count2;
    const int task_count = count0 * count1 * count2;
    assert(task_count <= INT_MAX );

    concurrency::parallel_for(0, task_count, [&](int task_index)
    {
        const int task_index_0 = task_index % task_count_0;
        const int task_index_1 = (task_index / task_count_0) % task_count_1;
        const int task_index_2 = (task_index / task_count_0) / task_count_1;

        TaskFuncPtr task_function = (TaskFuncPtr)(f);

        task_function(data, 
            0, 0, // note: thread count and thread index, dummy.
            task_index, task_count,
            task_index_0, task_index_1, task_index_2,
            task_count_0, task_count_1, task_count_2);
    });
}

void ISPCSync(void* handle) 
{
    TaskRecord* task_record_ptr = (TaskRecord*)(handle);
    if (task_record_ptr != nullptr) {
        delete task_record_ptr;
    }
}