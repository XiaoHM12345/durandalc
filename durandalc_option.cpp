//
// Created by durandal on 2023/4/9.
//
#include "durandalc_options.h"

durandalc::details::sqlite3_wrapper::sqlite3_wrapper(const std::string &db_name)
{
    int rc = sqlite3_open(db_name.c_str(), &db_);
    if (rc)
    {
        sqlite3_close(db_);
        BOOST_LOG_TRIVIAL(fatal) << "Failed to open database " << db_name << ": " << sqlite3_errmsg(db_);
    } else {
        BOOST_LOG_TRIVIAL(info) << "success to open database " << db_name;
    }
}

durandalc::details::sqlite3_wrapper::~sqlite3_wrapper() {
    if (db_)
    {
        sqlite3_close(db_);
    }
}

void durandalc::details::sqlite3_wrapper::create_table(const std::string &table_name,
                                                       const std::vector<std::string> &columns) {
    if (!db_)
    {
        BOOST_LOG_TRIVIAL(fatal) << "no database has been opened.";
    }

    std::string sql = "CREATE TABLE IF NOT EXISTS " + table_name + " (";
    for (const auto &column : columns)
    {
        sql += column + ",";
    }
    sql.pop_back(); // Remove the last comma
    sql += ");";

    char *zErrMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, 0, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        sqlite3_free(zErrMsg);
        BOOST_LOG_TRIVIAL(fatal) << "SQL error: " << zErrMsg << std::endl;
    } else {
        BOOST_LOG_TRIVIAL(info) << "success to create table.";
    }
}

bool durandalc::details::sqlite3_wrapper::insert(const std::string &table_name,
                                                 const std::vector<std::pair<std::string, std::string>> &values) {
    if (!db_)
    {
        BOOST_LOG_TRIVIAL(fatal) << "no database has been opened.";
        return false;
    }

    std::string sql = "INSERT INTO " + table_name + " (";
    for (const auto &value : values)
    {
        sql += value.first + ",";
    }
    sql.pop_back(); // Remove the last comma
    sql += ") VALUES (";
    for (const auto &value : values)
    {
        sql += "'" + value.second + "',";
    }
    sql.pop_back(); // Remove the last comma
    sql += ");";

    char *zErrMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, 0, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "success to insert to table.";
        return true;
    }
}

bool durandalc::details::sqlite3_wrapper::update(const std::string &table_name,
                                                 const std::vector<std::pair<std::string, std::string>> &values,
                                                 const std::string &condition) {
    if (!db_)
    {
        BOOST_LOG_TRIVIAL(fatal) << "no database has been opened.";
        return false;
    }

    std::string sql = "UPDATE " + table_name + " SET ";
    for (const auto &value : values)
    {
        sql += value.first + " = '" + value.second + "',";
    }
    sql.pop_back(); // Remove the last comma
    sql += " WHERE " + condition + ";";

    char *zErrMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, 0, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "success to update to table.";
        return true;
    }
}

bool durandalc::details::sqlite3_wrapper::remove(const std::string &table_name, const std::string &condition) {
    if (!db_)
    {
        BOOST_LOG_TRIVIAL(fatal) << "no database has been opened.";
        return false;
    }

    std::string sql = "DELETE FROM " + table_name + " WHERE " + condition + ";";

    char *zErrMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, 0, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "success to delete from table.";
        return true;
    }
}

bool durandalc::details::sqlite3_wrapper::query(const std::string &sql,
                                                const std::function<void(int, char **, char **)> &callback) {
    if (!db_)
    {
        BOOST_LOG_TRIVIAL(fatal) << "no database has been opened.";
        return false;
    }

    char *zErrMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(),
                          [](void *data, int argc, char **argv, char **azColName) -> int {
                              auto &cb = *static_cast<std::function<void(int, char **, char **)> *>(data);
                              cb(argc, argv, azColName);
                              return 0;
                          },
                          static_cast<void *>(const_cast<std::function<void(int, char **, char **)> *>(&callback)), &zErrMsg);

    if (rc != SQLITE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return false;
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "success to query from table.";
        return true;
    }
}

bool durandalc::details::sqlite3_wrapper::getAllRecords(const std::string &table_name,
                                                        std::vector<std::map<std::string, std::string>> &result) {
    if (!db_)
    {
        BOOST_LOG_TRIVIAL(fatal) << "no database has been opened.";
        return false;
    }

    std::string sql = "SELECT * FROM " + table_name + ";";

    return query(sql, [&result](int argc, char **argv, char **azColName) {
        std::map<std::string, std::string> record;
        for (int i = 0; i < argc; ++i)
        {
            record[azColName[i]] = argv[i] ? argv[i] : "";
        }
        result.push_back(record);
    });
}




