// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#include "odfancurveprovider.h"

#include "../../fanmodeprovider.h"
#include "common/fileutils.h"
#include "core/components/amdutils.h"
#include "core/info/amd/gpuinfoodfanctrl.h"
#include "core/info/igpuinfo.h"
#include "core/sysfsdatasource.h"
#include <filesystem>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

std::vector<std::unique_ptr<IControl>>
AMD::OdFanCurveProvider::provideGPUControls(IGPUInfo const &gpuInfo,
                                            ISWInfo const &) const
{
  if (!(gpuInfo.vendor() == Vendor::AMD &&
        gpuInfo.hasCapability(GPUInfoOdFanCtrl::ID)))
    return {};

  auto curveDataSource = createCurveDataSource(gpuInfo);
  if (!curveDataSource)
    return {};

  auto stopDataSource = createStopDataSource(gpuInfo);

  std::vector<std::unique_ptr<IControl>> controls;
  controls.emplace_back(std::make_unique<AMD::OdFanCurve>(
      std::move(*curveDataSource), std::move(stopDataSource)));

  return controls;
}

std::optional<AMD::OdFanCurve::CurveDataSource>
AMD::OdFanCurveProvider::createCurveDataSource(IGPUInfo const &gpuInfo) const
{
  auto path = gpuInfo.path().sys / "gpu_od" / "fan_ctrl" / "fan_curve";
  if (!Utils::File::isSysFSEntryValid(path))
    return {};

  auto data = Utils::File::readFileLines(path);
  auto tempRange = Utils::AMD::parseOverdriveFanCurveTempRange(data);
  auto speedRange = Utils::AMD::parseOverdriveFanCurveSpeedRange(data);
  if (!(Utils::AMD::parseOverdriveFanCurve(data) && tempRange && speedRange)) {
    SPDLOG_WARN("Unknown data format on {}", path.string());
    SPDLOG_DEBUG(data.front());
    return {};
  }

  return AMD::OdFanCurve::CurveDataSource{
      std::make_unique<SysFSDataSource<std::vector<std::string>>>(std::move(path)),
      std::move(*tempRange), std::move(*speedRange)};
}

std::optional<AMD::OdFanCurve::StopDataSource>
AMD::OdFanCurveProvider::createStopDataSource(IGPUInfo const &gpuInfo) const
{
  auto stopPath = gpuInfo.path().sys / "gpu_od" / "fan_ctrl" /
                  "fan_zero_rpm_enable";
  if (!Utils::File::isSysFSEntryValid(stopPath))
    return {};

  auto data = Utils::File::readFileLines(stopPath);
  if (!Utils::AMD::parseOverdriveFanStop(data)) {
    SPDLOG_WARN("Unknown data format on {}", stopPath.string());
    SPDLOG_DEBUG(data.front());
    return {};
  }

  auto tempPath = gpuInfo.path().sys / "gpu_od" / "fan_ctrl" /
                  "fan_zero_rpm_stop_temperature";
  if (!Utils::File::isSysFSEntryValid(tempPath))
    return {};

  data = Utils::File::readFileLines(tempPath);
  auto tempRange = Utils::AMD::parseOverdriveFanStopTempRange(data);
  if (!(Utils::AMD::parseOverdriveFanStopTemp(data) && tempRange)) {
    SPDLOG_WARN("Unknown data format on {}", tempPath.string());
    SPDLOG_DEBUG(data.front());
    return {};
  }

  return AMD::OdFanCurve::StopDataSource{
      std::make_unique<SysFSDataSource<std::vector<std::string>>>(
          std::move(stopPath)),
      std::make_unique<SysFSDataSource<std::vector<std::string>>>(
          std::move(tempPath)),
      std::move(*tempRange)};
}

bool const AMD::OdFanCurveProvider::registered_ =
    AMD::FanModeProvider::registerProvider(
        std::make_unique<AMD::OdFanCurveProvider>());
