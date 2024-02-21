// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "pmfreqodprovider.h"

#include "../pmoverclockprovider.h"
#include "common/fileutils.h"
#include "common/stringutils.h"
#include "core/components/amdutils.h"
#include "core/info/igpuinfo.h"
#include "core/info/iswinfo.h"
#include "core/sysfsdatasource.h"
#include "pmfreqod.h"
#include <filesystem>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <tuple>
#include <vector>

std::vector<std::unique_ptr<IControl>>
AMD::PMFreqOdProvider::provideGPUControls(IGPUInfo const &gpuInfo,
                                          ISWInfo const &swInfo) const
{
  if (gpuInfo.vendor() != Vendor::AMD)
    return {};

  auto kernel =
      Utils::String::parseVersion(swInfo.info(ISWInfo::Keys::kernelVersion));
  auto driver = gpuInfo.info(IGPUInfo::Keys::driver);

  if (!(driver == "amdgpu" && (kernel >= std::make_tuple(4, 8, 0) &&
                               kernel < std::make_tuple(4, 17, 0))))
    return {};

  auto sclkOd = gpuInfo.path().sys / "pp_sclk_od";
  auto mclkOd = gpuInfo.path().sys / "pp_mclk_od";
  auto dpmSclk = gpuInfo.path().sys / "pp_dpm_sclk";
  auto dpmMclk = gpuInfo.path().sys / "pp_dpm_mclk";
  if (!(Utils::File::isSysFSEntryValid(sclkOd) &&
        Utils::File::isSysFSEntryValid(mclkOd) &&
        Utils::File::isSysFSEntryValid(dpmSclk) &&
        Utils::File::isSysFSEntryValid(dpmMclk)))
    return {};

  unsigned int odValue;

  auto sclkOdLines = Utils::File::readFileLines(sclkOd);
  if (!Utils::String::toNumber<unsigned int>(odValue, sclkOdLines.front())) {
    SPDLOG_WARN("Unknown data format on {}", sclkOd.string());
    SPDLOG_DEBUG(sclkOdLines.front());
    return {};
  }

  auto mclkOdLines = Utils::File::readFileLines(mclkOd);
  if (!Utils::String::toNumber<unsigned int>(odValue, mclkOdLines.front())) {
    SPDLOG_WARN("Unknown data format on {}", mclkOd.string());
    SPDLOG_DEBUG(mclkOdLines.front());
    return {};
  }

  auto dpmSclkLines = Utils::File::readFileLines(dpmSclk);
  auto sclkStates = Utils::AMD::parseDPMStates(dpmSclkLines);
  if (!sclkStates) {
    SPDLOG_WARN("Unknown data format on {}", dpmSclk.string());
    for (auto const &line : dpmSclkLines)
      SPDLOG_DEBUG(line);
    return {};
  }

  auto dpmMclkLines = Utils::File::readFileLines(dpmMclk);
  auto mclkStates = Utils::AMD::parseDPMStates(dpmMclkLines);
  if (!mclkStates) {
    SPDLOG_WARN("Unknown data format on {}", dpmMclk.string());
    for (auto const &line : dpmMclkLines)
      SPDLOG_DEBUG(line);
    return {};
  }

  std::vector<std::unique_ptr<IControl>> controls;
  controls.emplace_back(std::make_unique<AMD::PMFreqOd>(
      std::make_unique<SysFSDataSource<unsigned int>>(
          sclkOd,
          [](std::string const &data, unsigned int &output) {
            Utils::String::toNumber<unsigned int>(output, data);
          }),
      std::make_unique<SysFSDataSource<unsigned int>>(
          mclkOd,
          [](std::string const &data, unsigned int &output) {
            Utils::String::toNumber<unsigned int>(output, data);
          }),
      sclkStates.value(), mclkStates.value()));

  return controls;
}

bool const AMD::PMFreqOdProvider::registered_ =
    AMD::PMOverclockProvider::registerProvider(
        std::make_unique<AMD::PMFreqOdProvider>());
