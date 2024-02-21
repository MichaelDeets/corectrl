// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#include "odfanautoprovider.h"

#include "../../fanmodeprovider.h"
#include "common/fileutils.h"
#include "core/components/amdutils.h"
#include "core/info/amd/gpuinfoodfanctrl.h"
#include "core/info/igpuinfo.h"
#include "core/sysfsdatasource.h"
#include "odfanauto.h"
#include <spdlog/spdlog.h>
#include <utility>

std::vector<std::unique_ptr<IControl>>
AMD::OdFanAutoProvider::provideGPUControls(IGPUInfo const &gpuInfo,
                                           ISWInfo const &) const
{
  std::vector<std::unique_ptr<IControl>> controls;

  if (!(gpuInfo.vendor() == Vendor::AMD &&
        gpuInfo.hasCapability(GPUInfoOdFanCtrl::ID)))
    return {};

  // Attempt to create a data source from one of the overdrive fan controls that
  // triggers the auto fan mode.
  auto dataSource = createOdFanTargetTempDataSource(gpuInfo);
  if (!dataSource)
    dataSource = createOdFanMinPWMDataSource(gpuInfo);
  if (!dataSource)
    dataSource = createOdFanAcousticTargetDataSource(gpuInfo);
  if (!dataSource)
    dataSource = createOdFanAcousticLimitDataSource(gpuInfo);

  if (dataSource)
    controls.emplace_back(
        std::make_unique<AMD::OdFanAuto>(std::move(*dataSource)));

  return controls;
}

std::optional<std::unique_ptr<IDataSource<std::vector<std::string>>>>
AMD::OdFanAutoProvider::createOdFanTargetTempDataSource(IGPUInfo const &gpuInfo) const
{
  auto path = gpuInfo.path().sys / "gpu_od" / "fan_ctrl" /
              "fan_target_temperature";
  if (!Utils::File::isSysFSEntryValid(path))
    return {};

  auto data = Utils::File::readFileLines(path);
  if (!Utils::AMD::hasOverdriveFanTargetTempControl(data)) {
    SPDLOG_WARN("Unknown data format on {}", path.string());
    SPDLOG_DEBUG(data.front());
    return {};
  }

  return createDataSource(std::move(path));
}

std::optional<std::unique_ptr<IDataSource<std::vector<std::string>>>>
AMD::OdFanAutoProvider::createOdFanMinPWMDataSource(IGPUInfo const &gpuInfo) const
{
  auto path = gpuInfo.path().sys / "gpu_od" / "fan_ctrl" / "fan_minimum_pwm";
  if (!Utils::File::isSysFSEntryValid(path))
    return {};

  auto data = Utils::File::readFileLines(path);
  if (!Utils::AMD::hasOverdriveFanMinimumPWMControl(data)) {
    SPDLOG_WARN("Unknown data format on {}", path.string());
    SPDLOG_DEBUG(data.front());
    return {};
  }

  return createDataSource(std::move(path));
}

std::optional<std::unique_ptr<IDataSource<std::vector<std::string>>>>
AMD::OdFanAutoProvider::createOdFanAcousticTargetDataSource(
    IGPUInfo const &gpuInfo) const
{
  auto path = gpuInfo.path().sys / "gpu_od" / "fan_ctrl" /
              "acoustic_target_rpm_threshold";
  if (!Utils::File::isSysFSEntryValid(path))
    return {};

  auto data = Utils::File::readFileLines(path);
  if (!Utils::AMD::hasOverdriveFanAcousticTargetControl(data)) {
    SPDLOG_WARN("Unknown data format on {}", path.string());
    SPDLOG_DEBUG(data.front());
    return {};
  }

  return createDataSource(std::move(path));
}

std::optional<std::unique_ptr<IDataSource<std::vector<std::string>>>>
AMD::OdFanAutoProvider::createOdFanAcousticLimitDataSource(
    IGPUInfo const &gpuInfo) const
{
  auto path = gpuInfo.path().sys / "gpu_od" / "fan_ctrl" /
              "acoustic_limit_rpm_threshold";
  if (!Utils::File::isSysFSEntryValid(path))
    return {};

  auto data = Utils::File::readFileLines(path);
  if (!Utils::AMD::hasOverdriveFanAcousticLimitControl(data)) {
    SPDLOG_WARN("Unknown data format on {}", path.string());
    SPDLOG_DEBUG(data.front());
    return {};
  }

  return createDataSource(std::move(path));
}

std::unique_ptr<IDataSource<std::vector<std::string>>>
AMD::OdFanAutoProvider::createDataSource(std::filesystem::path &&path) const
{
  return std::make_unique<SysFSDataSource<std::vector<std::string>>>(
      std::move(path));
}

bool const AMD::OdFanAutoProvider::registered_ =
    AMD::FanModeProvider::registerProvider(
        std::make_unique<AMD::OdFanAutoProvider>());
