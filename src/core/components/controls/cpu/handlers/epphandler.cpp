// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024 Juan Palacios <jpalaciosdev@gmail.com>

#include "epphandler.h"

#include "core/icommandqueue.h"
#include <algorithm>
#include <utility>

EPPHandler::EPPHandler(std::vector<std::string> &&eppHints,
                       std::vector<std::unique_ptr<IDataSource<std::string>>>
                           &&eppHintDataSources) noexcept
: hints_(std::move(eppHints))
, eppHintDataSources_(std::move(eppHintDataSources))
{
  hint("default");
  if (hint_.empty())
    hint(hints_.front());
}

std::string const &EPPHandler::hint() const
{
  return hint_;
}

void EPPHandler::hint(std::string const &eppHint)
{
  // only assign known EPP hints
  auto iter = std::find_if(
      hints().cbegin(), hints().cend(),
      [&](auto const &availableHint) { return eppHint == availableHint; });
  if (iter != hints().cend())
    hint_ = eppHint;
}

std::vector<std::string> const &EPPHandler::hints() const
{
  return hints_;
}

void EPPHandler::saveState()
{
}

void EPPHandler::restoreState(ICommandQueue &)
{
}

void EPPHandler::reset(ICommandQueue &)
{
}

void EPPHandler::sync(ICommandQueue &ctlCmds)
{
  for (auto &eppHintDataSource : eppHintDataSources_)
    if (eppHintDataSource->read(dataSourceEntry_)) {
      if (dataSourceEntry_ != hint())
        ctlCmds.add({eppHintDataSource->source(), hint()});
    }
}
