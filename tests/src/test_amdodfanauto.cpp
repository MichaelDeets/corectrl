// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#include <catch2/catch.hpp>

#include "common/commandqueuestub.h"
#include "common/vectorstringdatasourcestub.h"
#include "core/components/controls/amd/fan/overdrive/auto/odfanauto.h"

namespace Tests::AMD::OdFanAuto {

class OdFanAutoTestAdapter : public ::AMD::OdFanAuto
{
 public:
  using ::AMD::OdFanAuto::OdFanAuto;

  using ::AMD::OdFanAuto::cleanControl;
  using ::AMD::OdFanAuto::syncControl;
};

TEST_CASE("AMD OdFanAuto tests", "[GPU][AMD][Fan][Overdrive][OdFanAuto]")
{
  CommandQueueStub ctlCmds;
  OdFanAutoTestAdapter ts(std::make_unique<VectorStringDataSourceStub>("path"));

  SECTION("Has OdFanAuto ID")
  {
    REQUIRE(ts.ID() == ::AMD::OdFanAuto::ItemID);
  }

  SECTION("Is active by default")
  {
    REQUIRE(ts.active());
  }

  SECTION("Does not generate pre-init control commands")
  {
    ts.preInit(ctlCmds);

    REQUIRE(ctlCmds.commands().empty());
  }

  SECTION("Does not generate post-init control commands")
  {
    ts.postInit(ctlCmds);

    REQUIRE(ctlCmds.commands().empty());
  }

  SECTION("Does not generate clean control commands")
  {
    ts.cleanControl(ctlCmds);

    REQUIRE(ctlCmds.commands().empty());
  }

  SECTION("Does generate sync control commands the first time is sync")
  {
    ts.syncControl(ctlCmds);

    REQUIRE(ctlCmds.commands().size() == 2);
    auto &[path0, value0] = ctlCmds.commands().at(0);
    REQUIRE(path0 == "path");
    REQUIRE(value0 == "r");
    auto &[path1, value1] = ctlCmds.commands().at(1);
    REQUIRE(path1 == "path");
    REQUIRE(value1 == "c");
  }

  SECTION("Does not generate sync control commands when is already active")
  {
    ts.syncControl(ctlCmds);
    ctlCmds.clear();

    ts.syncControl(ctlCmds);

    REQUIRE(ctlCmds.commands().empty());
  }

  SECTION("Does generate sync control commands when is reactivated")
  {
    ts.syncControl(ctlCmds);
    ts.activate(false);
    ts.clean(ctlCmds);
    ts.activate(true);
    ctlCmds.clear();

    ts.syncControl(ctlCmds);

    REQUIRE(ctlCmds.commands().size() == 2);
    auto &[path0, value0] = ctlCmds.commands().at(0);
    REQUIRE(path0 == "path");
    REQUIRE(value0 == "r");
    auto &[path1, value1] = ctlCmds.commands().at(1);
    REQUIRE(path1 == "path");
    REQUIRE(value1 == "c");
  }
}

} // namespace Tests::AMD::OdFanAuto
