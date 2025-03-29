// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Juan Palacios <jpalaciosdev@gmail.com>

#include <catch2/catch_all.hpp>
#include <catch2/trompeloeil.hpp>

#include "common/commandqueuestub.h"
#include "common/stringdatasourcestub.h"
#include "common/vectorstringdatasourcestub.h"
#include "core/components/controls/amd/pm/advanced/overdrive/freqoffset/pmfreqoffset.h"
#include <units.h>

extern template struct trompeloeil::reporter<trompeloeil::specialized>;

namespace Tests::AMD::PMFreqOffset {

class PMFreqOffsetTestAdapter : public ::AMD::PMFreqOffset
{
 public:
  using ::AMD::PMFreqOffset::PMFreqOffset;

  using ::AMD::PMFreqOffset::cleanControl;
  using ::AMD::PMFreqOffset::exportControl;
  using ::AMD::PMFreqOffset::importControl;
  using ::AMD::PMFreqOffset::offset;
  using ::AMD::PMFreqOffset::range;
  using ::AMD::PMFreqOffset::syncControl;
};

class PMFreqOffsetImporterStub final : public ::AMD::PMFreqOffset::Importer
{
 public:
  PMFreqOffsetImporterStub(units::frequency::megahertz_t const offset)
  : offset_(offset)
  {
  }

  std::optional<std::reference_wrapper<Importable::Importer>>
  provideImporter(Item const &) override
  {
    return {};
  }

  bool provideActive() const override
  {
    return false;
  }

  units::frequency::megahertz_t providePMFreqOffsetValue() const override
  {
    return offset_;
  }

 private:
  units::frequency::megahertz_t const offset_;
};

class PMFreqOffsetExporterMock : public ::AMD::PMFreqOffset::Exporter
{
 public:
  MAKE_MOCK1(takePMFreqOffsetControlName, void(std::string const &), override);
  MAKE_MOCK2(takePMFreqOffsetRange,
             void(units::frequency::megahertz_t, units::frequency::megahertz_t),
             override);
  MAKE_MOCK1(takePMFreqOffsetValue, void(units::frequency::megahertz_t),
             override);

  MAKE_MOCK1(takeActive, void(bool), override);
  MAKE_MOCK1(
      provideExporter,
      std::optional<std::reference_wrapper<Exportable::Exporter>>(Item const &),
      override);
};

TEST_CASE("AMD PMFreqOffset tests",
          "[GPU][AMD][PM][PMAdvanced][PMOverdrive][PMFreqOffset]")
{
  // clang-format off
  std::vector<std::string> ppOdClkVoltageData {
                             "OD_SCLK_OFFSET:",
                             "0Mhz",
                             "OD_RANGE:",
                             "SCLK_OFFSET:  -500Mhz   1000Mhz"};
  // clang-format on
  ::AMD::PMFreqOffset::Range range = std::make_pair(
      units::make_unit<units::frequency::megahertz_t>(-500),
      units::make_unit<units::frequency::megahertz_t>(1000));

  CommandQueueStub ctlCmds;

  SECTION("Has PMFreqOffset ID")
  {
    PMFreqOffsetTestAdapter ts("SCLK", "s", std::move(range),
                               std::make_unique<VectorStringDataSourceStub>());
    REQUIRE(ts.ID() == ::AMD::PMFreqOffset::ItemID);
  }

  SECTION("Is active by default")
  {
    PMFreqOffsetTestAdapter ts("SCLK", "s", std::move(range),
                               std::make_unique<VectorStringDataSourceStub>());
    REQUIRE(ts.active());
  }

  SECTION("Does not generate pre-init control commands")
  {
    PMFreqOffsetTestAdapter ts("SCLK", "s", std::move(range),
                               std::make_unique<VectorStringDataSourceStub>(
                                   "pp_od_clk_voltage", ppOdClkVoltageData));
    ts.preInit(ctlCmds);

    auto &commands = ctlCmds.commands();
    REQUIRE(commands.empty());
  }

  SECTION("Generates post-init control commands")
  {
    PMFreqOffsetTestAdapter ts("SCLK", "s", std::move(range),
                               std::make_unique<VectorStringDataSourceStub>(
                                   "pp_od_clk_voltage", ppOdClkVoltageData));
    ts.preInit(ctlCmds);
    ctlCmds.clear();
    ts.postInit(ctlCmds);

    auto &commands = ctlCmds.commands();
    REQUIRE(commands.size() == 1);

    auto &[cmd0Path, cmd0Value] = commands.at(0);
    REQUIRE(cmd0Path == "pp_od_clk_voltage");
    REQUIRE(cmd0Value == "s 0");
  }

  SECTION("Initializes offset from pp_od_clk_voltage data source")
  {
    PMFreqOffsetTestAdapter ts("SCLK", "s", std::move(range),
                               std::make_unique<VectorStringDataSourceStub>(
                                   "pp_od_clk_voltage", ppOdClkVoltageData));
    ts.init();

    REQUIRE(ts.offset() == units::frequency::megahertz_t(0));
  }

  SECTION("Clamps offset value in range")
  {
    PMFreqOffsetTestAdapter ts("SCLK", "s", std::move(range),
                               std::make_unique<VectorStringDataSourceStub>(
                                   "pp_od_clk_voltage", ppOdClkVoltageData));
    ts.init();

    auto range = ts.range();

    // min
    ts.offset(units::frequency::megahertz_t(-1) + range.first);
    REQUIRE(ts.offset() == range.first);

    // max
    ts.offset(units::frequency::megahertz_t(1) + range.second);
    REQUIRE(ts.offset() == range.second);
  }

  SECTION("Imports its state")
  {
    PMFreqOffsetTestAdapter ts("SCLK", "s", std::move(range),
                               std::make_unique<VectorStringDataSourceStub>(
                                   "pp_od_clk_voltage", ppOdClkVoltageData));
    ts.init();

    auto offset = units::frequency::megahertz_t(-20);
    PMFreqOffsetImporterStub i(offset);

    ts.importControl(i);

    REQUIRE(ts.offset() == offset);
  }

  SECTION("Exports its state")
  {
    PMFreqOffsetTestAdapter ts("SCLK", "s", std::move(range),
                               std::make_unique<VectorStringDataSourceStub>(
                                   "pp_od_clk_voltage", ppOdClkVoltageData));
    ts.init();

    auto range = ts.range();

    trompeloeil::sequence seq;
    PMFreqOffsetExporterMock e;
    REQUIRE_CALL(e, takePMFreqOffsetControlName("SCLK")).IN_SEQUENCE(seq);
    REQUIRE_CALL(e, takePMFreqOffsetRange(trompeloeil::eq(range.first),
                                          trompeloeil::eq(range.second)))
        .IN_SEQUENCE(seq);
    REQUIRE_CALL(e, takePMFreqOffsetValue(units::frequency::megahertz_t(0)))
        .IN_SEQUENCE(seq);

    ts.exportControl(e);
  }

  SECTION("Does not generate clean control commands")
  {
    PMFreqOffsetTestAdapter ts("SCLK", "s", std::move(range),
                               std::make_unique<VectorStringDataSourceStub>(
                                   "pp_od_clk_voltage", ppOdClkVoltageData));
    ts.init();
    ts.cleanControl(ctlCmds);

    auto &commands = ctlCmds.commands();
    REQUIRE(commands.empty());
  }

  SECTION("Does not generate sync control commands when is synced")
  {
    PMFreqOffsetTestAdapter ts("SCLK", "s", std::move(range),
                               std::make_unique<VectorStringDataSourceStub>(
                                   "pp_od_clk_voltage", ppOdClkVoltageData));
    ts.init();
    ts.syncControl(ctlCmds);

    REQUIRE(ctlCmds.commands().empty());
  }

  SECTION("Generates sync control commands when is out of sync")
  {
    PMFreqOffsetTestAdapter ts("SCLK", "s", std::move(range),
                               std::make_unique<VectorStringDataSourceStub>(
                                   "pp_od_clk_voltage", ppOdClkVoltageData));
    ts.init();

    ts.offset(units::frequency::megahertz_t(-20));
    ts.syncControl(ctlCmds);

    auto &commands = ctlCmds.commands();
    REQUIRE(commands.size() == 1);

    auto &[cmd0Path, cmd0Value] = commands.at(0);
    REQUIRE(cmd0Path == "pp_od_clk_voltage");
    REQUIRE(cmd0Value == "s -20");
  }
}

} // namespace Tests::AMD::PMFreqOffset
