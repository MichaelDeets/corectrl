// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "memusage.h"

#include "../gpusensorprovider.h"
#include "../graphitemprofilepart.h"
#include "../graphitemxmlparser.h"
#include "../sensor.h"
#include "common/stringutils.h"
#include "core/components/amdutils.h"
#include "core/devfsdatasource.h"
#include "core/info/igpuinfo.h"
#include "core/info/vendor.h"
#include "core/iprofilepart.h"
#include "core/iprofilepartxmlparser.h"
#include "core/profilepartprovider.h"
#include "core/profilepartxmlparserprovider.h"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <units.h>
#include <utility>
#include <vector>

namespace AMD::MemUsage {

class Provider final : public IGPUSensorProvider::IProvider
{
 public:
  std::vector<std::unique_ptr<ISensor>>
  provideGPUSensors(IGPUInfo const &gpuInfo, ISWInfo const &) const override
  {
    if (gpuInfo.vendor() != Vendor::AMD)
      return {};

    std::optional<std::pair<units::data::megabyte_t, units::data::megabyte_t>> range;

    auto memInfo = gpuInfo.info(IGPUInfo::Keys::memory);
    if (!memInfo.empty()) {
      unsigned int memorySize;
      if (Utils::String::toNumber<unsigned int>(memorySize, memInfo))
        range = {units::data::megabyte_t(0), units::data::megabyte_t(memorySize)};
    }

    std::unique_ptr<ISensor> sensor;

    auto driver = gpuInfo.info(IGPUInfo::Keys::driver);
    if (driver == "amdgpu")
      sensor = createAMDGPUSensor(gpuInfo, std::move(range));
    else if (driver == "radeon")
      sensor = createRadeonSensor(gpuInfo, std::move(range));

    if (!sensor)
      return {};

    std::vector<std::unique_ptr<ISensor>> sensors;
    sensors.emplace_back(std::move(sensor));

    return sensors;
  }

 private:
  std::unique_ptr<ISensor> createAMDGPUSensor(
      IGPUInfo const &gpuInfo,
      std::optional<std::pair<units::data::megabyte_t, units::data::megabyte_t>>
          &&range) const
  {
#if defined(AMDGPU_INFO_VRAM_USAGE)
    std::vector<std::unique_ptr<IDataSource<unsigned int>>> dataSources;
    dataSources.emplace_back(std::make_unique<DevFSDataSource<unsigned int>>(
        gpuInfo.path().dev, [](int fd) {
          std::uint64_t value;
          bool success = Utils::AMD::readAMDGPUInfo(fd, &value,
                                                    AMDGPU_INFO_VRAM_USAGE);
          return success ? value / (1024 * 1024) : 0;
        }));

    return std::make_unique<Sensor<units::data::megabyte_t, unsigned int>>(
        AMD::MemUsage::ItemID, std::move(dataSources), std::move(range));
#else
    return {};
#endif
  }

  std::unique_ptr<ISensor> createRadeonSensor(
      IGPUInfo const &gpuInfo,
      std::optional<std::pair<units::data::megabyte_t, units::data::megabyte_t>>
          &&range) const
  {
#if defined(RADEON_INFO_VRAM_USAGE)
    std::vector<std::unique_ptr<IDataSource<unsigned int>>> dataSources;
    dataSources.emplace_back(std::make_unique<DevFSDataSource<unsigned int>>(
        gpuInfo.path().dev, [](int fd) {
          std::uint64_t value;
          bool success = Utils::AMD::readRadeonInfoSensor(
              fd, &value, RADEON_INFO_VRAM_USAGE);
          return success ? value / (1024 * 1024) : 0;
        }));

    return std::make_unique<Sensor<units::data::megabyte_t, unsigned int>>(
        AMD::MemUsage::ItemID, std::move(dataSources), std::move(range));
#else
    return {};
#endif
  }
};

static bool register_()
{
  GPUSensorProvider::registerProvider(
      std::make_unique<AMD::MemUsage::Provider>());

  ProfilePartProvider::registerProvider(AMD::MemUsage::ItemID, []() {
    return std::make_unique<GraphItemProfilePart>(AMD::MemUsage::ItemID,
                                                  "forestgreen");
  });

  ProfilePartXMLParserProvider::registerProvider(AMD::MemUsage::ItemID, []() {
    return std::make_unique<GraphItemXMLParser>(AMD::MemUsage::ItemID);
  });

  return true;
}

static bool const registered_ = register_();

} // namespace AMD::MemUsage
