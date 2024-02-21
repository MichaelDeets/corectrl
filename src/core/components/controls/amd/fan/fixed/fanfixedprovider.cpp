// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "fanfixedprovider.h"

#include "../fanmodeprovider.h"
#include "common/fileutils.h"
#include "common/stringutils.h"
#include "core/info/amd/gpuinfoodfanctrl.h"
#include "core/info/igpuinfo.h"
#include "core/info/iswinfo.h"
#include "core/sysfsdatasource.h"
#include "fanfixed.h"
#include <filesystem>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>

std::vector<std::unique_ptr<IControl>>
AMD::FanFixedProvider::provideGPUControls(IGPUInfo const &gpuInfo,
                                          ISWInfo const &) const
{
  if (!(gpuInfo.vendor() == Vendor::AMD &&
        !gpuInfo.hasCapability(GPUInfoOdFanCtrl::ID)))
    return {};

  auto path = Utils::File::findHWMonXDirectory(gpuInfo.path().sys / "hwmon");
  if (!path)
    return {};

  auto pwmEnable = path.value() / "pwm1_enable";
  auto pwm = path.value() / "pwm1";
  if (!(Utils::File::isSysFSEntryValid(pwm) &&
        Utils::File::isSysFSEntryValid(pwmEnable)))
    return {};

  unsigned int value;

  auto pwmEnableLines = Utils::File::readFileLines(pwmEnable);
  if (!Utils::String::toNumber<unsigned int>(value, pwmEnableLines.front())) {
    SPDLOG_WARN("Unknown data format on {}", pwmEnable.string());
    SPDLOG_DEBUG(pwmEnableLines.front());
    return {};
  }

  auto pwmLines = Utils::File::readFileLines(pwm);
  if (!Utils::String::toNumber<unsigned int>(value, pwmLines.front())) {
    SPDLOG_WARN("Unknown data format on {}", pwm.string());
    SPDLOG_DEBUG(pwmLines.front());
    return {};
  }

  std::vector<std::unique_ptr<IControl>> controls;
  controls.emplace_back(std::make_unique<AMD::FanFixed>(
      std::make_unique<SysFSDataSource<unsigned int>>(
          pwmEnable,
          [](std::string const &data, unsigned int &output) {
            Utils::String::toNumber<unsigned int>(output, data);
          }),
      std::make_unique<SysFSDataSource<unsigned int>>(
          pwm, [](std::string const &data, unsigned int &output) {
            Utils::String::toNumber<unsigned int>(output, data);
          })));

  return controls;
}

bool const AMD::FanFixedProvider::registered_ =
    AMD::FanModeProvider::registerProvider(
        std::make_unique<AMD::FanFixedProvider>());
