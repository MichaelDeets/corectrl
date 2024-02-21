// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "power.h"

#include "../gpusensorprovider.h"
#include "../graphitemprofilepart.h"
#include "../graphitemxmlparser.h"
#include "../sensor.h"
#include "common/fileutils.h"
#include "common/stringutils.h"
#include "core/components/amdutils.h"
#include "core/devfsdatasource.h"
#include "core/info/igpuinfo.h"
#include "core/info/vendor.h"
#include "core/iprofilepart.h"
#include "core/iprofilepartxmlparser.h"
#include "core/profilepartprovider.h"
#include "core/profilepartxmlparserprovider.h"
#include "core/sysfsdatasource.h"
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <units.h>
#include <utility>
#include <vector>

namespace AMD::Power {

class Provider final : public IGPUSensorProvider::IProvider
{
 public:
  std::vector<std::unique_ptr<ISensor>>
  provideGPUSensors(IGPUInfo const &gpuInfo, ISWInfo const &) const override
  {
    if (gpuInfo.vendor() != Vendor::AMD)
      return {};

    std::optional<std::vector<std::unique_ptr<IDataSource<unsigned int>>>> dataSource;
    std::optional<std::pair<units::power::microwatt_t, units::power::microwatt_t>>
        range;

    auto hwmonPath =
        Utils::File::findHWMonXDirectory(gpuInfo.path().sys / "hwmon");
    if (hwmonPath) {
      range = getRange(*hwmonPath);
      dataSource = createHWMonDataSource(*hwmonPath);
    }

    // The hwmon data source is not available. Try the ioctl data source.
    if (!dataSource)
      dataSource = createIOCtlDataSource(gpuInfo);

    if (!dataSource)
      return {};

    std::vector<std::unique_ptr<ISensor>> sensors;
    sensors.emplace_back(
        std::make_unique<Sensor<units::power::watt_t, unsigned int>>(
            AMD::Power::ItemID, std::move(*dataSource), std::move(range)));

    return sensors;
  }

 private:
  std::optional<std::vector<std::unique_ptr<IDataSource<unsigned int>>>>
  createIOCtlDataSource(IGPUInfo const &gpuInfo) const
  {
#if defined(AMDGPU_INFO_SENSOR_GPU_AVG_POWER)
    std::vector<std::unique_ptr<IDataSource<unsigned int>>> dataSource;
    dataSource.emplace_back(std::make_unique<DevFSDataSource<unsigned int>>(
        gpuInfo.path().dev, [](int fd) {
          unsigned int value;
          bool success = Utils::AMD::readAMDGPUInfoSensor(
              fd, &value, AMDGPU_INFO_SENSOR_GPU_AVG_POWER);
          return success ? value : 0;
        }));
    return dataSource;
#else
    return {};
#endif
  }

  std::optional<std::vector<std::unique_ptr<IDataSource<unsigned int>>>>
  createHWMonDataSource(std::filesystem::path const &hwmonPath) const
  {
    // Try average power.
    auto powerPath = hwmonPath / "power1_average";
    if (!Utils::File::isSysFSEntryValid(powerPath)) {
      // Some older models don't support average power.
      // Use instantaneous power instead.
      powerPath = hwmonPath / "power1_input";
      if (!Utils::File::isSysFSEntryValid(powerPath))
        return {};
    }

    unsigned long data;
    auto powerLines = Utils::File::readFileLines(powerPath);
    if (!Utils::String::toNumber<unsigned long>(data, powerLines.front()))
      return {};

    std::vector<std::unique_ptr<IDataSource<unsigned int>>> dataSource;
    dataSource.emplace_back(std::make_unique<SysFSDataSource<unsigned int>>(
        powerPath, [](std::string const &data, unsigned int &output) {
          unsigned int power;
          Utils::String::toNumber<unsigned int>(power, data);
          // Data source power unit is microwatt. Convert it to watt.
          output = power / 1000000;
        }));
    return dataSource;
  }

  std::optional<std::pair<units::power::microwatt_t, units::power::microwatt_t>>
  getRange(std::filesystem::path const &hwmonPath) const
  {
    auto power1CapMinPath = hwmonPath / "power1_cap_min";
    auto power1CapMaxPath = hwmonPath / "power1_cap_max";
    if (!(Utils::File::isSysFSEntryValid(power1CapMinPath) &&
          Utils::File::isSysFSEntryValid(power1CapMaxPath)))
      return {};

    unsigned long power1CapMinValue = 0;
    unsigned long power1CapMaxValue = 0;
    auto power1CapMinLines = Utils::File::readFileLines(power1CapMinPath);
    auto power1CapMaxLines = Utils::File::readFileLines(power1CapMaxPath);
    if (!(Utils::String::toNumber<unsigned long>(power1CapMinValue,
                                                 power1CapMinLines.front()) &&
          Utils::String::toNumber<unsigned long>(power1CapMaxValue,
                                                 power1CapMaxLines.front())))
      return {};

    if (power1CapMaxValue <= power1CapMinValue)
      return {};

    return std::make_optional(
        std::make_pair(units::power::microwatt_t(power1CapMinValue),
                       units::power::microwatt_t(power1CapMaxValue)));
  }
};

static bool register_()
{
  GPUSensorProvider::registerProvider(std::make_unique<AMD::Power::Provider>());

  ProfilePartProvider::registerProvider(AMD::Power::ItemID, []() {
    return std::make_unique<GraphItemProfilePart>(AMD::Power::ItemID, "gold");
  });

  ProfilePartXMLParserProvider::registerProvider(AMD::Power::ItemID, []() {
    return std::make_unique<GraphItemXMLParser>(AMD::Power::ItemID);
  });

  return true;
}

static bool const registered_ = register_();

} // namespace AMD::Power
