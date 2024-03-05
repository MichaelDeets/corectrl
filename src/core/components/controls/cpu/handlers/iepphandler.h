// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/components/controls/idatasourcehandler.h"
#include <string>
#include <utility>
#include <vector>

class IEPPHandler : public IDataSourceHandler
{
 public:
  virtual std::vector<std::string> const &hints() const = 0;

  virtual std::string const &hint() const = 0;
  virtual void hint(std::string const &hint) = 0;

  virtual ~IEPPHandler() = default;
};
