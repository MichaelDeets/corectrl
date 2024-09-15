// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "cpufreqprovider.h"

#include "common/fileutils.h"
#include "common/stringutils.h"
#include "core/info/icpuinfo.h"
#include "core/sysfsdatasource.h"
#include "cpufreq.h"
#include "cpufreqmodeprovider.h"
#include "handlers/epphandler.h"
#include <algorithm>
#include <filesystem>
#include <utility>

std::vector<std::unique_ptr<IControl>>
CPUFreqProvider::provideCPUControls(ICPUInfo const &cpuInfo, ISWInfo const &) const
{
  if (!Utils::File::isDirectoryPathValid("/sys/devices/system/cpu/cpufreq"))
    return {};

  auto &executionUnits = cpuInfo.executionUnits();
  if (executionUnits.empty())
    return {};

  auto governors = availableGovernors(cpuInfo);
  if (governors.empty())
    return {};

  auto governor = defaultGovernor(cpuInfo, governors);
  auto scalingGovernorDataSources = createScalingGovernorDataSources(cpuInfo);

  if (scalingGovernorDataSources.empty())
    return {};

  std::vector<std::unique_ptr<IControl>> controls;
  controls.emplace_back(std::make_unique<CPUFreq>(
      std::move(governors), governor, std::move(scalingGovernorDataSources),
      createEPPHandler(cpuInfo)));

  return controls;
}

std::vector<std::string>
CPUFreqProvider::availableGovernors(ICPUInfo const &cpuInfo) const
{
  std::string availableGovernorsPath{"cpufreq/scaling_available_governors"};

  // get available governors from the first execution unit
  auto unitAvailableGovernorsPath = cpuInfo.executionUnits().front().sysPath /
                                    availableGovernorsPath;

  if (!Utils::File::isSysFSEntryValid(unitAvailableGovernorsPath))
    return {};

  auto lines = Utils::File::readFileLines(unitAvailableGovernorsPath);
  auto governors = Utils::String::split(lines.front());

  // remove not implemented userspace option (see #175)
  std::erase(governors, "userspace");

  return governors;
}

std::string CPUFreqProvider::defaultGovernor(
    ICPUInfo const &cpuInfo, std::vector<std::string> const &governors) const
{
  std::string scalingDriverPath{"cpufreq/scaling_driver"};
  std::string fallbackGovernor = governors.front();

  // get scaling driver from the first execution unit
  auto unitScalingDriverPath = cpuInfo.executionUnits().front().sysPath /
                               scalingDriverPath;
  if (!Utils::File::isSysFSEntryValid(unitScalingDriverPath))
    return fallbackGovernor;

  auto lines = Utils::File::readFileLines(unitScalingDriverPath);
  if (lines.empty())
    return fallbackGovernor;

  std::string governor("ondemand");

  auto driver = lines.front();
  if (driver == "intel_pstate")
    governor = "powersave";

  // clamp governor into available governors
  auto iter = std::find_if(governors.cbegin(), governors.cend(),
                           [&](auto const &availableGovernor) {
                             return governor == availableGovernor;
                           });
  if (iter == governors.cend())
    return fallbackGovernor;

  return governor;
}

std::vector<std::unique_ptr<IDataSource<std::string>>>
CPUFreqProvider::createScalingGovernorDataSources(ICPUInfo const &cpuInfo) const
{
  std::vector<std::unique_ptr<IDataSource<std::string>>> scalingGovernorDataSources;

  std::string scalingGovernorPath{"cpufreq/scaling_governor"};
  for (auto const &executionUnit : cpuInfo.executionUnits()) {

    auto unitScalingGovernorPath = executionUnit.sysPath / scalingGovernorPath;
    if (!Utils::File::isSysFSEntryValid(unitScalingGovernorPath))
      continue;

    scalingGovernorDataSources.emplace_back(
        std::make_unique<SysFSDataSource<std::string>>(unitScalingGovernorPath));
  }

  return scalingGovernorDataSources;
}

std::unique_ptr<IEPPHandler>
CPUFreqProvider::createEPPHandler(ICPUInfo const &cpuInfo) const
{
  auto avaiableEPPHintsDataSource = createAvailableHintsDataSource(cpuInfo);
  if (!avaiableEPPHintsDataSource)
    return {};

  auto eppHintDataSources = createHintDataSources(cpuInfo);
  if (eppHintDataSources.empty())
    return {};

  return std::make_unique<EPPHandler>(std::move(avaiableEPPHintsDataSource),
                                      std::move(eppHintDataSources));
}

std::unique_ptr<IDataSource<std::string>>
CPUFreqProvider::createAvailableHintsDataSource(ICPUInfo const &cpuInfo) const
{
  std::string availableGovernorsPath{
      "cpufreq/energy_performance_available_preferences"};

  // available hints will be read from the first execution unit
  auto availableHintsPath = cpuInfo.executionUnits().front().sysPath /
                            availableGovernorsPath;

  if (!Utils::File::isSysFSEntryValid(availableHintsPath))
    return {};

  return std::make_unique<SysFSDataSource<std::string>>(availableHintsPath);
}

std::vector<std::unique_ptr<IDataSource<std::string>>>
CPUFreqProvider::createHintDataSources(ICPUInfo const &cpuInfo) const
{
  std::vector<std::unique_ptr<IDataSource<std::string>>> hintDataSources;

  std::string hintPath{"cpufreq/energy_performance_preference"};
  for (auto const &executionUnit : cpuInfo.executionUnits()) {

    auto unitHintPath = executionUnit.sysPath / hintPath;
    if (!Utils::File::isSysFSEntryValid(unitHintPath))
      continue;

    hintDataSources.emplace_back(
        std::make_unique<SysFSDataSource<std::string>>(unitHintPath));
  }

  return hintDataSources;
}

bool const CPUFreqProvider::registered_ =
    CPUFreqModeProvider::registerProvider(std::make_unique<CPUFreqProvider>());
