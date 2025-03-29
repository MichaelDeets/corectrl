// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Juan Palacios <jpalaciosdev@gmail.com>

#include "pmfreqoffset.h"

#include "core/components/amdutils.h"
#include "core/icommandqueue.h"
#include <algorithm>

AMD::PMFreqOffset::PMFreqOffset(
    std::string &&controlName, std::string &&controlCmdId,
    AMD::PMFreqOffset::Range &&range,
    std::unique_ptr<IDataSource<std::vector<std::string>>>
        &&ppOdClkVoltDataSource) noexcept
: Control(true)
, id_(AMD::PMFreqOffset::ItemID)
, controlName_(std::move(controlName))
, controlCmdId_(std::move(controlCmdId))
, ppOdClkVoltDataSource_(std::move(ppOdClkVoltDataSource))
, range_(std::move(range))
{
}

void AMD::PMFreqOffset::preInit(ICommandQueue &)
{
  if (!ppOdClkVoltDataSource_->read(ppOdClkVoltLines_))
    return;

  preInitOffset_ =
      Utils::AMD::parseOverdriveClkOffset(controlName(), ppOdClkVoltLines_).value();
}

void AMD::PMFreqOffset::postInit(ICommandQueue &ctlCmds)
{
  ctlCmds.add({ppOdClkVoltDataSource_->source(), ppOdClkVoltCmd(preInitOffset_)});
}

void AMD::PMFreqOffset::init()
{
  if (!ppOdClkVoltDataSource_->read(ppOdClkVoltLines_))
    return;

  auto value = Utils::AMD::parseOverdriveClkOffset(controlName(),
                                                   ppOdClkVoltLines_);
  offset_ = std::clamp(*value, range_.first, range_.second);
}

std::string const &AMD::PMFreqOffset::ID() const
{
  return id_;
}

std::string const &AMD::PMFreqOffset::instanceID() const
{
  return controlName();
}

void AMD::PMFreqOffset::importControl(IControl::Importer &i)
{
  auto &importer = dynamic_cast<AMD::PMFreqOffset::Importer &>(i);
  offset(importer.providePMFreqOffsetValue());
}

void AMD::PMFreqOffset::exportControl(IControl::Exporter &e) const
{
  auto &exporter = dynamic_cast<AMD::PMFreqOffset::Exporter &>(e);

  auto [min, max] = range();
  exporter.takePMFreqOffsetControlName(controlName());
  exporter.takePMFreqOffsetRange(min, max);
  exporter.takePMFreqOffsetValue(offset());
}

void AMD::PMFreqOffset::cleanControl(ICommandQueue &)
{
}

void AMD::PMFreqOffset::syncControl(ICommandQueue &ctlCmds)
{
  if (ppOdClkVoltDataSource_->read(ppOdClkVoltLines_)) {

    auto value = Utils::AMD::parseOverdriveClkOffset(controlName(),
                                                     ppOdClkVoltLines_)
                     .value();
    if (value != offset())
      ctlCmds.add({ppOdClkVoltDataSource_->source(), ppOdClkVoltCmd(offset())});
  }
}

std::string const &AMD::PMFreqOffset::controlName() const
{
  return controlName_;
}

std::string const &AMD::PMFreqOffset::controlCmdId() const
{
  return controlCmdId_;
}

std::pair<units::frequency::megahertz_t, units::frequency::megahertz_t> const &
AMD::PMFreqOffset::range() const
{
  return range_;
}

units::frequency::megahertz_t AMD::PMFreqOffset::offset() const
{
  return offset_;
}

void AMD::PMFreqOffset::offset(units::frequency::megahertz_t freq)
{
  offset_ = std::clamp(freq, range_.first, range_.second);
}

std::string
AMD::PMFreqOffset::ppOdClkVoltCmd(units::frequency::megahertz_t freq) const
{
  std::string cmd;
  cmd.reserve(16);
  cmd.append(controlCmdId()).append(" ").append(std::to_string(freq.to<int>()));
  return cmd;
}
