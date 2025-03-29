// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/components/controls/control.h"
#include "core/idatasource.h"
#include <memory>
#include <string>
#include <string_view>
#include <units.h>
#include <utility>
#include <vector>

namespace AMD {

/// Overdrive frequency offset control.
///
/// A frequency overdrive control has an identifier and a frequency offset.
class PMFreqOffset : public Control
{
 public:
  static constexpr std::string_view ItemID{"AMD_PM_FREQ_OFFSET"};

  class Importer : public IControl::Importer
  {
   public:
    virtual units::frequency::megahertz_t providePMFreqOffsetValue() const = 0;
  };

  class Exporter : public IControl::Exporter
  {
   public:
    virtual void takePMFreqOffsetControlName(std::string const &name) = 0;
    virtual void takePMFreqOffsetRange(units::frequency::megahertz_t min,
                                       units::frequency::megahertz_t max) = 0;
    virtual void takePMFreqOffsetValue(units::frequency::megahertz_t value) = 0;
  };

  using Range =
      std::pair<units::frequency::megahertz_t, units::frequency::megahertz_t>;

  PMFreqOffset(std::string &&controlName, std::string &&controlCmdId,
               AMD::PMFreqOffset::Range &&range,
               std::unique_ptr<IDataSource<std::vector<std::string>>>
                   &&ppOdClkVoltDataSource) noexcept;

  void preInit(ICommandQueue &ctlCmds) final override;
  void postInit(ICommandQueue &ctlCmds) final override;
  void init() final override;

  std::string const &ID() const final override;
  std::string const &instanceID() const final override;

 protected:
  void importControl(IControl::Importer &i) final override;
  void exportControl(IControl::Exporter &e) const final override;

  void cleanControl(ICommandQueue &ctlCmds) override;
  void syncControl(ICommandQueue &ctlCmds) override;

  std::string const &controlName() const;
  std::string const &controlCmdId() const;

  std::pair<units::frequency::megahertz_t, units::frequency::megahertz_t> const &
  range() const;

  units::frequency::megahertz_t offset() const;
  void offset(units::frequency::megahertz_t freq);

  std::string ppOdClkVoltCmd(units::frequency::megahertz_t freq) const;

 private:
  std::string const id_;
  std::string const controlName_;
  std::string const controlCmdId_;

  std::unique_ptr<IDataSource<std::vector<std::string>>> const ppOdClkVoltDataSource_;
  std::vector<std::string> ppOdClkVoltLines_;

  units::frequency::megahertz_t preInitOffset_;
  units::frequency::megahertz_t offset_;
  Range range_;
};

} // namespace AMD
