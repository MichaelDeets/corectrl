// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021 Juan Palacios <jpalaciosdev@gmail.com>

#include "pmfreqvoltprovider.h"

#include "../pmoverdriveprovider.h"
#include "common/fileutils.h"
#include "core/components/amdutils.h"
#include "core/components/controls/amd/pm/handlers/ppdpmhandler.h"
#include "core/info/amd/gpuinfopmoverdrive.h"
#include "core/info/igpuinfo.h"
#include "core/sysfsdatasource.h"
#include "pmfreqvolt.h"
#include <algorithm>
#include <cctype>
#include <spdlog/spdlog.h>

std::optional<std::filesystem::path>
AMD::PMFreqVoltProvider::getControlDPMPath(IGPUInfo const &gpuInfo,
                                           std::string control) const
{
  std::transform(control.cbegin(), control.cend(), control.begin(), ::tolower);

  auto dpmPath = gpuInfo.path().sys / ("pp_dpm_" + control);
  if (!Utils::File::isSysFSEntryValid(dpmPath))
    return {};

  auto dpmLines = Utils::File::readFileLines(dpmPath);
  if (!Utils::AMD::parseDPMStates(dpmLines)) {
    SPDLOG_WARN("Unknown data format on {}", dpmPath.string());
    for (auto const &line : dpmLines)
      SPDLOG_DEBUG(line);
    return {};
  }

  return dpmPath;
}

bool AMD::PMFreqVoltProvider::hasValidOverdriveControl(
    std::string const &controlName, std::filesystem::path const &ppOdClkVoltPath,
    std::vector<std::string> const &ppOdClkVoltLines) const
{
  auto valid =
      !Utils::AMD::ppOdClkVoltageHasKnownFreqVoltQuirks(controlName,
                                                        ppOdClkVoltLines) &&
      Utils::AMD::parseOverdriveClkRange(controlName, ppOdClkVoltLines) &&
      Utils::AMD::parseOverdriveVoltRange(ppOdClkVoltLines) &&
      Utils::AMD::parseOverdriveClksVolts(controlName, ppOdClkVoltLines);

  if (!valid) {
    SPDLOG_WARN("Invalid data on {} for control {}", ppOdClkVoltPath.string(),
                controlName);
    for (auto const &line : ppOdClkVoltLines)
      SPDLOG_DEBUG(line);
  }

  return valid;
}

std::optional<std::unique_ptr<IControl>> AMD::PMFreqVoltProvider::createControl(
    IGPUInfo const &gpuInfo, std::string controlName,
    std::filesystem::path const &perfLevelPath,
    std::filesystem::path const &ppOdClkVoltPath,
    std::vector<std::string> const &ppOdClkVoltLines) const
{
  auto controlDPMPath = getControlDPMPath(gpuInfo, controlName);
  if (!controlDPMPath)
    return {};

  if (!hasValidOverdriveControl(controlName, ppOdClkVoltPath, ppOdClkVoltLines))
    return {};

  auto controlCmdId = Utils::AMD::getOverdriveClkControlCmdId(controlName);
  if (!controlCmdId) {
    SPDLOG_WARN("Unsupported control {}", controlName);
    return {};
  }

  return std::make_unique<AMD::PMFreqVolt>(
      std::move(controlName), std::move(*controlCmdId),
      std::make_unique<SysFSDataSource<std::vector<std::string>>>(ppOdClkVoltPath),
      std::make_unique<PpDpmHandler>(
          std::make_unique<SysFSDataSource<std::string>>(perfLevelPath),
          std::make_unique<SysFSDataSource<std::vector<std::string>>>(
              *controlDPMPath)));
}

std::vector<std::unique_ptr<IControl>>
AMD::PMFreqVoltProvider::provideGPUControls(IGPUInfo const &gpuInfo,
                                            ISWInfo const &) const
{
  if (!(gpuInfo.vendor() == Vendor::AMD &&
        gpuInfo.hasCapability(GPUInfoPMOverdrive::ClkVolt)))
    return {};

  auto ppOdClkVoltPath = gpuInfo.path().sys / "pp_od_clk_voltage";
  auto ppOdClkVoltLines = Utils::File::readFileLines(ppOdClkVoltPath);

  auto const controlNames =
      Utils::AMD::parseOverdriveClkControls(ppOdClkVoltLines);
  if (!controlNames)
    return {};

  std::vector<std::unique_ptr<IControl>> controls;
  auto perfLevelPath = gpuInfo.path().sys / "power_dpm_force_performance_level";

  for (auto controlName : controlNames.value()) {
    auto control = createControl(gpuInfo, controlName, perfLevelPath,
                                 ppOdClkVoltPath, ppOdClkVoltLines);
    if (control)
      controls.emplace_back(std::move(*control));
  }

  return controls;
}

bool const AMD::PMFreqVoltProvider::registered_ =
    AMD::PMOverdriveProvider::registerProvider(
        std::make_unique<AMD::PMFreqVoltProvider>());
