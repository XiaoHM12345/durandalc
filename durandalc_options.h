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
#include <functional>

#include <sqlite3.h>

namespace durandalc {

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

    struct file_info_t {
        std::string filename_;
        std::string md5_;
        bool exsit_;
    };

    file_buffer() = default;
    std::map<std::string, file_info_t> get_all_file();

private:
    std::map<std::string, file_info_t> files_;
    std::mutex mu_;
};

}

}


#endif //DURANDALC_DURANDALC_OPTIONS_H
