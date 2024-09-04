// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "pmpowerprofileprovider.h"

#include "../pmadvancedprovider.h"
#include "common/fileutils.h"
#include "core/components/amdutils.h"
#include "core/info/igpuinfo.h"
#include "core/sysfsdatasource.h"
#include "pmpowerprofile.h"
#include <filesystem>
#include <memory>
#include <optional>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

std::vector<std::unique_ptr<IControl>>
AMD::PMPowerProfileProvider::provideGPUControls(IGPUInfo const &gpuInfo,
                                                ISWInfo const &) const
{
  if (gpuInfo.vendor() != Vendor::AMD)
    return {};

  auto driver = gpuInfo.info(IGPUInfo::Keys::driver);
  if (driver != "amdgpu")
    return {};

  auto perfLevel = gpuInfo.path().sys / "power_dpm_force_performance_level";
  auto profileMode = gpuInfo.path().sys / "pp_power_profile_mode";
  if (!(Utils::File::isSysFSEntryValid(perfLevel) &&
        Utils::File::isSysFSEntryValid(profileMode)))
    return {};

  auto modeLines = Utils::File::readFileLines(profileMode);
  auto columnarData = Utils::AMD::isPowerProfileModeDataColumnar(modeLines);
  auto modes = columnarData
                   ? Utils::AMD::parsePowerProfileModeModesColumnar(modeLines)
                   : Utils::AMD::parsePowerProfileModeModes(modeLines);
  if (!modes) {
    SPDLOG_WARN("Unknown data format on {}", profileMode.string());
    for (auto const &line : modeLines)
      SPDLOG_DEBUG(line);
    return {};
  }

  auto indexFn = columnarData
                 ? [](std::vector<std::string> const &data, std::optional<int> &output) {
    output = Utils::AMD::parsePowerProfileModeCurrentModeIndexColumnar(data);
  }
                 : [](std::vector<std::string> const &data, std::optional<int> &output) {
    output = Utils::AMD::parsePowerProfileModeCurrentModeIndex(data);
  };

  std::vector<std::unique_ptr<IControl>> controls;
  controls.emplace_back(std::make_unique<AMD::PMPowerProfile>(
      std::make_unique<SysFSDataSource<std::string>>(perfLevel),
      std::make_unique<SysFSDataSource<std::optional<int>, std::vector<std::string>>>(
          profileMode, std::move(indexFn)),
      *modes));

  return controls;
}

bool const AMD::PMPowerProfileProvider::registered_ =
    AMD::PMAdvancedProvider::registerProvider(
        std::make_unique<AMD::PMPowerProfileProvider>());
