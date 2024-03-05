// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/idatasource.h"
#include "iepphandler.h"
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class EPPHandler : public IEPPHandler
{
 public:
  EPPHandler(std::vector<std::string> &&eppHints,
             std::vector<std::unique_ptr<IDataSource<std::string>>>
                 &&eppHintDataSources) noexcept;

  std::string const &hint() const override;
  void hint(std::string const &hint) override;

  std::vector<std::string> const &hints() const override;

  void saveState() override;
  void restoreState(ICommandQueue &ctlCmds) override;

  void reset(ICommandQueue &ctlCmds) override;
  void sync(ICommandQueue &ctlCmds) override;

 private:
  std::vector<std::string> const hints_;
  std::vector<std::unique_ptr<IDataSource<std::string>>> const eppHintDataSources_;

  std::string hint_;
  std::string dataSourceEntry_;
};
