//
// Created by benny on 2022/3/20.
//

#ifndef CPPCOROUTINES_TASKS_06_SLEEP_SLEEPAWAITER_H_
#define CPPCOROUTINES_TASKS_06_SLEEP_SLEEPAWAITER_H_

#include "Executor.h"
#include "Scheduler.h"
#include "coroutine_common.h"
#include "CommonAwaiter.h"

struct SleepAwaiter : Awaiter<void> {

    explicit SleepAwaiter(long long duration) noexcept
            : _duration(duration) {}

    template<typename _Rep, typename _Period>
    explicit SleepAwaiter(std::chrono::duration<_Rep, _Period> &&duration) noexcept
            : _duration(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()) {}

    [[nodiscard]] bool await_ready() override {
        return false;
    };

    void after_suspend() override {
        scheduler.execute([this] { resume(); }, _duration);
    }

    void before_resume() override{}
private:
    static SleepScheduler scheduler;
//    static inline std::unique_ptr<SleepScheduler> scheduler = std::make_unique<SleepScheduler>();

    long long _duration;
};

#endif //CPPCOROUTINES_TASKS_06_SLEEP_SLEEPAWAITER_H_
