// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "pmpowerstateprovider.h"

#include "common/fileutils.h"
#include "core/info/igpuinfo.h"
#include "core/sysfsdatasource.h"
#include "pmpowerstate.h"
#include "pmpowerstatemodeprovider.h"
#include <filesystem>
#include <memory>
#include <string>

std::vector<std::unique_ptr<IControl>>
AMD::PMPowerStateProvider::provideGPUControls(IGPUInfo const &gpuInfo,
                                              ISWInfo const &) const
{
  if (gpuInfo.vendor() != Vendor::AMD)
    return {};

  auto driver = gpuInfo.info(IGPUInfo::Keys::driver);
  if (driver != "radeon")
    return {};

  auto powerDpmStatePath = gpuInfo.path().sys / "power_dpm_state";
  if (!Utils::File::isSysFSEntryValid(powerDpmStatePath))
    return {};

  std::vector<std::unique_ptr<IControl>> controls;
  controls.emplace_back(std::make_unique<AMD::PMPowerState>(
      std::make_unique<SysFSDataSource<std::string>>(powerDpmStatePath)));

  return controls;
}

bool const AMD::PMPowerStateProvider::registered_ =
    AMD::PMPowerStateModeProvider::registerProvider(
        std::make_unique<AMD::PMPowerStateProvider>());
