// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "icommandqueue.h"
#include <string>
#include <unordered_set>
#include <vector>

class CommandQueue : public ICommandQueue
{
 public:
  CommandQueue(std::unordered_set<std::string> &&multiCommandFiles = {}) noexcept;

  bool hasCommandQueuedFor(std::string const &file) override;
  void add(std::pair<std::string, std::string> &&cmd) override;
  QByteArray toRawData() override;
  void logCommands() const override;

 protected:
  std::vector<std::pair<std::string, std::string>> &commands();

 private:
  std::unordered_set<std::string> multiCommandFiles_;
  std::vector<std::pair<std::string, std::string>> commands_;
};
