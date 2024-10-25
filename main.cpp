//
// Created by benny on 2022/3/17.
//
#include "Executor.h"
#include "Task.h"
#include "io_utils.h"
#include "Channel.h"

#include <utility>
using namespace std::chrono_literals;


static inline shared_ptr<Channel<int>> channel;
static inline int i=0;


void producer() {
    while (true) {
        channel->writer_message(i);
    }
}

Task<void, LooperExecutor> Consumer() {
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

Task<void, LooperExecutor> Consumer2() {
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


void test_channel() {
    std::thread p(producer);

    auto consumer = Consumer();
    auto consumer2 = Consumer2();

    debug("sleep ...");
    std::this_thread::sleep_for(3s);
    debug("after sleep ...");
}

int main() {
//    auto channel = Channel<int>(10000);
    channel = std::make_shared<Channel<int>>(1000);

    test_channel();
    channel->close();
//  test_tasks();
    return 0;
}
