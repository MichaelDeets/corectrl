// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021 Juan Palacios <jpalaciosdev@gmail.com>

#include <catch2/catch.hpp>

#include "core/components/controls/amd/pm/powerstate/pmpowerstatemode.h"

namespace Tests::AMD::PMPowerStateMode {

TEST_CASE("AMD PMPowerStateMode tests", "[GPU][AMD][PM][PMPowerStateMode]")
{
  std::vector<std::unique_ptr<IControl>> controlMocks;

  SECTION("Has PMPowerStateMode ID")
  {
    ::AMD::PMPowerStateMode ts(std::move(controlMocks));
    REQUIRE(ts.ID() == ::AMD::PMPowerStateMode::ItemID);
  }

  SECTION("Is active by default")
  {
    ::AMD::PMPowerStateMode ts(std::move(controlMocks));
    REQUIRE(ts.active());
  }
}

} // namespace Tests::AMD::PMPowerStateMode
