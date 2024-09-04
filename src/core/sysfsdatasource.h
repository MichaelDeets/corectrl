// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "idatasource.h"
#include <filesystem>
#include <fstream>
#include <functional>
#include <spdlog/spdlog.h>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

template<typename T>
concept SysFSDataType = std::is_same_v<T, std::string> ||
                        std::is_same_v<T, std::vector<std::string>>;

/// SysFS data source that allows the transformation of the source data to other
/// types.
///
/// To perform a transformation of the whole contents of the file, use the type
/// std::vector<std::string> as the Input template type parameter. Otherwise, only
/// the first line of the file will be passed to the transformation function.
template<typename Output, SysFSDataType Input = std::string>
class SysFSDataSource : public IDataSource<Output>
{
 public:
  /// Creates a new SysFSDataSource instance using a file as data source and,
  /// optionally, transforming the sysfs data source data to the Output type data
  /// using the custom transform function.
  ///
  /// @param path path to the sysfs file.
  ///
  /// @param transform function to map the sysfs data source data to the
  /// specified Output type. This function is only needed when the Output type is not
  /// a SysFSDataType, requiring a transformation between the types.
  SysFSDataSource(
      std::filesystem::path const &path,
      std::function<void(Input const &, Output &output)> &&transform =
          [](Input const &, Output &) {}) noexcept
  : path_(path.string())
  , transform_(std::move(transform))
  {
    file_.open(path);
    if (!file_.is_open())
      SPDLOG_DEBUG("Cannot open {}", path_.c_str());
  }

  std::string source() const override
  {
    return path_;
  }

  bool read(Output &data) override
  {
    if (file_.is_open()) {
      if constexpr (std::is_same_v<Output, std::string>) {
        readFirstLine(data);
      }
      else if constexpr (std::is_same_v<Output, std::vector<std::string>>) {
        readAll(data);
      }
      else if constexpr (std::is_same_v<Input, std::vector<std::string>>) {
        readAll(fileData_);
        transform_(fileData_, data);
      }
      else if constexpr (std::is_same_v<Input, std::string>) {
        readFirstLine(lineData_);
        transform_(lineData_, data);
      }

      return true;
    }

    return false;
  }

 private:
  /// Reads the first line of the file into data.
  void readFirstLine(std::string &data)
  {
    file_.clear();
    file_.seekg(0);
    std::getline(file_, data);
  }

  /// Reads the file lines into data, reusing existing data elements as
  /// the new data holders when possible.
  void readAll(std::vector<std::string> &data)
  {
    file_.clear();
    file_.seekg(0);

    size_t index = 0;
    while (std::getline(file_, lineData_)) {
      if (data.size() == index)
        data.emplace_back(std::string());

      std::swap(lineData_, data[index++]);
    }
  }

  std::string const path_;
  std::function<void(Input const &, Output &output)> const transform_;

  std::ifstream file_;
  std::string lineData_;
  std::vector<std::string> fileData_;
};
