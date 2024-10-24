//
// Created by benny on 2022/3/17.
//
#include "Executor.h"
#include "Task.h"
#include "io_utils.h"
#include "Channel.h"

using namespace std::chrono_literals;

//class Producer {
//public:
//    Task<void, LooperExecutor> run(std::shared_ptr<Channel<int>> &channel) {
//        int i = 0;
//        while (channel->is_active()) {
//            debug("send: ", i);
//            co_await channel->write(++i);
//        }
//    }
//
//};


Task<void, LooperExecutor> run(std::shared_ptr<Channel<int>> &channel) {
        int i = 0;
        while (channel->is_active()) {
            debug("send: ", i);
            co_await channel->write(++i);
        }
}


Task<void, LooperExecutor> Consumer(std::shared_ptr<Channel<int>> &channel) {
    while (channel->is_active()) {
        try {
            auto received = co_await channel->read();

            debug("receive: ", received);
        } catch (std::exception &e) {
            debug("exception: ", e.what());
        }
    }

    debug("exit.");
}

Task<void, LooperExecutor> Consumer2(std::shared_ptr<Channel<int>> &channel) {
    while (channel->is_active()) {
        try {
            auto received = co_await channel->read();
            debug("receive2: ", received);
        } catch (std::exception &e) {
            debug("exception2: ", e.what());
        }
    }

    debug("exit.");
}


void test_channel(std::shared_ptr<Channel<int>> &channel) {
//    auto producer = make_unique<Producer>();
//    producer->run(channel);
    auto producer = run(channel);

    auto consumer = Consumer(channel);
    auto consumer2 = Consumer2(channel);

    debug("sleep ...");
    std::this_thread::sleep_for(3s);
    debug("after sleep ...");
}

int main() {
//    auto channel = Channel<int>(10000);
    auto channel = std::make_shared<Channel<int>>(1000);

    test_channel(channel);
    channel->close();
//  test_tasks();
    return 0;
}
