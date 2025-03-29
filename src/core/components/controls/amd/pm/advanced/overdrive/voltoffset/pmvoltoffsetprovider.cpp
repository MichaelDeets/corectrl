// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021 Juan Palacios <jpalaciosdev@gmail.com>

#include "pmvoltoffsetprovider.h"

#include "../pmoverdriveprovider.h"
#include "common/fileutils.h"
#include "core/components/amdutils.h"
#include "core/info/amd/gpuinfopmoverdrive.h"
#include "core/info/igpuinfo.h"
#include "core/sysfsdatasource.h"
#include "pmvoltoffset.h"
#include <filesystem>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

std::vector<std::unique_ptr<IControl>>
AMD::PMVoltOffsetProvider::provideGPUControls(IGPUInfo const &gpuInfo,
                                              ISWInfo const &) const
{
  if (!(gpuInfo.vendor() == Vendor::AMD &&
        gpuInfo.hasCapability(GPUInfoPMOverdrive::VoltOffset)))
    return {};

  auto ppOdClkVolt = gpuInfo.path().sys / "pp_od_clk_voltage";
  auto ppOdClkVoltLines = Utils::File::readFileLines(ppOdClkVolt);
  if (!Utils::AMD::parseOverdriveVoltOffset(ppOdClkVoltLines)) {
    SPDLOG_WARN("Invalid data on {}", ppOdClkVolt.string());
    for (auto const &line : ppOdClkVoltLines)
      SPDLOG_DEBUG(line);
    return {};
  }

  auto range = Utils::AMD::parseOverdriveVoltOffsetRange(ppOdClkVoltLines);
  if (!range)
    range = std::make_pair(units::voltage::millivolt_t(-250),
                           units::voltage::millivolt_t(250));

  std::vector<std::unique_ptr<IControl>> controls;
  controls.emplace_back(std::make_unique<AMD::PMVoltOffset>(
      std::move(*range),
      std::make_unique<SysFSDataSource<std::vector<std::string>>>(ppOdClkVolt)));

  return controls;
}

bool const AMD::PMVoltOffsetProvider::registered_ =
    AMD::PMOverdriveProvider::registerProvider(
        std::make_unique<AMD::PMVoltOffsetProvider>());
