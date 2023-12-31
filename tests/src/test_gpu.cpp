// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "common/commandqueuestub.h"
#include "common/controlmock.h"
#include "common/sensormock.h"
#include "core/components/gpu.h"
#include "core/info/igpuinfo.h"

extern template struct trompeloeil::reporter<trompeloeil::specialized>;

namespace Tests::GPU {

class GPUInfoStub : public IGPUInfo
{
 public:
  Vendor vendor() const override
  {
    return Vendor::AMD;
  }

  int index() const override
  {
    return 123;
  }

  IGPUInfo::Path const &path() const override
  {
    return path_;
  }

  std::vector<std::string> keys() const override
  {
    return std::vector<std::string>{"info_k1"};
  }

  std::string info(std::string_view) const override
  {
    return "info_1";
  }

  bool hasCapability(std::string_view) const override
  {
    return true;
  }

  void initialize(std::vector<std::unique_ptr<IGPUInfo::IProvider>> const &,
                  IHWIDTranslator const &) override
  {
  }

 private:
  IGPUInfo::Path path_{"_sys_", "_dev_"};
};

class GPUImporterStub : public IGPU::Importer
{
 public:
  std::optional<std::reference_wrapper<Importable::Importer>>
  provideImporter(Item const &) override
  {
    return *this;
  }

  bool provideActive() const override
  {
    return false;
  }
};

class GPUExporterMock : public IGPU::Exporter
{
 public:
  MAKE_MOCK1(takeActive, void(bool), override);
  MAKE_MOCK1(takeInfo, void(IGPUInfo const &), override);
  MAKE_MOCK1(takeSensor, void(ISensor const &), override);
  MAKE_MOCK1(
      provideExporter,
      std::optional<std::reference_wrapper<Exportable::Exporter>>(Item const &),
      override);
};

TEST_CASE("GPU tests", "[GPU]")
{
  CommandQueueStub ctlCmds;

  auto info = std::make_unique<GPUInfoStub>();
  auto &infoStub = *info;

  std::vector<std::unique_ptr<IControl>> controls;
  controls.emplace_back(std::make_unique<ControlMock>());
  auto &controlMock = static_cast<ControlMock &>(*controls[0]);

  std::vector<std::unique_ptr<ISensor>> sensors;
  sensors.emplace_back(std::make_unique<SensorMock>());
  auto &sensorMock = static_cast<SensorMock &>(*sensors[0]);

  ::GPU ts(std::move(info), std::move(controls), std::move(sensors));

  SECTION("Has IGPU ID")
  {
    REQUIRE(ts.ID() == IGPU::ItemID);
  }

  SECTION("Is active by default")
  {
    REQUIRE(ts.active());

    SECTION("Can be deactivated")
    {
      ts.activate(false);

      REQUIRE_FALSE(ts.active());

      SECTION("Can be activated")
      {
        ts.activate(true);

        REQUIRE(ts.active());
      }
    }
  }

  SECTION("Has a unique key in a system component scope")
  {
    // 'GPU' + gpu index combination seems to be good enough to differentiate
    // between system components.
    REQUIRE_THAT(ts.key(), Catch::Contains("GPU") && Catch::Contains("123"));
  }

  SECTION("Its GPU information can be retrieved")
  {
    REQUIRE(&ts.info() == &infoStub);
  }

  SECTION("GPU description and information can be retrieved")
  {
    auto [gpuDesc, infos] = ts.componentInfo();
    REQUIRE_THAT(gpuDesc, Catch::Contains("GPU") && Catch::Contains("123"));
    REQUIRE_FALSE(infos.empty());
    auto &[infoKey, info] = infos[0];
    REQUIRE(infoKey == "info_k1");
    REQUIRE(info == "info_1");
  }

  SECTION("Update its sensors")
  {
    std::string const sensorid("sensorid");
    ALLOW_CALL(sensorMock, ID()).LR_RETURN(sensorid);
    REQUIRE_CALL(sensorMock, update());

    std::unordered_map<std::string, std::unordered_set<std::string>> ignored;
    ts.updateSensors(ignored);
  }

  SECTION("Does not update ignored sensors")
  {
    std::string const sensorid("sensorid");
    std::unordered_map<std::string, std::unordered_set<std::string>> ignored;
    ignored[ts.key()] = {};
    ignored[ts.key()].emplace(sensorid);

    ALLOW_CALL(sensorMock, ID()).LR_RETURN(sensorid);
    FORBID_CALL(sensorMock, update());
    ts.updateSensors(ignored);
  }

  SECTION("Pre-init its controls")
  {
    REQUIRE_CALL(controlMock, preInit(trompeloeil::_));
    ts.preInit(ctlCmds);
  }

  SECTION("Post-init its controls")
  {
    REQUIRE_CALL(controlMock, postInit(trompeloeil::_));
    ts.postInit(ctlCmds);
  }

  SECTION("Init controls")
  {
    REQUIRE_CALL(controlMock, init());
    ts.init();
  }

  SECTION("Only clear and sync controls when is active")
  {
    ALLOW_CALL(sensorMock, update());

    REQUIRE_CALL(controlMock, clean(trompeloeil::_));
    REQUIRE_CALL(controlMock, sync(trompeloeil::_));
    ts.activate(true);
    ts.sync(ctlCmds);

    FORBID_CALL(controlMock, clean(trompeloeil::_));
    FORBID_CALL(controlMock, sync(trompeloeil::_));
    ts.activate(false);
    ts.sync(ctlCmds);
  }

  SECTION("Imports its state and controls")
  {
    REQUIRE_CALL(controlMock, importWith(trompeloeil::_));

    ts.activate(true);
    GPUImporterStub i;
    ts.importWith(i);

    REQUIRE_FALSE(ts.active());
  }

  SECTION("Exports its state, sensors and controls")
  {
    GPUExporterMock e;
    ALLOW_CALL(e, provideExporter(trompeloeil::_)).LR_RETURN(e);
    REQUIRE_CALL(e, takeActive(true));
    REQUIRE_CALL(e, takeInfo(trompeloeil::_)).LR_WITH(&_1 == &infoStub);
    REQUIRE_CALL(e, takeSensor(trompeloeil::_)).LR_WITH(&_1 == &sensorMock);

    REQUIRE_CALL(controlMock, exportWith(trompeloeil::_));

    ts.activate(true);
    ts.exportWith(e);
  }
}

} // namespace Tests::GPU
