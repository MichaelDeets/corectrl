// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include <catch2/catch.hpp>

#include "core/components/controls/amd/pm/advanced/freqmode/pmfreqmode.h"

namespace Tests::AMD::PMFreqMode {

TEST_CASE("AMD PMFreqMode tests", "[GPU][AMD][PM][PMAdvanced][PMFreqMode]")
{
  std::vector<std::unique_ptr<IControl>> controlMocks;

  SECTION("Has PMFreqMode ID")
  {
    ::AMD::PMFreqMode ts(std::move(controlMocks));
    REQUIRE(ts.ID() == ::AMD::PMFreqMode::ItemID);
  }

  SECTION("Is active by default")
  {
    ::AMD::PMFreqMode ts(std::move(controlMocks));
    REQUIRE(ts.active());
  }
}

} // namespace Tests::AMD::PMFreqMode
