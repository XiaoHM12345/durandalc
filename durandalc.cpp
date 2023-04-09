//
// Created by durandal on 2023/4/8.
//

#include "durandalc.h"

durandalc::durandalc::durandalc(const std::string &host, uint16_t port, const std::string& target, int version)
  : ioc_(),
    resolver_(ioc_),
    stream_(ioc_),
    remote_port_(port),
    remote_host_(host),
    local_host_(string("0, 0, 0, 0")),
    running_(false),
    mu_(),
    pool_(string("false"), details::THREAD_POOL_CAPACITY),
    target_(target),
    version_(version)
{
    auto const remote_ip = resolver_.resolve(remote_host_, std::to_string(remote_port_));

}

void durandalc::durandalc::run() {

}


void durandalc::details::thread_pool::push(std::function<void()> task) {
    std::unique_lock<std::mutex> lock(mu_);
    while (running_ && tasks_.size() >= max_thread_num_) {
        notFullCond.wait(lock);
    }
    tasks_.push(std::move(task));
    notEmptyCond.notify_one();
}

std::function<void()> durandalc::details::thread_pool::take() {
    std::unique_lock<std::mutex> lock(mu_);
    while (running_ && tasks_.empty()) {
        notEmptyCond.wait(lock);
    }
    std::function<void()> ret;
    if (!tasks_.empty()) {
        ret = tasks_.front();
        tasks_.pop();
        notFullCond.notify_one();
    }
    return ret;
}

void durandalc::details::thread_pool::run_in_thread() {
    while (running_) {
        auto newTask = take();
        if (newTask)
            newTask();
    }
}
