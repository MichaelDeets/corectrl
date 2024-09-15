// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include <catch2/catch_all.hpp>
#include <catch2/trompeloeil.hpp>

#include "common/commandqueuestub.h"
#include "common/stringdatasourcestub.h"
#include "core/components/controls/cpu/cpufreq.h"
#include "core/components/controls/cpu/handlers/iepphandler.h"

extern template struct trompeloeil::reporter<trompeloeil::specialized>;

namespace Tests::CPUFreq {

class CPUFreqTestAdapter : public ::CPUFreq
{
 public:
  using ::CPUFreq::CPUFreq;

  using ::CPUFreq::cleanControl;
  using ::CPUFreq::exportControl;
  using ::CPUFreq::importControl;
  using ::CPUFreq::scalingGovernor;
  using ::CPUFreq::scalingGovernors;
  using ::CPUFreq::syncControl;
};

class CPUFreqImporterStub : public ::CPUFreq::Importer
{
 public:
  CPUFreqImporterStub(std::string_view scalingGovernor,
                      std::optional<std::string> eppHint = std::nullopt)
  : scalingGovernor_(scalingGovernor)
  , eppHint_(eppHint)
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

  std::string const &provideCPUFreqScalingGovernor() const override
  {
    return scalingGovernor_;
  }

  std::optional<std::string> const &provideCPUFreqEPPHint() const override
  {
    return eppHint_;
  }

 private:
  std::string scalingGovernor_;
  std::optional<std::string> eppHint_;
};

class EPPHandlerMock : public IEPPHandler
{
 public:
  MAKE_CONST_MOCK0(hints, std::vector<std::string> const &(void), override);
  MAKE_CONST_MOCK0(hint, std::string const &(), override);
  MAKE_MOCK1(hint, void(std::string const &), override);
  MAKE_MOCK0(init, void(), override);
  MAKE_MOCK0(saveState, void(), override);
  MAKE_MOCK1(restoreState, void(ICommandQueue &), override);
  MAKE_MOCK1(reset, void(ICommandQueue &), override);
  MAKE_MOCK1(sync, void(ICommandQueue &), override);
};

class CPUFreqImporterMock : public ::CPUFreq::Importer
{
 public:
  MAKE_CONST_MOCK0(provideCPUFreqScalingGovernor, std::string const &(void),
                   override);
  MAKE_CONST_MOCK0(provideCPUFreqEPPHint,
                   std::optional<std::string> const &(void), override);
  MAKE_CONST_MOCK0(provideActive, bool(void), override);
  MAKE_MOCK1(
      provideImporter,
      std::optional<std::reference_wrapper<Importable::Importer>>(Item const &),
      override);
};

class CPUFreqExporterMock : public ::CPUFreq::Exporter
{
 public:
  MAKE_MOCK1(takeCPUFreqScalingGovernor, void(std::string const &), override);
  MAKE_MOCK1(takeCPUFreqScalingGovernors,
             void(std::vector<std::string> const &), override);
  MAKE_MOCK1(takeCPUFreqEPPHint, void(std::optional<std::string> const &),
             override);
  MAKE_MOCK1(takeCPUFreqEPPHints,
             void(std::optional<std::vector<std::string>> const &), override);
  MAKE_MOCK1(takeActive, void(bool), override);
  MAKE_MOCK1(
      provideExporter,
      std::optional<std::reference_wrapper<Exportable::Exporter>>(Item const &),
      override);
};

TEST_CASE("AMD CPUFreq tests", "[CPU][CPUFreq]")
{
  std::vector<std::string> availableGovernors{"performance", "powersave"};
  std::vector<std::unique_ptr<IDataSource<std::string>>> scalingGovernorDataSources;
  std::string defaultGovernor{"powersave"};
  std::string const scalingGovernorPath{"scaling_governor"};
  CommandQueueStub ctlCmds;

  SECTION("Has CPUFreq ID")
  {
    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources));

    REQUIRE(ts.ID() == ::CPUFreq::ItemID);
  }

  SECTION("Is active by default")
  {
    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources));

    REQUIRE(ts.active());
  }

  SECTION("Has defaultGovernor selected by default when is available")
  {
    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources));

    REQUIRE(ts.scalingGovernor() == defaultGovernor);
  }

  SECTION("Has the first available scaling governor selected by default when "
          "defaultGovernor is not available")
  {
    std::vector<std::string> otherGovernors{"_other_0_", "_other_1_"};
    CPUFreqTestAdapter ts(std::move(otherGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources));

    REQUIRE(ts.scalingGovernor() == "_other_0_");
  }

  SECTION("scalingGovernor only sets known scaling governors")
  {
    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources));

    ts.scalingGovernor("performance");
    REQUIRE(ts.scalingGovernor() == "performance");

    ts.scalingGovernor("unkown");
    REQUIRE(ts.scalingGovernor() == "performance");
  }

  SECTION(
      "Does not generate pre-init control commands when EPP is not supported")
  {
    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources));

    CommandQueueStub cmds;
    ts.preInit(cmds);

    REQUIRE(ctlCmds.commands().empty());
  }

  SECTION(
      "Does not generate pre-init control commands when EPP is supported and "
      "the current scaling governor is the EPP scaling governor ('powersave')")
  {
    auto eppHandlerMock = std::make_unique<EPPHandlerMock>();

    scalingGovernorDataSources.emplace_back(
        std::make_unique<StringDataSourceStub>(scalingGovernorPath,
                                               "powersave"));

    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources),
                          std::move(eppHandlerMock));
    ts.preInit(ctlCmds);

    REQUIRE(ctlCmds.commands().empty());
  }

  SECTION("Does generate pre-init control commands to set the scaling governor "
          "when EPP is supported and the current scaling governor is not the "
          "EPP scaling governor ('powersave')")
  {
    auto eppHandlerMock = std::make_unique<EPPHandlerMock>();

    scalingGovernorDataSources.emplace_back(
        std::make_unique<StringDataSourceStub>(scalingGovernorPath,
                                               "performance"));

    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources),
                          std::move(eppHandlerMock));
    ts.preInit(ctlCmds);

    REQUIRE(ctlCmds.commands().size() == 1);

    auto &[path, value] = ctlCmds.commands().front();
    REQUIRE(path == scalingGovernorPath);
    REQUIRE(value == "powersave");
  }

  SECTION("Does not generate post-init control commands")
  {
    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources));

    CommandQueueStub cmds;
    ts.postInit(cmds);

    REQUIRE(ctlCmds.commands().empty());
  }

  SECTION("Initialize EPP handler when EPP is supported")
  {
    auto eppHandlerMock = std::make_unique<EPPHandlerMock>();
    REQUIRE_CALL(*eppHandlerMock, init());

    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources),
                          std::move(eppHandlerMock));
    ts.init();
  }

  SECTION("Imports scaling governor state")
  {
    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources));
    CPUFreqImporterStub i("performance");
    ts.importControl(i);

    REQUIRE(ts.scalingGovernor() == "performance");
  }

  SECTION("Imports EPP hint state when EPP is supported")
  {
    std::string eppHint{"power"};
    auto eppHandlerMock = std::make_unique<EPPHandlerMock>();
    REQUIRE_CALL(*eppHandlerMock, hint(trompeloeil::_)).LR_WITH(_1 == eppHint);

    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources),
                          std::move(eppHandlerMock));
    CPUFreqImporterStub i("performance", eppHint);
    ts.importControl(i);
  }

  SECTION("Export scaling governor state and available scaling governors")
  {
    auto governors = availableGovernors;
    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources));
    trompeloeil::sequence seq;
    CPUFreqExporterMock e;
    REQUIRE_CALL(e, takeCPUFreqScalingGovernors(trompeloeil::_))
        .LR_WITH(_1 == governors)
        .IN_SEQUENCE(seq);
    REQUIRE_CALL(e, takeCPUFreqEPPHints(trompeloeil::eq(std::nullopt)))
        .IN_SEQUENCE(seq);
    REQUIRE_CALL(e, takeCPUFreqScalingGovernor(trompeloeil::eq(defaultGovernor)))
        .IN_SEQUENCE(seq);
    REQUIRE_CALL(e, takeCPUFreqEPPHint(trompeloeil::eq(std::nullopt)))
        .IN_SEQUENCE(seq);

    ts.exportControl(e);
  }

  SECTION("Export EPP hint state and available hints when EPP is supported")
  {
    std::vector<std::string> eppHints{"default", "power"};
    std::string eppHint{"default"};
    auto eppHandlerMock = std::make_unique<EPPHandlerMock>();
    ALLOW_CALL(*eppHandlerMock, hints()).RETURN(eppHints);
    ALLOW_CALL(*eppHandlerMock, hint()).RETURN(eppHint);

    auto governors = availableGovernors;
    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources),
                          std::move(eppHandlerMock));
    trompeloeil::sequence seq;
    CPUFreqExporterMock e;
    ALLOW_CALL(e, takeCPUFreqScalingGovernors(trompeloeil::_)).IN_SEQUENCE(seq);
    REQUIRE_CALL(e, takeCPUFreqEPPHints(trompeloeil::_))
        .LR_WITH(_1 == eppHints)
        .IN_SEQUENCE(seq);
    ALLOW_CALL(e, takeCPUFreqScalingGovernor(trompeloeil::_)).IN_SEQUENCE(seq);
    REQUIRE_CALL(e, takeCPUFreqEPPHint(trompeloeil::eq(eppHint))).IN_SEQUENCE(seq);

    ts.exportControl(e);
  }

  SECTION("Does not generate clean control commands")
  {
    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources));
    ts.cleanControl(ctlCmds);

    REQUIRE(ctlCmds.commands().empty());
  }

  SECTION("Does not generate sync control commands when is synced")
  {
    scalingGovernorDataSources.emplace_back(
        std::make_unique<StringDataSourceStub>(scalingGovernorPath,
                                               defaultGovernor));
    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources));

    ts.syncControl(ctlCmds);

    REQUIRE(ctlCmds.commands().empty());
  }

  SECTION("Does generate sync control commands when is out of sync")
  {
    scalingGovernorDataSources.emplace_back(
        std::make_unique<StringDataSourceStub>(scalingGovernorPath, "_other_"));
    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources));

    ts.syncControl(ctlCmds);

    REQUIRE(ctlCmds.commands().size() == 1);

    auto &[path, value] = ctlCmds.commands().front();
    REQUIRE(path == scalingGovernorPath);
    REQUIRE(value == defaultGovernor);
  }

  SECTION("Does not generate sync control commands for EPP when this feature "
          "is supported and the frequency scaling governor is not set "
          "to 'powersave'")
  {
    auto eppHandlerMock = std::make_unique<EPPHandlerMock>();
    FORBID_CALL(*eppHandlerMock, sync(trompeloeil::_));

    scalingGovernorDataSources.emplace_back(
        std::make_unique<StringDataSourceStub>(scalingGovernorPath,
                                               defaultGovernor));
    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources),
                          std::move(eppHandlerMock));

    ts.scalingGovernor("performance");
    ts.syncControl(ctlCmds);
  }

  SECTION("Could generate sync control commands for EPP when this feature "
          "is supported and the frequency scaling governor is set "
          "to 'powersave'")
  {
    auto eppHandlerMock = std::make_unique<EPPHandlerMock>();
    REQUIRE_CALL(*eppHandlerMock, sync(trompeloeil::_));

    scalingGovernorDataSources.emplace_back(
        std::make_unique<StringDataSourceStub>(scalingGovernorPath,
                                               defaultGovernor));
    CPUFreqTestAdapter ts(std::move(availableGovernors), defaultGovernor,
                          std::move(scalingGovernorDataSources),
                          std::move(eppHandlerMock));

    ts.syncControl(ctlCmds);
  }
}

} // namespace Tests::CPUFreq
