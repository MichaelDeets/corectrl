// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2022 Milan Cermak <krysmanta@post.cz>
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#include "cpuusage.h"

#include "../cpusensorprovider.h"
#include "../graphitemprofilepart.h"
#include "../graphitemxmlparser.h"
#include "../sensor.h"
#include "core/components/cpuutils.h"
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
#include <string>
#include <units.h>
#include <utility>
#include <vector>

namespace CPUUsage {

class CPUUsageDataSource : public IDataSource<unsigned int>
{
 public:
  CPUUsageDataSource() noexcept
  : procStatDataSource_("/proc/stat")
  {
  }

  std::string source() const override
  {
    return procStatDataSource_.source();
  }

  bool read(unsigned int &usedCPU) override
  {
    if (procStatDataSource_.read(rawData_)) {

      auto stat = Utils::CPU::parseProcStat(rawData_);
      rawData_.clear();

      if (stat.has_value()) {

        if (prevStat_.has_value())
          usedCPU = Utils::CPU::computeCPUUsage(*prevStat_, *stat);
        else
          usedCPU = 0;

        prevStat_.swap(stat);
        return true;
      }
    }

    return false;
  }

 private:
  SysFSDataSource<std::vector<std::string>> procStatDataSource_;
  std::vector<std::string> rawData_;
  std::optional<Utils::CPU::Stat> prevStat_;
};

class Provider final : public ICPUSensorProvider::IProvider
{
 public:
  std::vector<std::unique_ptr<ISensor>>
  provideCPUSensors(ICPUInfo const &, ISWInfo const &) const override
  {
    std::vector<std::unique_ptr<ISensor>> sensors;

    std::vector<std::unique_ptr<IDataSource<unsigned int>>> dataSources;
    dataSources.emplace_back(std::make_unique<CPUUsageDataSource>());

    sensors.emplace_back(
        std::make_unique<Sensor<units::dimensionless::scalar_t, unsigned int>>(
            CPUUsage::ItemID, std::move(dataSources),
            std::make_pair(units::dimensionless::scalar_t(0),
                           units::dimensionless::scalar_t(100))));

    return sensors;
  }
};

static bool register_()
{
  CPUSensorProvider::registerProvider(std::make_unique<CPUUsage::Provider>());

  ProfilePartProvider::registerProvider(CPUUsage::ItemID, []() {
    return std::make_unique<GraphItemProfilePart>(CPUUsage::ItemID,
                                                  "yellowgreen");
  });

  ProfilePartXMLParserProvider::registerProvider(CPUUsage::ItemID, []() {
    return std::make_unique<GraphItemXMLParser>(CPUUsage::ItemID);
  });

  return true;
}

static bool const registered_ = register_();

} // namespace CPUUsage
