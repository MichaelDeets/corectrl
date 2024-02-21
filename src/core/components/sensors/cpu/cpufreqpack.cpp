// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "cpufreqpack.h"

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
#include <algorithm>
#include <filesystem>
#include <memory>
#include <optional>
#include <spdlog/spdlog.h>
#include <string>
#include <units.h>
#include <utility>
#include <vector>

namespace CPUFreqPack {

class Provider final : public ICPUSensorProvider::IProvider
{
 public:
  std::vector<std::unique_ptr<ISensor>>
  provideCPUSensors(ICPUInfo const &cpuInfo, ISWInfo const &) const override
  {
    if (!Utils::File::isDirectoryPathValid("/sys/devices/system/cpu/cpufreq"))
      return {};

    auto &executionUnits = cpuInfo.executionUnits();
    if (executionUnits.empty())
      return {};

    std::optional<
        std::pair<units::frequency::megahertz_t, units::frequency::megahertz_t>>
        range;

    auto minFreqPath = executionUnits.front().sysPath /
                       "cpufreq/cpuinfo_min_freq";
    auto maxFreqPath = executionUnits.front().sysPath /
                       "cpufreq/cpuinfo_max_freq";
    if (Utils::File::isSysFSEntryValid(minFreqPath) &&
        Utils::File::isSysFSEntryValid(maxFreqPath)) {
      auto minFreqLines = Utils::File::readFileLines(minFreqPath);
      auto maxFreqLines = Utils::File::readFileLines(maxFreqPath);

      unsigned int minFreq{0};
      unsigned int maxFreq{0};
      if (Utils::String::toNumber<unsigned int>(minFreq, minFreqLines.front()) &&
          Utils::String::toNumber<unsigned int>(maxFreq, maxFreqLines.front())) {
        if (minFreq < maxFreq)
          range = {units::frequency::kilohertz_t(minFreq),
                   units::frequency::kilohertz_t(maxFreq)};
      }
    }

    std::vector<std::unique_ptr<IDataSource<unsigned int>>> dataSources;
    for (auto const &executionUnit : cpuInfo.executionUnits()) {
      auto curFreqPath = executionUnit.sysPath / "cpufreq/scaling_cur_freq";
      if (!Utils::File::isSysFSEntryValid(curFreqPath))
        continue;

      unsigned int value;
      auto curFreqLines = Utils::File::readFileLines(curFreqPath);
      if (!Utils::String::toNumber<unsigned int>(value, curFreqLines.front())) {
        SPDLOG_WARN("Unknown data format on {}", curFreqPath.string());
        SPDLOG_DEBUG(curFreqLines.front());
        continue;
      }

      dataSources.emplace_back(std::make_unique<SysFSDataSource<unsigned int>>(
          curFreqPath, [](std::string const &data, unsigned int &output) {
            Utils::String::toNumber<unsigned int>(output, data);
          }));
    }

    if (dataSources.empty())
      return {};

    std::vector<std::unique_ptr<ISensor>> sensors;
    sensors.emplace_back(
        std::make_unique<Sensor<units::frequency::megahertz_t, unsigned int>>(
            CPUFreqPack::ItemID, std::move(dataSources), std::move(range),
            [](std::vector<unsigned int> const &input) {
              auto maxIter = std::max_element(input.cbegin(), input.cend());
              if (maxIter != input.cend()) {
                units::frequency::kilohertz_t maxKHz(*maxIter);
                return maxKHz.convert<units::frequency::megahertz>()
                    .to<unsigned int>();
              }
              else
                return 0u;
            }));

    return sensors;
  }
};

static bool register_()
{
  CPUSensorProvider::registerProvider(std::make_unique<CPUFreqPack::Provider>());

  ProfilePartProvider::registerProvider(CPUFreqPack::ItemID, []() {
    return std::make_unique<GraphItemProfilePart>(CPUFreqPack::ItemID,
                                                  "fuchsia");
  });

  ProfilePartXMLParserProvider::registerProvider(CPUFreqPack::ItemID, []() {
    return std::make_unique<GraphItemXMLParser>(CPUFreqPack::ItemID);
  });

  return true;
}

static bool const registered_ = register_();

} // namespace CPUFreqPack
