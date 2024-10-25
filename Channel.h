//
// Created by benny on 2022/3/21.
//

#ifndef CPPCOROUTINES_TASKS_07_CHANNEL_CHANNEL_H_
#define CPPCOROUTINES_TASKS_07_CHANNEL_CHANNEL_H_

#include "coroutine_common.h"
#include "ChannelAwaiter.h"
#include <exception>
#include <utility>

template<typename ValueType>
struct Channel: public std::enable_shared_from_this<Channel<ValueType>> {

    struct ChannelClosedException : std::exception {
        [[nodiscard]] const char *what() const noexcept override {
            return "Channel is closed.";
        }
    };

    void check_closed() {
        if (!_is_active.load(std::memory_order_relaxed)) {
            throw ChannelClosedException();
        }
    }

    void try_push_reader(ReaderAwaiter<ValueType> *reader_awaiter) {
        std::unique_lock lock(channel_lock);
        check_closed();

        if (!buffer.empty()) {
            auto value = std::move(buffer.front());
            buffer.pop();

            if (!writer_list.empty()) {
                auto& writer = writer_list.front();
                writer_list.pop_front();
                buffer.push(std::move(writer->_value));
                lock.unlock();
                writer->resume();
            } else {
                lock.unlock();
            }

            reader_awaiter->resume(std::move(value));
            return;
        }

        if (!writer_list.empty()) {
            auto&& writer = writer_list.front();
            writer_list.pop_front();
            lock.unlock();

            reader_awaiter->resume(std::move(writer->_value));
            writer->resume();
            return;
        }

        reader_list.push_back(reader_awaiter);
    }

    void try_push_writer(WriterAwaiter<ValueType> *writer_awaiter) {
        std::unique_lock lock(channel_lock);
        check_closed();

        // 优先处理等待的reader
        if (!reader_list.empty()) {
            auto&& reader = reader_list.front();
            reader_list.pop_front();
            lock.unlock();

            reader->resume(std::move(writer_awaiter->_value));
            writer_awaiter->resume();
            return;
        }

        // 如果buffer未满，写入buffer
        if (buffer.size() < buffer_capacity) {
            buffer.push(std::move(writer_awaiter->_value));
            lock.unlock();
            writer_awaiter->resume();
            return;
        }

        // buffer已满，将writer加入等待队列
        writer_list.push_back(writer_awaiter);
    }

    void remove_writer(WriterAwaiter<ValueType> *writer_awaiter) {
        std::lock_guard lock(channel_lock);
        auto size = writer_list.remove(writer_awaiter);
        debug("remove writer ", size);
    }

    void remove_reader(ReaderAwaiter<ValueType> *reader_awaiter) {
        std::lock_guard lock(channel_lock);
        auto size = reader_list.remove(reader_awaiter);
        debug("remove reader ", size);
    }

    // 如果 value 后续用，可以直接在coawait处move
    auto write(const ValueType& value) {
        check_closed();
        return WriterAwaiter<ValueType>{this->shared_from_this(), value};
    }

    auto read() {
        check_closed();
        return ReaderAwaiter<ValueType>{this->shared_from_this()};
    }


    void close() {
        bool expect = true;
        if (_is_active.compare_exchange_strong(expect, false, std::memory_order_relaxed)) {
            clean_up();
        }
    }

    [[nodiscard]] bool is_active() const {
        return _is_active.load(std::memory_order_relaxed);
    }

    explicit Channel(int capacity = 0) : buffer_capacity(capacity) {
        _is_active.store(true, std::memory_order_relaxed);
    }

    Channel(Channel &&channel) = delete;

    Channel(const Channel &) = delete;

    Channel &operator=(const Channel &) = delete;

    Channel &operator=(Channel &&) = delete;

    ~Channel() {
        close();
    }

private:
    int buffer_capacity;
    std::queue<ValueType> buffer;
    std::list<WriterAwaiter<ValueType> *> writer_list;
    std::list<ReaderAwaiter<ValueType> *> reader_list;

    std::atomic<bool> _is_active;

    std::mutex channel_lock;
    std::condition_variable channel_condition;

    void clean_up() {
        std::lock_guard lock(channel_lock);

        for (auto writer: writer_list) {
            writer->resume();
        }
        writer_list.clear();

        for (auto reader: reader_list) {
            reader->resume_unsafe();
        }
        reader_list.clear();

        while (!buffer.empty()) {
            buffer.pop();
        }
    }
};

template<>
class Channel<int> : public std::enable_shared_from_this<Channel<int>> {
public:

    // 自定义异常类，用于表示通道已关闭
    struct SipChannelClosedException : std::exception {
        [[nodiscard]] const char *what() const noexcept override {
            return "SipChannel is closed.";
        }
    };

    void check_closed() {
        if (!_is_active.load(std::memory_order_relaxed)) {
            throw SipChannelClosedException();
        }
    }

    // 构造函数，指定缓冲区容量
    explicit Channel(unsigned long capacity = 16) : buffer_capacity(capacity) {
        _is_active.store(true, std::memory_order_relaxed);
    }


    // 尝试将读取器（reader）推入通道
    void try_push_reader(ReaderAwaiter<int> *reader_awaiter) {
        std::unique_lock lock(channel_lock);
        check_closed();

        // 如果缓冲区不为空，直接从缓冲区读取数据
        if (!buffer.empty()) {
            auto value = buffer.front();
            buffer.pop();
            lock.unlock();
            reader_awaiter->resume(value);
            return;
        }

        // 如果没有可用数据，将读取器加入等待列表
        reader_list.push_back(reader_awaiter);
    }

    // 从等待列表中移除读取器
    void remove_reader(ReaderAwaiter<int> *reader_awaiter) {
        std::lock_guard lock(channel_lock);
        reader_list.remove(reader_awaiter);
    }

    void writer_message(int& value) {
        std::unique_lock lock(channel_lock);
        check_closed();
        // 优先处理等待的reader
        if (!reader_list.empty()) {
            value++;
            auto reader = reader_list.front();
            reader_list.pop_front();
            lock.unlock();
            reader->resume(value);
            return;
        }

        // 如果buffer未满，写入buffer
        if (buffer.size() < buffer_capacity) {
            value++;
            buffer.push(value);
            lock.unlock();
            return;
        }
    }

    // 从通道读取数据
    auto read() {
        check_closed();
        return ReaderAwaiter<int>{this->shared_from_this()};
    }

    // 关闭通道
    void close() {
        bool expect = true;
        if (_is_active.compare_exchange_strong(expect, false, std::memory_order_relaxed)) {
            clean_up();
        }
    }

    // 检查通道是否活跃
    [[nodiscard]] bool is_active() const noexcept {
        return _is_active.load(std::memory_order_relaxed);
    }

    // 禁用移动构造函数
    Channel(Channel &&) = delete;

    // 禁用拷贝构造函数
    Channel(const Channel &) = delete;

    // 禁用赋值运算符
    Channel &operator=(const Channel &) = delete;

    Channel &operator=(Channel &&) = delete;

    // 析构函数，确保通道被关闭
    ~Channel() {
        close();
    }

private:
    unsigned long buffer_capacity;  // 缓冲区容量
    std::queue<int> buffer;  // 数据缓冲区
    std::list<ReaderAwaiter<int> *> reader_list;  // 等待的读取器列表

    std::atomic<bool> _is_active;  // 通道是否活跃的原子标志

    std::mutex channel_lock;  // 通道锁
    std::condition_variable channel_condition;

    // 清理函数，在关闭通道时调用
    void clean_up() {
        std::lock_guard lock(channel_lock);

        for (auto reader: reader_list) {
            reader->resume_unsafe();
        }
        reader_list.clear();

        while (!buffer.empty()) {
            buffer.pop();
        }
    }
};

#endif //CPPCOROUTINES_TASKS_07_CHANNEL_CHANNEL_H_
