// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "cpufreq.h"

#include "core/icommandqueue.h"
#include <algorithm>
#include <cassert>
#include <utility>

CPUFreq::CPUFreq(std::vector<std::string> &&scalingGovernors,
                 std::string const &defaultGovernor,
                 std::vector<std::unique_ptr<IDataSource<std::string>>>
                     &&scalingGovernorDataSources,
                 std::unique_ptr<IEPPHandler> &&eppHandler) noexcept
: Control(true)
, id_(CPUFreq::ItemID)
, scalingGovernors_(std::move(scalingGovernors))
, scalingGovernorDataSources_(std::move(scalingGovernorDataSources))
, eppHandler_(std::move(eppHandler))
{
  scalingGovernor(defaultGovernor);
  if (scalingGovernor_.empty())
    scalingGovernor(scalingGovernors_.front());
}

void CPUFreq::preInit(ICommandQueue &)
{
}

void CPUFreq::postInit(ICommandQueue &)
{
}

void CPUFreq::init()
{
}

std::string const &CPUFreq::ID() const
{
  return id_;
}

void CPUFreq::importControl(IControl::Importer &i)
{
  auto &importer = dynamic_cast<CPUFreq::Importer &>(i);
  scalingGovernor(importer.provideCPUFreqScalingGovernor());

  if (eppHandler_) {
    auto hint = importer.provideCPUFreqEPPHint();
    assert(hint.has_value());
    eppHandler_->hint(*hint);
  }
}

void CPUFreq::exportControl(IControl::Exporter &e) const
{
  auto &exporter = dynamic_cast<CPUFreq::Exporter &>(e);
  exporter.takeCPUFreqScalingGovernors(scalingGovernors());
  exporter.takeCPUFreqEPPHints(eppHints());
  exporter.takeCPUFreqScalingGovernor(scalingGovernor());
  exporter.takeCPUFreqEPPHint(eppHint());
}

void CPUFreq::cleanControl(ICommandQueue &)
{
}

void CPUFreq::syncControl(ICommandQueue &ctlCmds)
{
  for (auto &scalingGovernorDataSource : scalingGovernorDataSources_)
    if (scalingGovernorDataSource->read(dataSourceEntry_)) {
      if (dataSourceEntry_ != scalingGovernor())
        ctlCmds.add({scalingGovernorDataSource->source(), scalingGovernor()});
    }

  if (eppHandler_ && scalingGovernor() == eppScalingGovernor_)
    eppHandler_->sync(ctlCmds);
}

std::string const &CPUFreq::scalingGovernor() const
{
  return scalingGovernor_;
}

void CPUFreq::scalingGovernor(std::string const &scalingGovernor)
{
  // only assign known scalingGovernors
  auto iter = std::find_if(scalingGovernors().cbegin(),
                           scalingGovernors().cend(),
                           [&](auto const &availableScalingGovernor) {
                             return scalingGovernor == availableScalingGovernor;
                           });
  if (iter != scalingGovernors().cend())
    scalingGovernor_ = scalingGovernor;
}

std::vector<std::string> const &CPUFreq::scalingGovernors() const
{
  return scalingGovernors_;
}

std::optional<std::string> CPUFreq::eppHint() const
{
  return eppHandler_ ? std::make_optional(eppHandler_->hint()) : std::nullopt;
}

std::optional<std::vector<std::string>> CPUFreq::eppHints() const
{
  return eppHandler_ ? std::make_optional(eppHandler_->hints()) : std::nullopt;
}
