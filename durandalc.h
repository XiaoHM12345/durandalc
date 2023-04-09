//
// Created by durandal on 2023/4/8.
//

#ifndef DURANDALC_DURANDALC_H
#define DURANDALC_DURANDALC_H

#include <boost/noncopyable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/thread_pool.hpp>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <string>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <thread>
#include <queue>

// durandalc is not a thread safe class
// Only use in a main thread to control http connection \
// and control other IO thread

namespace durandalc {

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;
using std::string;
using crstring = const string&;

namespace details {
std::atomic<uint> THREAD_POOL_CAPACITY = 5;

class barrier : boost::noncopyable {
public:
    barrier(int num)
      : count_(num)
    { }

    void wait() {
        std::unique_lock<std::mutex> lock(mu_);
        while (count_ > 0)
            cond_.wait(lock);
    }

    void countDown() {
        std::unique_lock<std::mutex> lock(mu_);
        count_--;
        if (count_ <= 0)
            cond_.notify_all();
    }

private:
    std::condition_variable cond_;
    std::mutex mu_; //guard count_
    int count_;
};

class thread_pool : boost::noncopyable {
public:
    thread_pool(const std::string& name, int maxThread)
            : name_(name),
              running_(false),
              max_thread_num_(maxThread)
    {
        for (int index = 0; index < max_thread_num_; index++)
            threads_.emplace_back();
    }

    ~thread_pool()
    {
        stop();
        for (auto& t : threads_)
            t.join();
        printf("thread pool %s destroyed\n", name_.c_str());
    }

    void push(std::function<void()> task);

    void start()
    {
        running_ = true;
        for (auto& t : threads_) {
            t = std::thread([this] { run_in_thread(); });
        }
    }

    void stop()
    {
        std::unique_lock<std::mutex> lock(mu_);
        running_ = false;
        notEmptyCond.notify_all();
        notFullCond.notify_all();
        printf("thread pool %s stoped\n", name_.c_str());
    }

private:
    std::function<void()> take();
    void run_in_thread();
    std::mutex mu_; //guard thread_;
    std::queue<std::function<void()>> tasks_;
    std::vector<std::thread> threads_;
    std::condition_variable notFullCond, notEmptyCond;
    bool running_;
    std::string name_;
    int max_thread_num_;
};

}

namespace {
http::request<http::vector_body<char>> generate_chunk_request(const std::string& file_path, const std::string& host,
                                    const std::string& port, const std::string& target,
                                    const std::vector<char>& data, size_t chunk_num)
{
    http::request<http::vector_body<char>> req(http::verb::post, target, 11);
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::content_type, "application/octet-stream");
    req.set(http::field::content_length, std::to_string(data.size()));
    req.set("X-Chunk-Num", std::to_string(chunk_num));
    req.body() = data;
    return req;
}

void send_chunk(asio::io_context& ioc, const std::string& host, const std::string& port,
                const http::request<http::vector_body<char>>& request_body, size_t chunk_num)
{
    tcp::resolver resolver(ioc);
    tcp::socket socket(ioc);
    boost::system::error_code ec;

    auto const remote_ip_port = resolver.resolve(host, port, ec);
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "Failed to resolve ip and port from " << host << ":" << port << ", the reason is : " << ec.message();
        return;
    } else {
        BOOST_LOG_TRIVIAL(trace) << "Success to resolve " << host << ":" << port;
    }

    asio::connect(socket, remote_ip_port.begin(), remote_ip_port.end(), ec);
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "Failed to connect to " << host << ":" << port << ", the reason is : " << ec.message();
        return;
    } else {
        BOOST_LOG_TRIVIAL(info) << "Success to connect to " << host << ":" << port;
    }

    // TODO: add retry code
    http::write(socket, request_body, ec);
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "Failed to write to " << host << ":" << port << ", the reason is : " << ec.message();
        return;
    } else {
        BOOST_LOG_TRIVIAL(trace) << "Success to write to " << host << ":" << port;
    }

    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    http::read(socket, buffer, res, ec);
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "Failed to read from " << host << ":" << port << ", the reason is : " << ec.message();
        return;
    } else {
        BOOST_LOG_TRIVIAL(trace) << "Success to read from " << host << ":" << port;
    }

    if (res.result_int() >= 200 && res.result_int() < 300) {
        BOOST_LOG_TRIVIAL(trace) << "File upload succeeded. Response: " << res;
    } else if (res.result_int() >= 400 && res.result_int() < 500) {
        BOOST_LOG_TRIVIAL(error) << "File upload failed due to a client error. Response: " << res;
    } else if (res.result_int() >= 500 && res.result_int() < 600) {
        BOOST_LOG_TRIVIAL(error) << "File upload failed due to a server error. Response: " << res;
    } else {
        BOOST_LOG_TRIVIAL(error) << "File upload failed with an unexpected response. Response: " << res;
    }
}

}

class durandalc : boost::noncopyable {
public:

    durandalc(crstring host, uint16_t port, crstring target, int version);

private:

    bool running_;
    std::mutex mu_;


    details::thread_pool pool_;

    // io operation
    asio::io_context ioc_; // global io operation
    tcp::resolver resolver_; //local io
    beast::tcp_stream stream_; // local io

    // http info
    string local_host_;
    string remote_host_;
    uint16_t remote_port_;
    string target_;
    int version_;
};

}
#endif //DURANDALC_DURANDALC_H



