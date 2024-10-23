//
// Created by benny on 2022/3/21.
//

#ifndef CPPCOROUTINES_TASKS_07_CHANNEL_CHANNELAWAITER_H_
#define CPPCOROUTINES_TASKS_07_CHANNEL_CHANNELAWAITER_H_

#include "coroutine_common.h"
#include "CommonAwaiter.h"
#include "utility"

using namespace std;
template<typename ValueType>
struct Channel;

template<typename ValueType>
struct WriterAwaiter : public Awaiter<void> {
    shared_ptr<Channel<ValueType>> channel;
    ValueType _value;


    WriterAwaiter(const shared_ptr<Channel<ValueType>> &channel, const ValueType& value)
            : channel(channel), _value(value) {}

    WriterAwaiter(WriterAwaiter &&other) noexcept
            : Awaiter(other),
              channel(std::exchange(other.channel, nullptr)),
              _value(std::move(other._value)) {}

    [[nodiscard]] bool await_ready() override { return false; }

    void after_suspend() override {
        channel->try_push_writer(this);
    }

    void before_resume() override {
        channel->check_closed();
        channel = nullptr;
    }

    ~WriterAwaiter() {
        if (channel) channel->remove_writer(this);
    }
};

template<typename ValueType>
struct ReaderAwaiter : public Awaiter<ValueType> {
    shared_ptr<Channel<ValueType>> channel;
    ValueType *p_value = nullptr;

    explicit ReaderAwaiter(const shared_ptr<Channel<ValueType>>& channel) : Awaiter<ValueType>(), channel(channel) {}

    ReaderAwaiter(ReaderAwaiter &&other) noexcept
            : Awaiter<ValueType>(other),
              channel(std::exchange(other.channel, nullptr)),
              p_value(std::exchange(other.p_value, nullptr)) {}

    [[nodiscard]] bool await_ready() override { return false; }


    void after_suspend() override {
        channel->try_push_reader(this);
    }

    void before_resume() override {
        channel->check_closed();
        if (p_value) {
            *p_value = std::move(this->_result->get_or_throw());
        }
        channel = nullptr;
    }

    ~ReaderAwaiter() {
        if (channel) channel->remove_reader(this);
    }
};

#endif //CPPCOROUTINES_TASKS_07_CHANNEL_CHANNELAWAITER_H_
