// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#include "odfanauto.h"

#include "core/icommandqueue.h"
#include "core/idatasource.h"
#include <utility>

AMD::OdFanAuto::OdFanAuto(
    std::unique_ptr<IDataSource<std::vector<std::string>>> &&dataSource) noexcept
: Control(true)
, id_(AMD::OdFanAuto::ItemID)
, triggerAutoOpMode_(true)
, dataSource_(std::move(dataSource))
{
}

void AMD::OdFanAuto::preInit(ICommandQueue &)
{
}

void AMD::OdFanAuto::postInit(ICommandQueue &)
{
}

void AMD::OdFanAuto::init()
{
}

std::string const &AMD::OdFanAuto::ID() const
{
  return id_;
}

void AMD::OdFanAuto::importControl(IControl::Importer &)
{
}

void AMD::OdFanAuto::exportControl(IControl::Exporter &) const
{
}

void AMD::OdFanAuto::cleanControl(ICommandQueue &)
{
  triggerAutoOpMode_ = true;
}

void AMD::OdFanAuto::syncControl(ICommandQueue &ctlCmds)
{
  if (triggerAutoOpMode_) {
    addResetCmds(ctlCmds);
    triggerAutoOpMode_ = false;
  }
}

void AMD::OdFanAuto::addResetCmds(ICommandQueue &ctlCmds) const
{
  ctlCmds.add({dataSource_->source(), "r"});

  // NOTE Apparently, there is no need to submit the commit command after the
  // reset one on the new fan overdrive interfaces [1]. This interaction model
  // digress from the overdrive clock-voltage interface model, where the reset
  // command only restores the default values without actually committing them.
  //
  // Submit a commit command just in case this is changed to align with the
  // original overdrive interface interaction model.
  //
  // [1] https://gitlab.freedesktop.org/drm/amd/-/issues/2402#note_2211197
  ctlCmds.add({dataSource_->source(), "c"});
}
