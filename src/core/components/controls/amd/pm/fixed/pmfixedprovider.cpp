// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "pmfixedprovider.h"

#include "../pmperfmodeprovider.h"
#include "common/fileutils.h"
#include "core/info/amd/gpuinfopm.h"
#include "core/sysfsdatasource.h"
#include "pmfixedlegacy.h"
#include "pmfixedr600.h"
#include <filesystem>
#include <memory>
#include <string>

std::vector<std::unique_ptr<IControl>>
AMD::PMFixedProvider::provideGPUControls(IGPUInfo const &gpuInfo,
                                         ISWInfo const &) const
{
  std::vector<std::unique_ptr<IControl>> controls;

  if (gpuInfo.vendor() != Vendor::AMD)
    return {};

  if (gpuInfo.hasCapability(GPUInfoPM::Legacy)) {

    auto powerMethod = gpuInfo.path().sys / "power_method";
    auto powerProfile = gpuInfo.path().sys / "power_profile";
    if (!(Utils::File::isSysFSEntryValid(powerMethod) &&
          Utils::File::isSysFSEntryValid(powerProfile)))
      return {};

    controls.emplace_back(std::make_unique<AMD::PMFixedLegacy>(
        std::make_unique<SysFSDataSource<std::string>>(powerMethod),
        std::make_unique<SysFSDataSource<std::string>>(powerProfile)));
  }
  else if (gpuInfo.hasCapability(GPUInfoPM::Radeon) ||
           gpuInfo.hasCapability(GPUInfoPM::Amdgpu)) {

    auto perfLevel = gpuInfo.path().sys / "power_dpm_force_performance_level";
    if (!Utils::File::isSysFSEntryValid(perfLevel))
      return {};

    controls.emplace_back(std::make_unique<AMD::PMFixedR600>(
        std::make_unique<SysFSDataSource<std::string>>(perfLevel)));
  }

  return controls;
}

bool const AMD::PMFixedProvider::registered_ =
    AMD::PMPerfModeProvider::registerProvider(
        std::make_unique<AMD::PMFixedProvider>());
