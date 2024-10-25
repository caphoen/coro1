//
// Created by benny on 2022/3/26.
//

#ifndef CPPCOROUTINES_TASKS_08_AWAITER_COMMONAWAITER_H_
#define CPPCOROUTINES_TASKS_08_AWAITER_COMMONAWAITER_H_

#include "Executor.h"
#include "Result.h"
#include "coroutine_common.h"

template<typename R>
struct Awaiter {

    using ResultType = R;

    [[nodiscard]] virtual bool await_ready() = 0;

    virtual void await_suspend(std::coroutine_handle<> handle) {
        this->_handle = handle;
        after_suspend();
    }

    virtual R await_resume() {
        before_resume();
        return _result->get_or_throw();
    }

    void resume(R&& value) {
        dispatch([this, value]() mutable {
            _result = Result(value);
            _handle.resume();
        });
    }

    void resume_unsafe() {
        dispatch([this]() { _handle.resume(); });
    }

    void resume_exception(std::exception_ptr &&e) {
        dispatch([this, e]() {
            _result = Result<R>(std::move(e));
            _handle.resume();
        });
    }

    void install_executor(std::shared_ptr<AbstractExecutor> &executor) {
        _executor = executor;
    }

protected:
    std::optional<Result<R>> _result{};

    virtual void after_suspend() {}

    virtual void before_resume() {}

private:
    std::shared_ptr<AbstractExecutor> _executor{nullptr};
    std::coroutine_handle<> _handle{nullptr};

    void dispatch(std::function<void()> &&f) {
        if (_executor) {
            _executor->execute(std::move(f));
        } else {
            f();
        }
    }
};

template<>
struct Awaiter<int> {

    using ResultType = int;

    [[nodiscard]] virtual bool await_ready() = 0;

    virtual void await_suspend(std::coroutine_handle<> handle) {
        this->_handle = handle;
        after_suspend();
    }

    virtual int await_resume() {
        before_resume();
        return _result->get_or_throw();
    }

    void resume(const int& value) {
        dispatch([this, value]() mutable {
            _result = Result<int>(value);
            _handle.resume();
        });
    }

    void resume_unsafe() {
        dispatch([this]() { _handle.resume(); });
    }

    void resume_exception(std::exception_ptr e) {  // 注意这里改为值传递
        dispatch([this, e = std::move(e)]() mutable {  // 添加 mutable
            _result = Result<int>(std::move(e));
            _handle.resume();
        });
    }

    void install_executor(std::shared_ptr<AbstractExecutor> &executor) {
        _executor = executor;
    }

protected:
    std::optional<Result<int>> _result{};

    virtual void after_suspend() {}

    virtual void before_resume() {}

private:
    std::shared_ptr<AbstractExecutor> _executor{nullptr};
    std::coroutine_handle<> _handle{nullptr};

    void dispatch(std::function<void()> &&f) {
        if (_executor) {
            _executor->execute(std::move(f));
        } else {
            f();
        }
    }
};

template<>
struct Awaiter<void> {

    using ResultType = void;

    [[nodiscard]] virtual bool await_ready() = 0;

    void await_suspend(std::coroutine_handle<> handle) {
        this->_handle = handle;
        after_suspend();
    }

    void await_resume() {
        before_resume();
        _result->get_or_throw();
    }

    void resume() {
        if (!_handle || _handle.done())return;
        dispatch([this]() {
            _result = Result<void>();
            _handle.resume();
        });
    }

    void resume_unsafe() {
        dispatch([this]() { _handle.resume(); });
    }

    void resume_exception(std::exception_ptr &&e) {
        dispatch([this, e]() {
            _result = Result<void>(static_cast<std::exception_ptr>(e));
            _handle.resume();
        });
    }

    void install_executor(std::shared_ptr<AbstractExecutor> &executor) {
        _executor = executor;
    }

    virtual void after_suspend() = 0;

    virtual void before_resume() = 0;

private:
    std::optional<Result<void>> _result{};
    std::shared_ptr<AbstractExecutor> _executor{nullptr};
    std::coroutine_handle<> _handle{nullptr};

    void dispatch(std::function<void()> &&f) {
        if (_executor) {
            _executor->execute(std::move(f));
        } else {
            f();
        }
    }
};

#endif //CPPCOROUTINES_TASKS_08_AWAITER_COMMONAWAITER_H_
