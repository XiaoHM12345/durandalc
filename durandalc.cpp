//
// Created by durandal on 2023/4/8.
//

#include "durandalc.h"
#include "durandalc_options.h"

durandalc::durandalc::durandalc(const std::string &host, uint16_t port, const std::string& target, int version)
  : ioc_(),
    resolver_(ioc_),
    stream_(ioc_),
    remote_port_(port),
    remote_host_(host),
    local_host_(string("0.0.0.0")),
    running_(false),
    mu_(),
    pool_(string("false"), details::THREAD_POOL_CAPACITY),
    target_(target),
    version_(version),
    p_sqlite3_wrapper_(std::make_shared<details::sqlite3_wrapper>(DATABASE_PATH)),
    p_file_buffer_(std::make_shared<details::file_buffer>())
{
    std::error_code ec;
    std::vector<std::string> columns = {
            "ID INTEGER PRIMARY KEY AUTOINCREMENT",
            "FILENAME TEXT NOT NULL UNIQUE",
            "MD5 TEXT NOT NULL",
            "FILE_EXISTS INTEGER NOT NULL"
        };

    (*p_sqlite3_wrapper_).create_table("file_info", columns);

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

void durandalc::durandalc::run_http_in_thread() {

}

void durandalc::durandalc::run_update_files_in_thread() {
    while (true) {
        std::vector<std::string> result;
        get_files_in_directory(BACKUP_FILE_PATH, result);
        std::vector<std::map<std::string, std::string>> db_result;
        (*p_sqlite3_wrapper_).getAllRecords("file_info", db_result);
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}
