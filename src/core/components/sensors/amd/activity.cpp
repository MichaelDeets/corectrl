// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "activity.h"

#include "../gpusensorprovider.h"
#include "../graphitemprofilepart.h"
#include "../graphitemxmlparser.h"
#include "../sensor.h"
#include "core/components/amdutils.h"
#include "core/devfsdatasource.h"
#include "core/info/igpuinfo.h"
#include "core/info/vendor.h"
#include "core/iprofilepart.h"
#include "core/iprofilepartxmlparser.h"
#include "core/profilepartprovider.h"
#include "core/profilepartxmlparserprovider.h"
#include <memory>
#include <string>
#include <units.h>
#include <utility>
#include <vector>

namespace AMD::Activity {

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
      sensor = createSensor(gpuInfo);

    if (!sensor)
      return {};

    std::vector<std::unique_ptr<ISensor>> sensors;
    sensors.emplace_back(std::move(sensor));

    return sensors;
  }

 private:
  std::unique_ptr<ISensor> createSensor(IGPUInfo const &gpuInfo) const
  {
#if defined(AMDGPU_INFO_SENSOR_GPU_LOAD)
    std::vector<std::unique_ptr<IDataSource<unsigned int>>> dataSources;
    dataSources.emplace_back(std::make_unique<DevFSDataSource<unsigned int>>(
        gpuInfo.path().dev, [](int fd) {
          unsigned int value;
          bool success = Utils::AMD::readAMDGPUInfoSensor(
              fd, &value, AMDGPU_INFO_SENSOR_GPU_LOAD);
          return success ? value : 0;
        }));

    return std::make_unique<Sensor<units::dimensionless::scalar_t, unsigned int>>(
        AMD::Activity::ItemID, std::move(dataSources),
        std::make_pair(units::dimensionless::scalar_t(0),
                       units::dimensionless::scalar_t(100)));
#else
    return {};
#endif
  }
};

static bool register_()
{
  GPUSensorProvider::registerProvider(
      std::make_unique<AMD::Activity::Provider>());

  ProfilePartProvider::registerProvider(AMD::Activity::ItemID, []() {
    return std::make_unique<GraphItemProfilePart>(AMD::Activity::ItemID,
                                                  "yellowgreen");
  });

  ProfilePartXMLParserProvider::registerProvider(AMD::Activity::ItemID, []() {
    return std::make_unique<GraphItemXMLParser>(AMD::Activity::ItemID);
  });

  return true;
}

static bool const registered_ = register_();

} // namespace AMD::Activity
