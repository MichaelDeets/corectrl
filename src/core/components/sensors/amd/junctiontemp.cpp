// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021 Juan Palacios <jpalaciosdev@gmail.com>

#include "junctiontemp.h"

#include "../gpusensorprovider.h"
#include "../graphitemprofilepart.h"
#include "../graphitemxmlparser.h"
#include "../sensor.h"
#include "common/fileutils.h"
#include "common/stringutils.h"
#include "core/components/amdutils.h"
#include "core/info/igpuinfo.h"
#include "core/info/iswinfo.h"
#include "core/info/vendor.h"
#include "core/iprofilepart.h"
#include "core/iprofilepartxmlparser.h"
#include "core/profilepartprovider.h"
#include "core/profilepartxmlparserprovider.h"
#include "core/sysfsdatasource.h"
#include <easylogging++.h>
#include <filesystem>
#include <fmt/format.h>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <units.h>
#include <utility>
#include <vector>

namespace AMD::JunctionTemp {

class Provider final : public IGPUSensorProvider::IProvider
{
 public:
  std::vector<std::unique_ptr<ISensor>>
  provideGPUSensors(IGPUInfo const &gpuInfo, ISWInfo const &swInfo) const override
  {
    std::vector<std::unique_ptr<ISensor>> sensors;

    if (gpuInfo.vendor() == Vendor::AMD) {
      auto driver = gpuInfo.info(IGPUInfo::Keys::driver);
      auto kernel = Utils::String::parseVersion(
          swInfo.info(ISWInfo::Keys::kernelVersion));

      if ((driver == "amdgpu" && kernel >= std::make_tuple(5, 3, 0))) {

        auto path =
            Utils::File::findHWMonXDirectory(gpuInfo.path().sys / "hwmon");
        if (path.has_value()) {

          auto tempInput = path.value() / "temp2_input";
          if (Utils::File::isSysFSEntryValid(tempInput)) {

            int value;
            auto data = Utils::File::readFileLines(tempInput);

            if (Utils::String::toNumber<int>(value, data.front())) {

              std::optional<std::pair<units::temperature::celsius_t,
                                      units::temperature::celsius_t>>
                  range;

              data = Utils::File::readFileLines(path.value() / "temp2_crit");
              if (!data.empty()) {
                if (Utils::String::toNumber<int>(value, data.front()) &&
                    // do not use bogus values, see #103
                    (value >= 0 && value < 150000)) {
                  range = {units::temperature::celsius_t(0),
                           units::temperature::celsius_t(value / 1000)};
                }
              }

              std::vector<std::unique_ptr<IDataSource<int>>> dataSources;
              dataSources.emplace_back(std::make_unique<SysFSDataSource<int>>(
                  tempInput, [](std::string const &data, int &output) {
                    int value;
                    Utils::String::toNumber<int>(value, data);
                    output = value / 1000;
                  }));

              sensors.emplace_back(
                  std::make_unique<Sensor<units::temperature::celsius_t, int>>(
                      AMD::JunctionTemp::ItemID, std::move(dataSources),
                      std::move(range)));
            }
            else {
              LOG(WARNING) << fmt::format("Unknown data format on {}",
                                          tempInput.string());
              LOG(ERROR) << data.front().c_str();
            }
          }
        }
      }
    }

    return sensors;
  }
};

static bool register_()
{
  GPUSensorProvider::registerProvider(
      std::make_unique<AMD::JunctionTemp::Provider>());

  ProfilePartProvider::registerProvider(AMD::JunctionTemp::ItemID, []() {
    return std::make_unique<GraphItemProfilePart>(AMD::JunctionTemp::ItemID,
                                                  "red");
  });

  ProfilePartXMLParserProvider::registerProvider(AMD::JunctionTemp::ItemID, []() {
    return std::make_unique<GraphItemXMLParser>(AMD::JunctionTemp::ItemID);
  });

  return true;
}

static bool const registered_ = register_();

} // namespace AMD::JunctionTemp
