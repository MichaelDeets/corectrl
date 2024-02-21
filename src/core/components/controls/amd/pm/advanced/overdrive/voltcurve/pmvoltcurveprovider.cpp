// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021 Juan Palacios <jpalaciosdev@gmail.com>

#include "pmvoltcurveprovider.h"

#include "../pmoverdriveprovider.h"
#include "common/fileutils.h"
#include "core/components/amdutils.h"
#include "core/info/amd/gpuinfopmoverdrive.h"
#include "core/info/igpuinfo.h"
#include "core/sysfsdatasource.h"
#include "pmvoltcurve.h"
#include <filesystem>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

std::vector<std::unique_ptr<IControl>>
AMD::PMVoltCurveProvider::provideGPUControls(IGPUInfo const &gpuInfo,
                                             ISWInfo const &) const
{
  if (!(gpuInfo.vendor() == Vendor::AMD &&
        gpuInfo.hasCapability(GPUInfoPMOverdrive::VoltCurve)))
    return {};

  auto ppOdClkVolt = gpuInfo.path().sys / "pp_od_clk_voltage";
  auto ppOdClkVoltLines = Utils::File::readFileLines(ppOdClkVolt);

  auto valid =
      !Utils::AMD::ppOdClkVoltageHasKnownVoltCurveQuirks(ppOdClkVoltLines) &&
      Utils::AMD::parseOverdriveVoltCurveRange(ppOdClkVoltLines).has_value() &&
      Utils::AMD::parseOverdriveVoltCurve(ppOdClkVoltLines).has_value();

  if (!valid) {
    SPDLOG_WARN("Invalid data on {}", ppOdClkVolt.string());
    for (auto const &line : ppOdClkVoltLines)
      SPDLOG_DEBUG(line);
    return {};
  }

  std::vector<std::unique_ptr<IControl>> controls;
  controls.emplace_back(std::make_unique<AMD::PMVoltCurve>(
      "vc",
      std::make_unique<SysFSDataSource<std::vector<std::string>>>(ppOdClkVolt)));

  return controls;
}

bool const AMD::PMVoltCurveProvider::registered_ =
    AMD::PMOverdriveProvider::registerProvider(
        std::make_unique<AMD::PMVoltCurveProvider>());
