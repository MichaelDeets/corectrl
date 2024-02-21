// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021 Juan Palacios <jpalaciosdev@gmail.com>

#include "pmfreqrangeprovider.h"

#include "../pmoverdriveprovider.h"
#include "common/fileutils.h"
#include "core/components/amdutils.h"
#include "core/info/amd/gpuinfopmoverdrive.h"
#include "core/info/igpuinfo.h"
#include "core/sysfsdatasource.h"
#include "pmfreqrange.h"
#include <algorithm>
#include <spdlog/spdlog.h>

std::optional<std::unique_ptr<IControl>> AMD::PMFreqRangeProvider::createControl(
    std::string controlName, std::filesystem::path const &ppOdClkVoltPath,
    std::vector<std::string> const &ppOdClkVoltLines) const
{
  auto outOfRangeStates = Utils::AMD::ppOdClkVoltageFreqRangeOutOfRangeStates(
      controlName, ppOdClkVoltLines);

  if (outOfRangeStates.has_value()) {
    for (auto stateIndex : outOfRangeStates.value()) {
      SPDLOG_WARN("Detected out of range state index {} on control {}",
                  stateIndex, controlName);
    }
  }

  auto valid =
      !(outOfRangeStates.has_value() && outOfRangeStates->size() > 1) &&
      Utils::AMD::parseOverdriveClkRange(controlName, ppOdClkVoltLines).has_value() &&
      Utils::AMD::parseOverdriveClks(controlName, ppOdClkVoltLines).has_value();

  if (!valid) {
    SPDLOG_WARN("Invalid data on {} for control {}", ppOdClkVoltPath.string(),
                controlName);
    for (auto const &line : ppOdClkVoltLines)
      SPDLOG_DEBUG(line);
    return {};
  }

  auto controlCmdId = Utils::AMD::getOverdriveClkControlCmdId(controlName);
  if (!controlCmdId) {
    SPDLOG_WARN("Unsupported control {}", controlName);
    return {};
  }

  auto disabledBound =
      outOfRangeStates.has_value()
          ? std::optional<AMD::PMFreqRange::DisabledBound>(
                AMD::PMFreqRange::DisabledBound{outOfRangeStates->at(0)})
          : std::nullopt;

  return std::make_unique<AMD::PMFreqRange>(
      std::move(controlName), std::move(*controlCmdId),
      std::make_unique<SysFSDataSource<std::vector<std::string>>>(ppOdClkVoltPath),
      std::move(disabledBound));
}

std::vector<std::unique_ptr<IControl>>
AMD::PMFreqRangeProvider::provideGPUControls(IGPUInfo const &gpuInfo,
                                             ISWInfo const &) const
{
  if (!(gpuInfo.vendor() == Vendor::AMD &&
        gpuInfo.hasCapability(GPUInfoPMOverdrive::Clk)))
    return {};

  auto ppOdClkVoltPath = gpuInfo.path().sys / "pp_od_clk_voltage";
  auto ppOdClkVoltLines = Utils::File::readFileLines(ppOdClkVoltPath);

  auto const controlNames =
      Utils::AMD::parseOverdriveClkControls(ppOdClkVoltLines);
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

bool const AMD::PMFreqRangeProvider::registered_ =
    AMD::PMOverdriveProvider::registerProvider(
        std::make_unique<AMD::PMFreqRangeProvider>());
