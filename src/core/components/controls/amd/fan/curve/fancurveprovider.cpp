// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "fancurveprovider.h"

#include "../fanmodeprovider.h"
#include "common/fileutils.h"
#include "common/stringutils.h"
#include "core/info/amd/gpuinfoodfanctrl.h"
#include "core/info/igpuinfo.h"
#include "core/sysfsdatasource.h"
#include "fancurve.h"
#include <filesystem>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>

std::vector<std::unique_ptr<IControl>>
AMD::FanCurveProvider::provideGPUControls(IGPUInfo const &gpuInfo,
                                          ISWInfo const &) const
{
  std::vector<std::unique_ptr<IControl>> controls;

  if (!(gpuInfo.vendor() == Vendor::AMD &&
        !gpuInfo.hasCapability(GPUInfoOdFanCtrl::ID)))
    return {};

  auto path = Utils::File::findHWMonXDirectory(gpuInfo.path().sys / "hwmon");
  if (!path)
    return {};

  auto pwmEnable = path.value() / "pwm1_enable";
  auto pwm = path.value() / "pwm1";
  auto tempInput = path.value() / "temp1_input";
  auto tempCrit = path.value() / "temp1_crit";
  if (!(Utils::File::isSysFSEntryValid(pwm) &&
        Utils::File::isSysFSEntryValid(pwmEnable) &&
        Utils::File::isSysFSEntryValid(tempInput) &&
        Utils::File::isSysFSEntryValid(tempCrit)))
    return {};

  int tempCritValue{0};
  Utils::String::toNumber(tempCritValue,
                          Utils::File::readFileLines(tempCrit).front());
  tempCritValue = (tempCritValue > 0 &&
                   tempCritValue < 150000) // check bogus values, see #103
                      ? tempCritValue / 1000
                      : 90;

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

  int tempInputValue;
  auto tempInputLines = Utils::File::readFileLines(tempInput);
  if (!Utils::String::toNumber<int>(tempInputValue, tempInputLines.front())) {
    SPDLOG_WARN("Unknown data format on {}", tempInput.string());
    SPDLOG_DEBUG(tempInputLines.front());
    return {};
  }

  controls.emplace_back(std::make_unique<AMD::FanCurve>(
      std::make_unique<SysFSDataSource<unsigned int>>(
          pwmEnable,
          [](std::string const &data, unsigned int &output) {
            Utils::String::toNumber<unsigned int>(output, data);
          }),
      std::make_unique<SysFSDataSource<unsigned int>>(
          pwm,
          [](std::string const &data, unsigned int &output) {
            Utils::String::toNumber<unsigned int>(output, data);
          }),
      std::make_unique<SysFSDataSource<int>>(
          tempInput,
          [](std::string const &data, int &output) {
            int value;
            Utils::String::toNumber<int>(value, data);
            output = value / 1000;
          }),
      units::temperature::celsius_t(0),
      units::temperature::celsius_t(tempCritValue)));

  return controls;
}

bool const AMD::FanCurveProvider::registered_ =
    AMD::FanModeProvider::registerProvider(
        std::make_unique<AMD::FanCurveProvider>());
