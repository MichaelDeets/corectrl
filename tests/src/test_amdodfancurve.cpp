// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include <catch2/catch_all.hpp>
#include <catch2/trompeloeil.hpp>

#include "common/commandqueuestub.h"
#include "common/vectorstringdatasourcestub.h"
#include "core/components/controls/amd/fan/overdrive/curve/odfancurve.h"

extern template struct trompeloeil::reporter<trompeloeil::specialized>;

#include <iostream>

namespace Tests::AMD::OdFanCurve {

class OdFanCurveTestAdapter : public ::AMD::OdFanCurve
{
 public:
  // clang-format off
  using ::AMD::OdFanCurve::OdFanCurve;

  using ::AMD::OdFanCurve::exportControl;
  using ::AMD::OdFanCurve::importControl;
  using ::AMD::OdFanCurve::cleanControl;
  using ::AMD::OdFanCurve::syncControl;

  using ::AMD::OdFanCurve::defaultCurve;
  using ::AMD::OdFanCurve::fanCurve;
  using ::AMD::OdFanCurve::controlPoints;
  using ::AMD::OdFanCurve::tempRange;
  using ::AMD::OdFanCurve::speedRange;
  using ::AMD::OdFanCurve::isZeroCurve;
  using ::AMD::OdFanCurve::setPointCoordinatesFrom;
  using ::AMD::OdFanCurve::controlPointCmd;
  using ::AMD::OdFanCurve::toCurvePoints;

  // clang-format on
};

class FanCurveImporterStub : public ::AMD::OdFanCurve::Importer
{
 public:
  FanCurveImporterStub(std::vector<::AMD::OdFanCurve::CurvePoint> const &curve)
  : curve_(curve)
  {
  }

  std::optional<std::reference_wrapper<Importable::Importer>>
  provideImporter(Item const &) override
  {
    return *this;
  }

  bool provideActive() const override
  {
    return false;
  }

  std::vector<::AMD::OdFanCurve::CurvePoint> const &provideFanCurve() const override
  {
    return curve_;
  }

 private:
  std::vector<::AMD::OdFanCurve::CurvePoint> const curve_;
};

class FanCurveExporterMock : public ::AMD::OdFanCurve::Exporter
{
 public:
  MAKE_MOCK1(takeFanCurve,
             void(std::vector<::AMD::OdFanCurve::CurvePoint> const &), override);
  MAKE_MOCK2(takeFanCurveRange,
             void(::AMD::OdFanCurve::TempRange, ::AMD::OdFanCurve::SpeedRange),
             override);
  MAKE_MOCK1(takeActive, void(bool), override);
  MAKE_MOCK1(
      provideExporter,
      std::optional<std::reference_wrapper<Exportable::Exporter>>(Item const &),
      override);
};

TEST_CASE("AMD OdFanCurve tests", "[GPU][AMD][Fan][Overdrive][OdFanCurve]")
{
  CommandQueueStub ctlCmds;
  // clang-format off
  std::vector<std::string> regularInput{
    "OD_FAN_CURVE:",
    "0: 10C 10%",
    "1: 45C 20%",
    "2: 50C 50%",
    "3: 75C 70%",
    "4: 100C 100%",
    "OD_RANGE:",
    "FAN_CURVE(hotspot temp): 10C 100C",
    "FAN_CURVE(fan speed): 10% 100%"
  };
  std::vector<std::string> zeroCurveInput{
    "OD_FAN_CURVE:",
    "0: 0C 0%",
    "1: 0C 0%",
    "2: 0C 0%",
    "3: 0C 0%",
    "4: 0C 0%",
    "OD_RANGE:",
    "FAN_CURVE(hotspot temp): 10C 100C",
    "FAN_CURVE(fan speed): 10% 100%"
  };
  // clang-format on

  SECTION("toCurvePoints")
  {
    // clang-format off
    std::vector<std::tuple<unsigned int, units::temperature::celsius_t,
                           units::concentration::percent_t>>
        controlCurve {
          {0, units::temperature::celsius_t(0), units::concentration::percent_t(0)},
          {1, units::temperature::celsius_t(1), units::concentration::percent_t(1)},
          {2, units::temperature::celsius_t(2), units::concentration::percent_t(2)},
          {3, units::temperature::celsius_t(3), units::concentration::percent_t(3)},
          {4, units::temperature::celsius_t(4), units::concentration::percent_t(4)}
        };

    std::vector<std::pair<units::temperature::celsius_t,
                          units::concentration::percent_t>>
        curvePoints {
          {units::temperature::celsius_t(0), units::concentration::percent_t(0)},
          {units::temperature::celsius_t(1), units::concentration::percent_t(1)},
          {units::temperature::celsius_t(2), units::concentration::percent_t(2)},
          {units::temperature::celsius_t(3), units::concentration::percent_t(3)},
          {units::temperature::celsius_t(4), units::concentration::percent_t(4)}
        };
    // clang-format on

    OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>());
    auto output = ts.toCurvePoints(controlCurve);
    REQUIRE_THAT(output, Catch::Matchers::Equals(curvePoints));
  }

  SECTION("setPointCoordinatesFrom")
  {
    // clang-format off
    std::vector<std::tuple<unsigned int, units::temperature::celsius_t,
                           units::concentration::percent_t>>
        curve {
          {0, units::temperature::celsius_t(0), units::concentration::percent_t(0)},
          {1, units::temperature::celsius_t(0), units::concentration::percent_t(0)},
          {2, units::temperature::celsius_t(0), units::concentration::percent_t(0)},
          {3, units::temperature::celsius_t(0), units::concentration::percent_t(0)},
          {4, units::temperature::celsius_t(0), units::concentration::percent_t(0)}
        };
    std::vector<std::pair<units::temperature::celsius_t,
                          units::concentration::percent_t>>
        values {
          {units::temperature::celsius_t(-10), units::concentration::percent_t(-10)},
          {units::temperature::celsius_t(0), units::concentration::percent_t(0)},
          {units::temperature::celsius_t(1), units::concentration::percent_t(1)},
          {units::temperature::celsius_t(20), units::concentration::percent_t(20)},
          {units::temperature::celsius_t(300), units::concentration::percent_t(300)}
        };
    std::vector<std::tuple<unsigned int, units::temperature::celsius_t,
                           units::concentration::percent_t>>
        targetCurve {
          {0, units::temperature::celsius_t(-10), units::concentration::percent_t(-10)},
          {1, units::temperature::celsius_t(0), units::concentration::percent_t(0)},
          {2, units::temperature::celsius_t(1), units::concentration::percent_t(1)},
          {3, units::temperature::celsius_t(20), units::concentration::percent_t(20)},
          {4, units::temperature::celsius_t(300), units::concentration::percent_t(300)}
        };
    // clang-format on

    OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>());
    ts.setPointCoordinatesFrom(curve, values);
    REQUIRE_THAT(curve, Catch::Matchers::Equals(targetCurve));
  }

  SECTION("isZeroCurve")
  {
    // clang-format off
    std::vector<std::tuple<unsigned int, units::temperature::celsius_t,
                           units::concentration::percent_t>>
        zeroCurve {
          {0, units::temperature::celsius_t(0), units::concentration::percent_t(0)},
          {1, units::temperature::celsius_t(0), units::concentration::percent_t(0)},
          {2, units::temperature::celsius_t(0), units::concentration::percent_t(0)},
          {3, units::temperature::celsius_t(0), units::concentration::percent_t(0)},
          {4, units::temperature::celsius_t(0), units::concentration::percent_t(0)}
        };
    std::vector<std::tuple<unsigned int, units::temperature::celsius_t,
                           units::concentration::percent_t>>
        nonZeroCurve {
          {0, units::temperature::celsius_t(0), units::concentration::percent_t(0)},
          {1, units::temperature::celsius_t(1), units::concentration::percent_t(1)},
          {2, units::temperature::celsius_t(2), units::concentration::percent_t(2)},
          {3, units::temperature::celsius_t(3), units::concentration::percent_t(3)},
          {4, units::temperature::celsius_t(4), units::concentration::percent_t(4)}
        };
    // clang-format on

    OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>());
    REQUIRE(ts.isZeroCurve(zeroCurve));
    REQUIRE_FALSE(ts.isZeroCurve(nonZeroCurve));
  }

  SECTION("controlPointCmd")
  {
    auto point = std::make_tuple(0, units::temperature::celsius_t(-100),
                                 units::concentration::percent_t(100));

    OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>());
    auto output = ts.controlPointCmd(point);
    REQUIRE(output == "0 -100 100");
  }

  SECTION("Has OdFanCurve ID")
  {
    OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>());

    REQUIRE(ts.ID() == ::AMD::OdFanCurve::ItemID);
  }

  SECTION("Is not active by default")
  {
    OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>());

    REQUIRE_FALSE(ts.active());
  }

  SECTION("Generate pre-init control commands")
  {
    OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>(
        "fan_curve", regularInput));
    ts.preInit(ctlCmds);

    REQUIRE(ctlCmds.commands().size() == 2);
    auto &[cmd0Path, cmd0Value] = ctlCmds.commands().front();
    REQUIRE(cmd0Path == "fan_curve");
    REQUIRE(cmd0Value == "r");
    auto &[cmd1Path, cmd1Value] = ctlCmds.commands().back();
    REQUIRE(cmd1Path == "fan_curve");
    REQUIRE(cmd1Value == "c");
  }

  SECTION("Does not generate post-init control commands with a pre-init zero "
          "point curve")
  {
    OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>(
        "fan_curve", zeroCurveInput));
    ts.preInit(ctlCmds);
    ts.init();
    ctlCmds.clear();

    ts.postInit(ctlCmds);

    REQUIRE(ctlCmds.commands().empty());
  }

  SECTION("Generate post-init control commands with a pre-init non-zero points "
          "curve restoring the pre-init curve state")
  {
    OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>(
        "fan_curve", regularInput));
    ts.preInit(ctlCmds);
    ts.init();
    ctlCmds.clear();

    ts.postInit(ctlCmds);

    auto const &cmds = ctlCmds.commands();
    REQUIRE(cmds.size() == 6);
    REQUIRE(cmds[0].first == "fan_curve");
    REQUIRE(cmds[0].second == "0 10 10");
    REQUIRE(cmds[1].first == "fan_curve");
    REQUIRE(cmds[1].second == "1 45 20");
    REQUIRE(cmds[2].first == "fan_curve");
    REQUIRE(cmds[2].second == "2 50 50");
    REQUIRE(cmds[3].first == "fan_curve");
    REQUIRE(cmds[3].second == "3 75 70");
    REQUIRE(cmds[4].first == "fan_curve");
    REQUIRE(cmds[4].second == "4 100 100");
    REQUIRE(cmds[5].second == "c");
  }

  SECTION("Initializes...")
  {
    SECTION("Both temperature and speed ranges")
    {
      OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>(
          "fan_curve", regularInput));
      ts.init();

      REQUIRE(ts.tempRange() ==
              std::make_pair(units::temperature::celsius_t(10),
                             units::temperature::celsius_t(100)));
      REQUIRE(ts.speedRange() ==
              std::make_pair(units::concentration::percent_t(10),
                             units::concentration::percent_t(100)));
    }

    SECTION("Control curve with the custom default curve when the GPU has "
            "default zero point curve")
    {
      OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>(
          "fan_curve", zeroCurveInput));
      ts.init();

      auto defaultCurvePoints = ts.defaultCurve();
      auto controlPoints = ts.controlPoints();
      REQUIRE(controlPoints.size() == 5);
      auto const &[_i0, t0, s0] = controlPoints[0];
      REQUIRE(t0 == defaultCurvePoints[0].first);
      REQUIRE(s0 == defaultCurvePoints[0].second);
      auto const &[_i1, t1, s1] = controlPoints[1];
      REQUIRE(t1 == defaultCurvePoints[1].first);
      REQUIRE(s1 == defaultCurvePoints[1].second);
      auto const &[_i2, t2, s2] = controlPoints[2];
      REQUIRE(t2 == defaultCurvePoints[2].first);
      REQUIRE(s2 == defaultCurvePoints[2].second);
      auto const &[_i3, t3, s3] = controlPoints[3];
      REQUIRE(t3 == defaultCurvePoints[3].first);
      REQUIRE(s3 == defaultCurvePoints[3].second);
      auto const &[_i4, t4, s4] = controlPoints[4];
      REQUIRE(t4 == defaultCurvePoints[4].first);
      REQUIRE(s4 == defaultCurvePoints[4].second);
    }

    SECTION("Control curve with the device default curve when the GPU has "
            "default non-zero point curve")
    {
      OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>(
          "fan_curve", regularInput));
      ts.init();

      auto controlPoints = ts.controlPoints();
      REQUIRE(controlPoints.size() == 5);
      auto const &[_i0, t0, s0] = controlPoints[0];
      REQUIRE(t0 == units::temperature::celsius_t(10));
      REQUIRE(s0 == units::concentration::percent_t(10));
      auto const &[_i1, t1, s1] = controlPoints[1];
      REQUIRE(t1 == units::temperature::celsius_t(45));
      REQUIRE(s1 == units::concentration::percent_t(20));
      auto const &[_i2, t2, s2] = controlPoints[2];
      REQUIRE(t2 == units::temperature::celsius_t(50));
      REQUIRE(s2 == units::concentration::percent_t(50));
      auto const &[_i3, t3, s3] = controlPoints[3];
      REQUIRE(t3 == units::temperature::celsius_t(75));
      REQUIRE(s3 == units::concentration::percent_t(70));
      auto const &[_i4, t4, s4] = controlPoints[4];
      REQUIRE(t4 == units::temperature::celsius_t(100));
      REQUIRE(s4 == units::concentration::percent_t(100));
    }

    SECTION("Control curve with normalized curve point coordinates")
    {
      // clang-format off
      std::vector<std::string> input{
        "OD_FAN_CURVE:",
        "0: 0C 0%",     // out of range
        "1: 45C 20%",
        "2: 50C 50%",
        "3: 75C 70%",
        "4: 200C 200%", // out of range
        "OD_RANGE:",
        "FAN_CURVE(hotspot temp): 10C 100C",
        "FAN_CURVE(fan speed): 10% 100%"
      };
      // clang-format on
      OdFanCurveTestAdapter ts(
          std::make_unique<VectorStringDataSourceStub>("fan_curve", input));
      ts.init();

      auto const &tempRange = ts.tempRange();
      auto const &speedRange = ts.speedRange();
      auto const &points = ts.controlPoints();

      REQUIRE_FALSE(
          std::any_of(points.cbegin(), points.cend(), [&](auto const &point) {
            return std::get<1>(point) < tempRange.first ||
                   std::get<1>(point) > tempRange.second ||
                   std::get<2>(point) < speedRange.first ||
                   std::get<2>(point) > speedRange.second;
          }));
    }
  }

  SECTION("Imports its state")
  {
    // clang-format off
    std::vector<std::pair<units::temperature::celsius_t, units::concentration::percent_t>>
        curve {
      {units::temperature::celsius_t(10), units::concentration::percent_t(15)},
      {units::temperature::celsius_t(20), units::concentration::percent_t(25)},
      {units::temperature::celsius_t(30), units::concentration::percent_t(35)},
      {units::temperature::celsius_t(40), units::concentration::percent_t(45)},
      {units::temperature::celsius_t(50), units::concentration::percent_t(55)},
    };
    // clang-format on

    FanCurveImporterStub i(curve);
    OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>(
        "fan_curve", regularInput));
    ts.init();

    ts.importControl(i);

    auto controlPoints = ts.controlPoints();
    REQUIRE(controlPoints.size() == 5);
    auto const &[_i0, t0, s0] = controlPoints[0];
    REQUIRE(t0 == units::temperature::celsius_t(10));
    REQUIRE(s0 == units::concentration::percent_t(15));
    auto const &[_i1, t1, s1] = controlPoints[1];
    REQUIRE(t1 == units::temperature::celsius_t(20));
    REQUIRE(s1 == units::concentration::percent_t(25));
    auto const &[_i2, t2, s2] = controlPoints[2];
    REQUIRE(t2 == units::temperature::celsius_t(30));
    REQUIRE(s2 == units::concentration::percent_t(35));
    auto const &[_i3, t3, s3] = controlPoints[3];
    REQUIRE(t3 == units::temperature::celsius_t(40));
    REQUIRE(s3 == units::concentration::percent_t(45));
    auto const &[_i4, t4, s4] = controlPoints[4];
    REQUIRE(t4 == units::temperature::celsius_t(50));
    REQUIRE(s4 == units::concentration::percent_t(55));
  }

  SECTION("Export its state")
  {
    // clang-format off
    std::vector<std::pair<units::temperature::celsius_t, units::concentration::percent_t>>
        curve {
      {units::temperature::celsius_t(10), units::concentration::percent_t(10)},
      {units::temperature::celsius_t(45), units::concentration::percent_t(20)},
      {units::temperature::celsius_t(50), units::concentration::percent_t(50)},
      {units::temperature::celsius_t(75), units::concentration::percent_t(70)},
      {units::temperature::celsius_t(100), units::concentration::percent_t(100)},
    };
    // clang-format on

    OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>(
        "fan_curve", regularInput));
    ts.init();

    trompeloeil::sequence seq;
    FanCurveExporterMock e;
    REQUIRE_CALL(e, takeFanCurveRange(trompeloeil::_, trompeloeil::_))
        .WITH(_1 == std::make_pair(units::temperature::celsius_t(10),
                                   units::temperature::celsius_t(100)))
        .WITH(_2 == std::make_pair(units::concentration::percent_t(10),
                                   units::concentration::percent_t(100)))
        .IN_SEQUENCE(seq);
    REQUIRE_CALL(e, takeFanCurve(trompeloeil::_))
        .LR_WITH(_1 == curve)
        .IN_SEQUENCE(seq);

    ts.exportControl(e);
  }

  SECTION("Generate clean control commands")
  {
    OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>(
        "fan_curve", regularInput));
    ts.cleanControl(ctlCmds);

    auto const &cmds = ctlCmds.commands();
    REQUIRE(cmds.size() == 2);
    auto const &[cmd0Path, cmd0Value] = cmds[0];
    REQUIRE(cmd0Path == "fan_curve");
    REQUIRE(cmd0Value == "r");
    auto const &[cmd1Path, cmd1Value] = cmds[1];
    REQUIRE(cmd1Path == "fan_curve");
    REQUIRE(cmd1Value == "c");
  }

  SECTION("Does not generate sync control commands when is synced")
  {
    OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>(
        "fan_curve", regularInput));
    ts.init();
    ts.syncControl(ctlCmds);
    ctlCmds.clear();

    ts.syncControl(ctlCmds);

    REQUIRE(ctlCmds.commands().empty());
  }

  SECTION("Does generate sync control commands when...")
  {
    SECTION("curve is in sync but the operation mode must be set to manual")
    {
      OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>(
          "fan_curve", regularInput));
      ts.init();

      ts.syncControl(ctlCmds);

      auto const &cmds = ctlCmds.commands();
      REQUIRE(cmds.size() == 2);
      auto const &[cmd0Path, cmd0Value] = cmds[0];
      REQUIRE(cmd0Path == "fan_curve");
      REQUIRE(cmd0Value == "r");
      auto const &[cmd1Path, cmd1Value] = cmds[1];
      REQUIRE(cmd1Path == "fan_curve");
      REQUIRE(cmd1Value == "c");
    }

    SECTION("curve is out of sync")
    {
      // clang-format off
    std::vector<std::pair<units::temperature::celsius_t,
                          units::concentration::percent_t>>
        curve {
          {units::temperature::celsius_t(15), units::concentration::percent_t(15)},
          {units::temperature::celsius_t(20), units::concentration::percent_t(20)},
          {units::temperature::celsius_t(30), units::concentration::percent_t(30)},
          {units::temperature::celsius_t(40), units::concentration::percent_t(40)},
          {units::temperature::celsius_t(50), units::concentration::percent_t(50)}
        };
      // clang-format on

      OdFanCurveTestAdapter ts(std::make_unique<VectorStringDataSourceStub>(
          "fan_curve", regularInput));
      ts.init();

      ts.fanCurve(curve);
      ts.syncControl(ctlCmds);

      auto const &cmds = ctlCmds.commands();
      REQUIRE(cmds.size() == 6);
      REQUIRE(cmds[0].first == "fan_curve");
      REQUIRE(cmds[0].second == "0 15 15");
      REQUIRE(cmds[1].first == "fan_curve");
      REQUIRE(cmds[1].second == "1 20 20");
      REQUIRE(cmds[2].first == "fan_curve");
      REQUIRE(cmds[2].second == "2 30 30");
      REQUIRE(cmds[3].first == "fan_curve");
      REQUIRE(cmds[3].second == "3 40 40");
      REQUIRE(cmds[4].first == "fan_curve");
      REQUIRE(cmds[4].second == "4 50 50");
      REQUIRE(cmds[5].second == "c");
    }
  }
}

} // namespace Tests::AMD::OdFanCurve
