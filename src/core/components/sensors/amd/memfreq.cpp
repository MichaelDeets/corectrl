// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "memfreq.h"

#include "../gpusensorprovider.h"
#include "../graphitemprofilepart.h"
#include "../graphitemxmlparser.h"
#include "../sensor.h"
#include "common/fileutils.h"
#include "core/components/amdutils.h"
#include "core/devfsdatasource.h"
#include "core/info/igpuinfo.h"
#include "core/info/vendor.h"
#include "core/iprofilepart.h"
#include "core/iprofilepartxmlparser.h"
#include "core/profilepartprovider.h"
#include "core/profilepartxmlparserprovider.h"
#include <algorithm>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <units.h>
#include <utility>
#include <vector>

namespace AMD::MemFreq {

class Provider final : public IGPUSensorProvider::IProvider
{
 public:
  std::vector<std::unique_ptr<ISensor>>
  provideGPUSensors(IGPUInfo const &gpuInfo, ISWInfo const &) const override
  {
    if (gpuInfo.vendor() != Vendor::AMD)
      return {};

    std::unique_ptr<ISensor> sensor;

    auto driver = gpuInfo.info(IGPUInfo::Keys::driver);
    if (driver == "amdgpu")
      sensor = createAMDGPUSensor(gpuInfo);
    else if (driver == "radeon")
      sensor = createRadeonSensor(gpuInfo);

    if (!sensor)
      return {};

    std::vector<std::unique_ptr<ISensor>> sensors;
    sensors.emplace_back(std::move(sensor));

    return sensors;
  }

 private:
  std::unique_ptr<ISensor> createAMDGPUSensor(IGPUInfo const &gpuInfo) const
  {
#if defined(AMDGPU_INFO_SENSOR_GFX_MCLK)
    std::optional<
        std::pair<units::frequency::megahertz_t, units::frequency::megahertz_t>>
        range;

    // get range from dpm mclk states (4.6+)
    auto dpmData =
        Utils::File::readFileLines(gpuInfo.path().sys / "pp_dpm_mclk");
    auto memStates = Utils::AMD::parseDPMStates(dpmData);
    if (memStates.has_value() && !memStates->empty())
      range = {memStates->front().second, memStates->back().second};

    // expand range using dpm sclk states
    dpmData = Utils::File::readFileLines(gpuInfo.path().sys / "pp_dpm_sclk");
    auto gpuStates = Utils::AMD::parseDPMStates(dpmData);
    if (gpuStates.has_value() && !gpuStates->empty()) {
      if (range.has_value())
        range = {std::min(gpuStates->front().second, range->first),
                 std::max(gpuStates->back().second, range->second)};
      else // memory clock range not available
        range = {gpuStates->front().second, gpuStates->back().second};
    }

    std::vector<std::unique_ptr<IDataSource<unsigned int>>> dataSources;
    dataSources.emplace_back(std::make_unique<DevFSDataSource<unsigned int>>(
        gpuInfo.path().dev, [](int fd) {
          unsigned int value;
          bool success = Utils::AMD::readAMDGPUInfoSensor(
              fd, &value, AMDGPU_INFO_SENSOR_GFX_MCLK);
          return success ? value : 0;
        }));

    return std::make_unique<Sensor<units::frequency::megahertz_t, unsigned int>>(
        AMD::MemFreq::ItemID, std::move(dataSources), std::move(range));
#else
    return {};
#endif
  }

  std::unique_ptr<ISensor> createRadeonSensor(IGPUInfo const &gpuInfo) const
  {
#if defined(RADEON_INFO_CURRENT_GPU_MCLK)
    std::vector<std::unique_ptr<IDataSource<unsigned int>>> dataSources;
    dataSources.emplace_back(std::make_unique<DevFSDataSource<unsigned int>>(
        gpuInfo.path().dev, [](int fd) {
          unsigned int value;
          bool success = Utils::AMD::readRadeonInfoSensor(
              fd, &value, RADEON_INFO_CURRENT_GPU_MCLK);
          return success ? value : 0;
        }));

    return std::make_unique<Sensor<units::frequency::megahertz_t, unsigned int>>(
        AMD::MemFreq::ItemID, std::move(dataSources));
#else
    return {};
#endif
  }
};

static bool register_()
{
  GPUSensorProvider::registerProvider(std::make_unique<AMD::MemFreq::Provider>());

  ProfilePartProvider::registerProvider(AMD::MemFreq::ItemID, []() {
    return std::make_unique<GraphItemProfilePart>(AMD::MemFreq::ItemID,
                                                  "orchid");
  });

  ProfilePartXMLParserProvider::registerProvider(AMD::MemFreq::ItemID, []() {
    return std::make_unique<GraphItemXMLParser>(AMD::MemFreq::ItemID);
  });

  return true;
}

static bool const registered_ = register_();

} // namespace AMD::MemFreq
