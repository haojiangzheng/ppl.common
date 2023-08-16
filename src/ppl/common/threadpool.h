#ifndef _ST_HPC_PPL_COMMON_THREADPOOL_H_
#define _ST_HPC_PPL_COMMON_THREADPOOL_H_

#include "ppl/common/retcode.h"
#include "ppl/common/message_queue.h"
#include <vector>
#include <memory>

namespace ppl { namespace common {

class ThreadTask {
public:
    virtual ~ThreadTask() {}
    /**
       returns a task that will be executed right after Run() returns,
       or returns nullptr so that scheduler will pick up a task from task queue.
     */
    virtual std::shared_ptr<ThreadTask> Run() = 0;
};

class JoinableThreadTask : public ThreadTask {
public:
    JoinableThreadTask();
    virtual ~JoinableThreadTask();
    std::shared_ptr<ThreadTask> Run() override final;
    void Join();

protected:
    virtual bool IsFinished() const = 0;
    virtual std::shared_ptr<ThreadTask> Process() = 0;

private:
    pthread_mutex_t mutex_;
    pthread_cond_t cond_;
};

class ThreadPool final {
public:
    struct ThreadInfo {
        pthread_t pid; // pthread_self()
#ifndef _MSC_VER
        pid_t tid; // gettid()
#endif
    };

public:
    ThreadPool();
    ~ThreadPool();

    ppl::common::RetCode Init(uint32_t thread_num = 0);
    uint32_t GetThreadNum() const { return threads_.size(); }
    ppl::common::RetCode AddTask(const std::shared_ptr<ThreadTask>&);

    /**
       0 <= thread_id < ThreadNum()
       0 <= core_list[i] < sysconf(_SC_NPROCESSORS_ONLN)
     */
    ppl::common::RetCode SetAffinity(uint32_t thread_id, const uint32_t* core_list, uint32_t core_num);

private:
    static void* ThreadWorker(void*);

    std::vector<ThreadInfo> threads_;
    MessageQueue<std::shared_ptr<ThreadTask>> queue_;
    uint32_t cpu_core_num_;
};

}}

#endif