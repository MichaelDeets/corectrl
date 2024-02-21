// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#include "odfancurveprovider.h"

#include "../../fanmodeprovider.h"
#include "common/fileutils.h"
#include "core/components/amdutils.h"
#include "core/info/amd/gpuinfoodfanctrl.h"
#include "core/info/igpuinfo.h"
#include "core/sysfsdatasource.h"
#include "odfancurve.h"
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

  auto path = gpuInfo.path().sys / "gpu_od" / "fan_ctrl" / "fan_curve";
  if (!Utils::File::isSysFSEntryValid(path))
    return {};

  auto data = Utils::File::readFileLines(path);
  if (!Utils::AMD::hasOverdriveFanCurveControl(data)) {
    SPDLOG_WARN("Unknown data format on {}", path.string());
    SPDLOG_DEBUG(data.front());
    return {};
  }

  std::vector<std::unique_ptr<IControl>> controls;
  controls.emplace_back(std::make_unique<AMD::OdFanCurve>(
      std::make_unique<SysFSDataSource<std::vector<std::string>>>(
          std::move(path))));

  return controls;
}

bool const AMD::OdFanCurveProvider::registered_ =
    AMD::FanModeProvider::registerProvider(
        std::make_unique<AMD::OdFanCurveProvider>());
