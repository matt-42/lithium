#pragma once

#include <mysql.h>
#include <memory>

/**
 * @brief Store the data that mysql_statement holds.
 *
 */
struct mysql_statement_data : std::enable_shared_from_this<mysql_statement_data> {

  MYSQL_STMT* stmt_ = nullptr;
  int num_fields_ = -1;
  MYSQL_RES* metadata_ = nullptr;
  MYSQL_FIELD* fields_ = nullptr;

  mysql_statement_data(MYSQL_STMT* stmt) {
    stmt_ = stmt;
    metadata_ = mysql_stmt_result_metadata(stmt_);
    if (metadata_) {
      fields_ = mysql_fetch_fields(metadata_);
      num_fields_ = mysql_num_fields(metadata_);
    }
  }

  ~mysql_statement_data() {
    if (metadata_)
      mysql_free_result(metadata_);
    mysql_stmt_free_result(stmt_);
    if (mysql_stmt_close(stmt_))
      std::cerr << "Error: could not free mysql statement" << std::endl;
  }
};
