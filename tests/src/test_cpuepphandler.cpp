// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024 Juan Palacios <jpalaciosdev@gmail.com>

#include <catch2/catch_all.hpp>

#include "common/commandqueuestub.h"
#include "common/stringdatasourcestub.h"
#include "core/components/controls/cpu/handlers/epphandler.h"

namespace Tests::CPU::EPPHandler {

TEST_CASE("AMD PpDpmHandler tests", "[CPU][DataSourceHandler][EPPHandler]")
{
  CommandQueueStub ctlCmds;

  std::string defaultHint{"default"};

  auto availableEPPHintsDataSource = std::make_unique<StringDataSourceStub>(
      "energy_performance_available_preferences", "default power");
  std::vector<std::unique_ptr<IDataSource<std::string>>> eppHintDataSource{};
  eppHintDataSource.emplace_back(std::make_unique<StringDataSourceStub>(
      "energy_performance_preference", "default"));

  ::EPPHandler ts(std::move(availableEPPHintsDataSource),
                  std::move(eppHintDataSource));
  ts.init();

  SECTION("Initialize the available EPP hints from its data source")
  {
    std::vector<std::string> eppHints = {"default", "power"};
    REQUIRE_THAT(ts.hints(), Catch::Matchers::Equals(eppHints));
  }

  SECTION("Has 'default' as the active hint by default when available")
  {
    REQUIRE(ts.hint() == defaultHint);
  }

  SECTION("Setting a hint...")
  {
    SECTION("Works with known hints")
    {
      ts.hint("power");
      REQUIRE(ts.hint() == "power");
    }

    SECTION("Ignores unknown hints")
    {
      ts.hint("unknown");
      REQUIRE(ts.hint() == defaultHint);
    }
  }

  SECTION("Does not save and restore CPU data source state")
  {
    ts.saveState();
    ts.restoreState(ctlCmds);
    REQUIRE(ctlCmds.commands().empty());
  }

  SECTION("Does not generate reset control commands")
  {
    ts.reset(ctlCmds);
    REQUIRE(ctlCmds.commands().empty());
  }

  SECTION("Does not generate sync control commands when is synced")
  {
    ts.sync(ctlCmds);
    REQUIRE(ctlCmds.commands().empty());
  }

  SECTION("Generates sync control commands when CPU hint is out of sync")
  {
    ts.hint("power");
    ts.sync(ctlCmds);

    auto &commands = ctlCmds.commands();
    REQUIRE(commands.size() == 1);
    auto &[cmdPath, cmdValue] = commands.at(0);
    REQUIRE(cmdPath == "energy_performance_preference");
    REQUIRE(cmdValue == "power");
  }
}

} // namespace Tests::CPU::EPPHandler
