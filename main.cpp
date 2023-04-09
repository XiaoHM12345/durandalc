#include <iostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>


int main() {
    boost::log::add_file_log
            (
                    boost::log::keywords::file_name = "logs/%Y%m%d.log",
                    boost::log::keywords::rotation_size = 10 * 1024 * 1024,
                    boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0),
                    boost::log::keywords::auto_flush = true,
                    boost::log::keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%]: %Message%"
            );

    boost::log::add_common_attributes();

    std::cout << "hello world" << std::endl;


    return 0;
}
