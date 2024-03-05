// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/components/controls/control.h"
#include "core/components/controls/cpu/handlers/iepphandler.h"
#include "core/idatasource.h"
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class CPUFreq : public Control
{
 public:
  static constexpr std::string_view ItemID{"CPU_CPUFREQ"};

  class Importer : public IControl::Importer
  {
   public:
    virtual std::string const &provideCPUFreqScalingGovernor() const = 0;
    virtual std::optional<std::string> const &provideCPUFreqEPPHint() const = 0;
  };

  class Exporter : public IControl::Exporter
  {
   public:
    virtual void takeCPUFreqScalingGovernor(std::string const &governor) = 0;
    virtual void
    takeCPUFreqScalingGovernors(std::vector<std::string> const &governors) = 0;
    virtual void takeCPUFreqEPPHint(std::optional<std::string> const &hint) = 0;
    virtual void
    takeCPUFreqEPPHints(std::optional<std::vector<std::string>> const &hints) = 0;
  };

  CPUFreq(std::vector<std::string> &&scalingGovernors,
          std::string const &defaultGovernor,
          std::vector<std::unique_ptr<IDataSource<std::string>>>
              &&scalingGovernorDataSources,
          std::unique_ptr<IEPPHandler> &&eppHandler = nullptr) noexcept;

  void preInit(ICommandQueue &ctlCmds) final override;
  void postInit(ICommandQueue &ctlCmds) final override;
  void init() final override;

  std::string const &ID() const final override;

 protected:
  void importControl(IControl::Importer &i) final override;
  void exportControl(IControl::Exporter &e) const final override;

  void cleanControl(ICommandQueue &ctlCmds) final override;
  void syncControl(ICommandQueue &ctlCmds) final override;

  std::string const &scalingGovernor() const;
  void scalingGovernor(std::string const &governor);
  std::vector<std::string> const &scalingGovernors() const;

  std::optional<std::string> eppHint() const;
  std::optional<std::vector<std::string>> eppHints() const;

 private:
  std::string const id_;
  std::vector<std::string> const scalingGovernors_;
  std::vector<std::unique_ptr<IDataSource<std::string>>> const
      scalingGovernorDataSources_;
  std::unique_ptr<IEPPHandler> eppHandler_;

  std::string scalingGovernor_;
  std::string dataSourceEntry_;
};
