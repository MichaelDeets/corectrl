// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "fanautoprovider.h"

#include "../fanmodeprovider.h"
#include "common/fileutils.h"
#include "common/stringutils.h"
#include "core/info/amd/gpuinfoodfanctrl.h"
#include "core/info/igpuinfo.h"
#include "core/sysfsdatasource.h"
#include "fanauto.h"
#include <filesystem>
#include <memory>

std::vector<std::unique_ptr<IControl>>
AMD::FanAutoProvider::provideGPUControls(IGPUInfo const &gpuInfo,
                                         ISWInfo const &) const
{
  if (!(gpuInfo.vendor() == Vendor::AMD &&
        !gpuInfo.hasCapability(GPUInfoOdFanCtrl::ID)))
    return {};

  auto path = Utils::File::findHWMonXDirectory(gpuInfo.path().sys / "hwmon");
  if (!path)
    return {};

  auto pwmEnable = path.value() / "pwm1_enable";
  if (!Utils::File::isSysFSEntryValid(pwmEnable))
    return {};

  std::vector<std::unique_ptr<IControl>> controls;
  controls.emplace_back(std::make_unique<AMD::FanAuto>(
      std::make_unique<SysFSDataSource<unsigned int>>(
          pwmEnable, [](std::string const &data, unsigned int &output) {
            Utils::String::toNumber<unsigned int>(output, data);
          })));

  return controls;
}

bool const AMD::FanAutoProvider::registered_ =
    AMD::FanModeProvider::registerProvider(
        std::make_unique<AMD::FanAutoProvider>());
