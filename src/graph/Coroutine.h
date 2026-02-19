#ifndef COROUTINE_H
#define COROUTINE_H

#include <coroutine>
#include <exception>
#include <functional>
#include <list>
#include <Arduino.h>

// 1. 定义协程的返回类型 Task
struct Task {
    struct promise_type {
        // 存储等待者（谁在 co_await 我？）
        std::coroutine_handle<> continuation = nullptr;

        Task get_return_object() { return Task{std::coroutine_handle<promise_type>::from_promise(*this)}; }

        // 初始不挂起，创建即运行
        std::suspend_never initial_suspend() { return {}; }

        // 核心修复：自定义 final_suspend
        // 在协程结束时，唤醒等待者，然后自我销毁
        struct FinalAwaiter {
            bool await_ready() const noexcept { return false; }

            void await_suspend(std::coroutine_handle<promise_type> h) noexcept {
                auto& promise = h.promise();
                if (promise.continuation) {
                    promise.continuation.resume();
                }
                h.destroy();
            }
            void await_resume() noexcept {}
        };

        FinalAwaiter final_suspend() noexcept { return {}; }

        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    // 构造函数
    explicit Task(std::coroutine_handle<promise_type> h) : handle(h) {}

    // 析构函数：由于 promise 里的 final_suspend 处理了销毁，这里不需要手动 destroy
    // 除非我们想支持取消功能，但在嵌入式简单场景下，自动销毁最安全防止泄漏
    ~Task() {}

    // --- 核心修复：实现 Awaitable 接口 ---

    // 1. 是否准备好了？(如果是 false，就会调用 await_suspend)
    bool await_ready() const { return false; }

    // 2. 挂起时的操作：记录谁在等待我
    void await_suspend(std::coroutine_handle<> waiting_coro) {
        // 将等待者的句柄存入当前任务的 promise 中
        if (handle && !handle.done()) {
            handle.promise().continuation = waiting_coro;
        }
    }

    // 3. 恢复时的操作：无返回值
    void await_resume() {}
};

// 2. 定义调度器 (Scheduler) - 保持不变
class Scheduler {
public:
    using TaskHandle = std::function<bool()>;

    static Scheduler& getInstance() {
        static Scheduler instance;
        return instance;
    }

    void addTask(TaskHandle task) {
        tasks.push_back(task);
    }

    void run() {
        auto it = tasks.begin();
        while (it != tasks.end()) {
            if ((*it)()) {
                it = tasks.erase(it);
            } else {
                ++it;
            }
        }
    }
    bool isEmpty() const {
        return tasks.empty();
    }
private:
    std::list<TaskHandle> tasks;
};

// 3. 实现 async_sleep (Awaitable) - 保持不变
struct AsyncSleep {
    unsigned long duration_ms;
    unsigned long start_time;

    AsyncSleep(unsigned long ms) : duration_ms(ms), start_time(0) {}

    bool await_ready() const { return false; }

    void await_suspend(std::coroutine_handle<> h) {
        start_time = millis();
        Scheduler::getInstance().addTask([this, h]() mutable -> bool {
            if (millis() - start_time >= duration_ms) {
                h.resume();
                return true;
            }
            return false;
        });
    }

    void await_resume() {}
};

inline AsyncSleep async_sleep(unsigned long ms) {
    return AsyncSleep(ms);
}

#endif // COROUTINE_H