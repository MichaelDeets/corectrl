// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "commandqueue.h"

#include <QByteArray>
#include <algorithm>
#include <filesystem>
#include <iterator>
#include <spdlog/spdlog.h>
#include <utility>

CommandQueue::CommandQueue(std::unordered_set<std::string> &&multiCommandFiles) noexcept
: multiCommandFiles_(std::move(multiCommandFiles))
{
  commands().reserve(50);
}

bool CommandQueue::hasCommandQueuedFor(std::string const &file)
{
  // find the last queued command for the file
  auto it = std::find_if(commands().crbegin(), commands().crend(),
                         [&](auto const &v) { return v.first == file; });
  return it != commands().crend();
}

void CommandQueue::add(std::pair<std::string, std::string> &&cmd)
{
  // find the last queued command for the file
  auto lastIt = std::find_if(commands().crbegin(), commands().crend(),
                             [&](auto const &v) { return v.first == cmd.first; });

  if (lastIt != commands().crend() && lastIt->second == cmd.second)
    return; // command already queued

  // insert command at the end by default
  auto insertIt = commands().cend();

  // when a different command is already queued for that file...
  if (lastIt != commands().crend()) {

    auto file = std::filesystem::path(cmd.first).filename();
    if (multiCommandFiles_.contains(file)) {
      // insert it after the last queued command for that particular file
      insertIt = lastIt.base();
    }
    else {
      // remove the previous queued command
      commands().erase(std::prev(lastIt.base()));
      insertIt = commands().cend(); // refresh iterator
    }
  }

  commands().emplace(insertIt, std::move(cmd));
}

QByteArray CommandQueue::toRawData()
{
  QByteArray data;
  for (auto const &[path, value] : commands()) {
    data += path.c_str();
    data += '\0';
    data += value.c_str();
    data += '\0';
  }

  commands().clear();
  return data;
}

void CommandQueue::logCommands() const
{
  for (auto const &[path, value] : commands_)
    SPDLOG_INFO("{}: {}", path, value);
}

std::vector<std::pair<std::string, std::string>> &CommandQueue::commands()
{
  return commands_;
}
