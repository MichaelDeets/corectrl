// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Juan Palacios <jpalaciosdev@gmail.com>

#include "pmfreqoffsetprovider.h"

#include "../pmoverdriveprovider.h"
#include "common/fileutils.h"
#include "core/components/amdutils.h"
#include "core/info/amd/gpuinfopmoverdrive.h"
#include "core/info/igpuinfo.h"
#include "core/sysfsdatasource.h"
#include "pmfreqoffset.h"
#include <spdlog/spdlog.h>

std::vector<std::unique_ptr<IControl>>
AMD::PMFreqOffsetProvider::provideGPUControls(IGPUInfo const &gpuInfo,
                                              ISWInfo const &) const
{
  if (!(gpuInfo.vendor() == Vendor::AMD &&
        gpuInfo.hasCapability(GPUInfoPMOverdrive::ClkOffset)))
    return {};

  auto ppOdClkVoltPath = gpuInfo.path().sys / "pp_od_clk_voltage";
  auto ppOdClkVoltLines = Utils::File::readFileLines(ppOdClkVoltPath);

  auto const controlNames =
      Utils::AMD::parseOverdriveClkOffsetControls(ppOdClkVoltLines);
  if (!controlNames)
    return {};

  std::vector<std::unique_ptr<IControl>> controls;

  for (auto controlName : controlNames.value()) {
    auto control = createControl(controlName, ppOdClkVoltPath, ppOdClkVoltLines);
    if (control)
      controls.emplace_back(std::move(*control));
  }

  return controls;
}

std::optional<std::unique_ptr<IControl>> AMD::PMFreqOffsetProvider::createControl(
    std::string controlName, std::filesystem::path const &ppOdClkVoltPath,
    std::vector<std::string> const &ppOdClkVoltLines) const
{
  auto range = Utils::AMD::parseOverdriveClkOffsetRange(controlName,
                                                        ppOdClkVoltLines);
  auto valid = range.has_value() && range->first < range->second &&
               Utils::AMD::parseOverdriveClkOffset(controlName, ppOdClkVoltLines)
                   .has_value();

  if (!valid) {
    SPDLOG_WARN("Invalid data on {} for control {}", ppOdClkVoltPath.string(),
                controlName);
    for (auto const &line : ppOdClkVoltLines)
      SPDLOG_DEBUG(line);
    return {};
  }

  auto controlCmdId = Utils::AMD::getOverdriveClkOffsetControlCmdId(controlName);
  if (!controlCmdId) {
    SPDLOG_WARN("Unsupported control {}", controlName);
    return {};
  }

  return std::make_unique<AMD::PMFreqOffset>(
      std::move(controlName), std::move(*controlCmdId), std::move(*range),
      std::make_unique<SysFSDataSource<std::vector<std::string>>>(
          ppOdClkVoltPath));
}

bool const AMD::PMFreqOffsetProvider::registered_ =
    AMD::PMOverdriveProvider::registerProvider(
        std::make_unique<AMD::PMFreqOffsetProvider>());
