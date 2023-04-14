// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2022 Milan Cermak <krysmanta@post.cz>

#include "cpucoretemp.h"

#include "../cpusensorprovider.h"
#include "../graphitemprofilepart.h"
#include "../graphitemxmlparser.h"
#include "../sensor.h"
#include "common/fileutils.h"
#include "common/stringutils.h"
#include "core/idatasource.h"
#include "core/info/icpuinfo.h"
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

namespace CPUCoreTemp {

class Provider final : public ICPUSensorProvider::IProvider
{
 public:
  std::vector<std::unique_ptr<ISensor>>
  provideCPUSensors(ICPUInfo const &cpuInfo, ISWInfo const &) const override
  {
    std::vector<std::unique_ptr<ISensor>> sensors;
    auto hwmonPath = fmt::format("/sys/devices/platform/coretemp.{}/hwmon",
                                 cpuInfo.physicalId());
    auto path = Utils::File::findHWMonXDirectory(hwmonPath);
    if (path.has_value()) {

      std::optional<
          std::pair<units::temperature::celsius_t, units::temperature::celsius_t>>
          range;

      auto critFilePath = path.value() / "temp1_crit";
      if (Utils::File::isFilePathValid(critFilePath)) {

        auto data = Utils::File::readFileLines(critFilePath);
        if (!data.empty()) {
          int value;
          if (Utils::String::toNumber<int>(value, data.front()) &&
              // do not use bogus values, see #103
              (value >= 0 && value < 150000)) {
            range = {units::temperature::celsius_t(0),
                     units::temperature::celsius_t(value / 1000)};
          }
        }
      }

      auto tempInput = path.value() / "temp1_input";
      if (Utils::File::isSysFSEntryValid(tempInput)) {

        int value;
        auto tempInputLines = Utils::File::readFileLines(tempInput);

        if (Utils::String::toNumber<int>(value, tempInputLines.front())) {

          std::vector<std::unique_ptr<IDataSource<int>>> dataSources;
          dataSources.emplace_back(std::make_unique<SysFSDataSource<int>>(
              tempInput, [](std::string const &data, int &output) {
                int value;
                Utils::String::toNumber<int>(value, data);
                output = value / 1000;
              }));

          sensors.emplace_back(
              std::make_unique<Sensor<units::temperature::celsius_t, int>>(
                  CPUCoreTemp::ItemID, std::move(dataSources), std::move(range)));
        }
        else {
          LOG(WARNING) << fmt::format("Unknown data format on {}",
                                      tempInput.string());
          LOG(ERROR) << tempInputLines.front().c_str();
        }
      }
    }

    return sensors;
  }
};

static bool register_()
{
  CPUSensorProvider::registerProvider(std::make_unique<CPUCoreTemp::Provider>());

  ProfilePartProvider::registerProvider(CPUCoreTemp::ItemID, []() {
    return std::make_unique<GraphItemProfilePart>(CPUCoreTemp::ItemID,
                                                  "crimson");
  });

  ProfilePartXMLParserProvider::registerProvider(CPUCoreTemp::ItemID, []() {
    return std::make_unique<GraphItemXMLParser>(CPUCoreTemp::ItemID);
  });

  return true;
}

static bool const registered_ = register_();

} // namespace CPUCoreTemp
