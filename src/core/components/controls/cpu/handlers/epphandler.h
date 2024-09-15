// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/idatasource.h"
#include "iepphandler.h"
#include <memory>
#include <string>
#include <vector>

class EPPHandler : public IEPPHandler
{
 public:
  EPPHandler(std::unique_ptr<IDataSource<std::string>> &&avaiableEPPHintsDataSource,
             std::vector<std::unique_ptr<IDataSource<std::string>>>
                 &&eppHintDataSources) noexcept;

  std::string const &hint() const override;
  void hint(std::string const &hint) override;

  std::vector<std::string> const &hints() const override;

  virtual void init() override;

  void saveState() override;
  void restoreState(ICommandQueue &ctlCmds) override;

  void reset(ICommandQueue &ctlCmds) override;
  void sync(ICommandQueue &ctlCmds) override;

 private:
  std::unique_ptr<IDataSource<std::string>> const avaiableEPPHintsDataSource_;
  std::vector<std::unique_ptr<IDataSource<std::string>>> const eppHintDataSources_;

  std::vector<std::string> hints_;
  std::string hint_;
  std::string dataSourceEntry_;
};
