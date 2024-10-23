//
// Created by benny on 2022/3/17.
//

#ifndef CPPCOROUTINES_04_TASK_TASK_H_
#define CPPCOROUTINES_04_TASK_TASK_H_

#include "coroutine_common.h"
#include "TaskPromise.h"

template<typename ResultType, typename Executor = LooperExecutor>
struct Task {

    using promise_type = TaskPromise<ResultType, Executor>;

    explicit Task(std::coroutine_handle<promise_type> handle) noexcept: handle(handle) {}

    Task(Task &&task) noexcept: handle(std::exchange(task.handle, {})) {}

    Task &operator=(Task &&task) = delete;

    Task(const Task &) = delete;

    Task &operator=(const Task &) = delete;

    ~Task() = default;

private:
    std::coroutine_handle<promise_type> handle;
};

template<typename Executor>
struct Task<void, Executor> {

    using promise_type = TaskPromise<void, Executor>;

    explicit Task(std::coroutine_handle<promise_type> handle) noexcept: handle{handle} {}

    Task(const Task &) = delete;

    Task &operator=(const Task &) = delete;

    Task(Task &&task) noexcept: handle(std::exchange(task.handle, nullptr)) {}

    Task &operator=(Task &&) = delete;

    ~Task() = default;


private:
    std::coroutine_handle<promise_type> handle;
};


#endif //CPPCOROUTINES_04_TASK_TASK_H_
