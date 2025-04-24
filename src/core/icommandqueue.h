// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include <string>
#include <utility>

class QByteArray;

class ICommandQueue
{
 public:
  /// Whether a command has been queued for the specified file.
  /// @param file queued command file to check
  virtual bool hasCommandQueuedFor(std::string const &file) = 0;

  /// Adds a command to the end of the queue.
  /// If the command was already queued, there is no effect in the queue.
  /// @param cmd command to queue
  virtual void add(std::pair<std::string, std::string> &&cmd) = 0;

  /// Transform all commands into raw data, cleaning the command queue.
  virtual QByteArray toRawData() = 0;

  /// Write the queued commands to the log.
  virtual void logCommands() const = 0;

  virtual ~ICommandQueue() = default;
};
