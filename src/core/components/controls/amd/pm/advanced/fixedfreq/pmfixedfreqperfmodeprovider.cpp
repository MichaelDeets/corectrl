// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "pmfixedfreqperfmodeprovider.h"

#include "../../pmperfmodeprovider.h"
#include "common/fileutils.h"
#include "common/stringutils.h"
#include "core/components/amdutils.h"
#include "core/components/controls/amd/pm/handlers/ppdpmhandler.h"
#include "core/info/igpuinfo.h"
#include "core/info/iswinfo.h"
#include "core/sysfsdatasource.h"
#include "pmfixedfreq.h"
#include <filesystem>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <tuple>
#include <vector>

std::vector<std::unique_ptr<IControl>>
AMD::PMFixedFreqPerfModeProvider::provideGPUControls(IGPUInfo const &gpuInfo,
                                                     ISWInfo const &swInfo) const
{
  if (gpuInfo.vendor() != Vendor::AMD)
    return {};

  auto driver = gpuInfo.info(IGPUInfo::Keys::driver);
  if (driver != "amdgpu")
    return {};

  auto kernel =
      Utils::String::parseVersion(swInfo.info(ISWInfo::Keys::kernelVersion));
  if (kernel < std::make_tuple(4, 18, 0))
    return {};

  auto perfLevel = gpuInfo.path().sys / "power_dpm_force_performance_level";
  auto dpmSclk = gpuInfo.path().sys / "pp_dpm_sclk";
  auto dpmMclk = gpuInfo.path().sys / "pp_dpm_mclk";
  if (!(Utils::File::isSysFSEntryValid(perfLevel) &&
        Utils::File::isSysFSEntryValid(dpmSclk) &&
        Utils::File::isSysFSEntryValid(dpmMclk)))
    return {};

  auto dpmSclkLines = Utils::File::readFileLines(dpmSclk);
  if (!Utils::AMD::parseDPMStates(dpmSclkLines)) {
    SPDLOG_WARN("Unknown data format on {}", dpmSclk.string());
    for (auto const &line : dpmSclkLines)
      SPDLOG_DEBUG(line);
    return {};
  }

  auto dpmMclkLines = Utils::File::readFileLines(dpmMclk);
  if (!Utils::AMD::parseDPMStates(dpmMclkLines)) {
    SPDLOG_WARN("Unknown data format on {}", dpmMclk.string());
    for (auto const &line : dpmMclkLines)
      SPDLOG_DEBUG(line);
    return {};
  }

  std::vector<std::unique_ptr<IControl>> controls;
  controls.emplace_back(std::make_unique<AMD::PMFixedFreq>(
      std::make_unique<PpDpmHandler>(
          std::make_unique<SysFSDataSource<std::string>>(perfLevel),
          std::make_unique<SysFSDataSource<std::vector<std::string>>>(dpmSclk)),
      std::make_unique<PpDpmHandler>(
          std::make_unique<SysFSDataSource<std::string>>(perfLevel),
          std::make_unique<SysFSDataSource<std::vector<std::string>>>(dpmMclk))));

  return controls;
}

bool const AMD::PMFixedFreqPerfModeProvider::registered_ =
    AMD::PMPerfModeProvider::registerProvider(
        std::make_unique<AMD::PMFixedFreqPerfModeProvider>());
