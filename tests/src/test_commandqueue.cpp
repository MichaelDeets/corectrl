// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include <catch2/catch_all.hpp>

#include "core/commandqueue.h"
#include <QString>

namespace Tests::CommandQueue {

class CommandQueueTestAdapter : public ::CommandQueue
{
 public:
  using ::CommandQueue::CommandQueue;

  using ::CommandQueue::add;
  using ::CommandQueue::hasCommandQueuedFor;
  using ::CommandQueue::commands;
  using ::CommandQueue::toRawData;
};

TEST_CASE("CommandQueue tests", "[CommandQueue]")
{
  CommandQueueTestAdapter ts({"multi-cmd-file"});

  SECTION("Initially, is empty")
  {
    REQUIRE(ts.commands().size() == 0);

    SECTION("Commands can be added")
    {
      ts.add({"path", "value"});

      REQUIRE(ts.commands().size() == 1);

      std::pair<std::string, std::string> cmd{"path", "value"};
      REQUIRE(ts.commands().front() == cmd);

      SECTION("Queuing an already queued command has no effect")
      {
        ts.add({"path", "value"});

        REQUIRE(ts.commands().size() == 1);
      }

      SECTION("Queuing a command for a multi-command file inserts the command "
              "just after the last queued command for that file")
      {
        ts.add({"multi-cmd-file", "1"});
        ts.add({"other", "value"});
        ts.add({"multi-cmd-file", "2"});

        auto &commands = ts.commands();
        REQUIRE(commands.size() == 4);

        REQUIRE(commands[0].first == "path");
        REQUIRE(commands[0].second == "value");
        REQUIRE(commands[1].first == "multi-cmd-file");
        REQUIRE(commands[1].second == "1");
        REQUIRE(commands[2].first == "multi-cmd-file");
        REQUIRE(commands[2].second == "2");
        REQUIRE(commands[3].first == "other");
        REQUIRE(commands[3].second == "value");
      }

      SECTION(
          "Queuing a command for a non multi-command file removes the last "
          "queued command for that file and inserts the new command at the end")
      {
        ts.add({"other", "value"});

        auto &commands = ts.commands();
        REQUIRE(commands.size() == 2);

        REQUIRE(commands[0].first == "path");
        REQUIRE(commands[0].second == "value");
        REQUIRE(commands[1].first == "other");
        REQUIRE(commands[1].second == "value");

        ts.add({"path", "value1"});

        REQUIRE(commands[0].first == "other");
        REQUIRE(commands[0].second == "value");
        REQUIRE(commands[1].first == "path");
        REQUIRE(commands[1].second == "value1");
      }

      SECTION("Commands are transformed into raw data...")
      {
        auto data = ts.toRawData();

        REQUIRE(data == QByteArray("path\0value\0", 11));

        SECTION("The queue is cleared")
        {
          REQUIRE(ts.commands().size() == 0);
        }
      }
    }
  }
}

} // namespace Tests::CommandQueue
