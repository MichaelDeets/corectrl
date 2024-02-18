// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "common/fileutils.h"
#include "core/idatasource.h"
#include <spdlog/spdlog.h>
#include <string>

class SWInfoKernelDataSource : public IDataSource<std::string>
{
 public:
  std::string source() const override
  {
    return "/proc/version";
  }

  bool read(std::string &data) override
  {
    auto const lines = Utils::File::readFileLines(source());
    if (!lines.empty()) {
      data = lines.front();
      return true;
    }

    SPDLOG_WARN("Cannot retrieve kernel version");
    return false;
  }
};
