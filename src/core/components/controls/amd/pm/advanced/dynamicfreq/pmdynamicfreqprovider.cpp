// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "pmdynamicfreqprovider.h"

#include "../freqmode/pmfreqmodeprovider.h"
#include "common/fileutils.h"
#include "core/info/igpuinfo.h"
#include "core/sysfsdatasource.h"
#include "pmdynamicfreq.h"
#include <filesystem>
#include <memory>
#include <string>

std::vector<std::unique_ptr<IControl>>
AMD::PMDynamicFreqProvider::provideGPUControls(IGPUInfo const &gpuInfo,
                                               ISWInfo const &) const
{
  if (gpuInfo.vendor() != Vendor::AMD)
    return {};

  auto driver = gpuInfo.info(IGPUInfo::Keys::driver);
  if (driver != "amdgpu")
    return {};

  auto perfLevel = gpuInfo.path().sys / "power_dpm_force_performance_level";
  if (!Utils::File::isSysFSEntryValid(perfLevel))
    return {};

  std::vector<std::unique_ptr<IControl>> controls;
  controls.emplace_back(std::make_unique<AMD::PMDynamicFreq>(
      std::make_unique<SysFSDataSource<std::string>>(perfLevel)));

  return controls;
}

bool AMD::PMDynamicFreqProvider::register_()
{
  AMD::PMFreqModeProvider::registerProvider(
      std::make_unique<AMD::PMDynamicFreqProvider>());

  return true;
}

bool const AMD::PMDynamicFreqProvider::registered_ =
    AMD::PMDynamicFreqProvider::register_();
