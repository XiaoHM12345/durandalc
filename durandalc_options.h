//
// Created by durandal on 2023/4/8.
//

#ifndef DURANDALC_DURANDALC_OPTIONS_H
#define DURANDALC_DURANDALC_OPTIONS_H

#include <boost/noncopyable.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <vector>
#include <string>
#include <mutex>
#include <map>
#include <algorithm>
#include <functional>

#include <sqlite3.h>

namespace durandalc {

//#define BOOST_LOG_TRIVIAL(info) INFO;
//#define BOOST_LOG_TRIVIAL(trace) TRACE;
//#define BOOST_LOG_TRIVIAL(error) ERROR;
//#define BOOST_LOG_TRIVIAL(fatal) FATAL;
//#define BOOST_LOG_TRIVIAL(debug) DEBUG;
//#define BOOST_LOG_TRIVIAL(warning) WARNING;

namespace details {

class sqlite3_wrapper: boost::noncopyable
{
public:
    sqlite3_wrapper(const std::string& db_name);
    ~sqlite3_wrapper();

    void create_table(const std::string &table_name, const std::vector<std::string> &columns);
    bool insert(const std::string &table_name, const std::vector<std::pair<std::string, std::string>> &values);
    bool update(const std::string &table_name, const std::vector<std::pair<std::string, std::string>> &values, const std::string &condition);
    bool remove(const std::string &table_name, const std::string &condition);
    bool query(const std::string &sql, const std::function<void(int, char **, char **)> &callback);
    bool getAllRecords(const std::string &table_name, std::vector<std::map<std::string, std::string>> &result);
private:
    sqlite3 *db_;
};


class file_buffer : boost::noncopyable
{
public:
    file_buffer() = default;
    void get(std::vector<std::map<std::string, std::string>>& rf);
    bool get(const std::string& filename, std::map<std::string, std::string>& rf);
    void set(const std::vector<std::map<std::string, std::string>>& rf);
    void set(const std::map<std::string, std::string>& rf);
private:
    std::vector<std::map<std::string, std::string>> files_;
    std::mutex mu_;
};

}

}


#endif //DURANDALC_DURANDALC_OPTIONS_H
