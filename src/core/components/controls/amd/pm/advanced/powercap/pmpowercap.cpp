// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "pmpowercap.h"

#include "core/icommandqueue.h"
#include <algorithm>
#include <utility>

AMD::PMPowerCap::PMPowerCap(
    std::unique_ptr<IDataSource<unsigned long>> &&powerCapDataSource,
    units::power::watt_t min, units::power::watt_t max,
    std::optional<units::power::watt_t> defaultValue) noexcept
: Control(true)
, id_(AMD::PMPowerCap::ItemID)
, powerCapDataSource_(std::move(powerCapDataSource))
, powerCapPreInitValue_{0u}
, min_(min)
, max_(max)
, defaultValue_(defaultValue)
, value_(defaultValue ? *defaultValue : units::power::watt_t(1))
{
  if (min_ == units::power::watt_t(0))
    min_ = units::power::watt_t(1);
}

void AMD::PMPowerCap::preInit(ICommandQueue &ctlCmds)
{
  if (defaultValue_)
    return;

  powerCapDataSource_->read(powerCapPreInitValue_);
  cleanControl(ctlCmds);
}

void AMD::PMPowerCap::postInit(ICommandQueue &ctlCmds)
{
  if (defaultValue_)
    return;

  ctlCmds.add(
      {powerCapDataSource_->source(), std::to_string(powerCapPreInitValue_)});
}

void AMD::PMPowerCap::init()
{
  if (defaultValue_)
    return;

  unsigned long powerCapValue;
  if (powerCapDataSource_->read(powerCapValue))
    value(units::power::microwatt_t(powerCapValue));
}

std::string const &AMD::PMPowerCap::ID() const
{
  return id_;
}

void AMD::PMPowerCap::importControl(IControl::Importer &i)
{
  auto &pmPowerCapImporter = dynamic_cast<AMD::PMPowerCap::Importer &>(i);
  value(pmPowerCapImporter.providePMPowerCapValue());
}

void AMD::PMPowerCap::exportControl(IControl::Exporter &e) const
{
  auto &pmPowerCapExporter = dynamic_cast<AMD::PMPowerCap::Exporter &>(e);
  pmPowerCapExporter.takePMPowerCapRange(min(), max());
  pmPowerCapExporter.takePMPowerCapValue(value());
}

void AMD::PMPowerCap::cleanControl(ICommandQueue &ctlCmds)
{
  auto value = defaultValue_
                   ? std::to_string((*defaultValue_).to<unsigned long>())
                   : "0"; // restore default value
  ctlCmds.add({powerCapDataSource_->source(), value});
}

void AMD::PMPowerCap::syncControl(ICommandQueue &ctlCmds)
{
  unsigned long powerCapValue;
  if (powerCapDataSource_->read(powerCapValue)) {
    if (units::power::microwatt_t(powerCapValue) != value())
      ctlCmds.add({powerCapDataSource_->source(),
                   std::to_string(value().to<unsigned long>())});
  }
}

units::power::microwatt_t AMD::PMPowerCap::value() const
{
  return value_;
}

void AMD::PMPowerCap::value(units::power::microwatt_t value)
{
  value_ = std::clamp(value, min(), max());
}

units::power::microwatt_t AMD::PMPowerCap::min() const
{
  return min_;
}

units::power::microwatt_t AMD::PMPowerCap::max() const
{
  return max_;
}
