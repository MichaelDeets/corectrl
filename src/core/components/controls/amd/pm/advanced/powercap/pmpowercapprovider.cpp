// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "pmpowercapprovider.h"

#include "../pmadvancedprovider.h"
#include "common/fileutils.h"
#include "common/stringutils.h"
#include "core/info/igpuinfo.h"
#include "core/sysfsdatasource.h"
#include "pmpowercap.h"
#include <easylogging++.h>
#include <fmt/format.h>
#include <memory>
#include <string>
#include <vector>

std::optional<units::power::microwatt_t>
AMD::PMPowerCapProvider::readPowerFrom(std::filesystem::path const &path) const
{
  if (!Utils::File::isSysFSEntryValid(path))
    return {};

  unsigned long value;
  auto lines = Utils::File::readFileLines(path);
  if (!Utils::String::toNumber<unsigned long>(value, lines.front())) {
    LOG(WARNING) << fmt::format("Unknown data format on {}", path.string());
    LOG(ERROR) << lines.front();
    return {};
  }

  return units::power::microwatt_t(value);
}

std::vector<std::unique_ptr<IControl>>
AMD::PMPowerCapProvider::provideGPUControls(IGPUInfo const &gpuInfo,
                                            ISWInfo const &) const
{
  std::vector<std::unique_ptr<IControl>> controls;

  if (gpuInfo.vendor() != Vendor::AMD)
    return controls;

  auto path = Utils::File::findHWMonXDirectory(gpuInfo.path().sys / "hwmon");
  if (!path)
    return controls;

  auto power1CapPath = path.value() / "power1_cap";
  auto value = readPowerFrom(power1CapPath);
  auto min = readPowerFrom(path.value() / "power1_cap_min");
  auto max = readPowerFrom(path.value() / "power1_cap_max");
  if (!(value && min && max))
    return controls;

  // Drivers might report bogus values for either (or both) upper
  // and lower range bounds. See #337.
  if (*max <= *min) {
    LOG(ERROR) << fmt::format("Bogus power cap range bounds detected: "
                              "power1_cap_min ({}), power1_cap_max ({}).",
                              (*min).to<unsigned long>(),
                              (*max).to<unsigned long>());
    return controls;
  }

  auto defaultValue = readPowerFrom(path.value() / "power1_cap_default");

  controls.emplace_back(std::make_unique<AMD::PMPowerCap>(
      std::make_unique<SysFSDataSource<unsigned long>>(
          power1CapPath,
          [](std::string const &data, unsigned long &output) {
            Utils::String::toNumber<unsigned long>(output, data);
          }),
      *min, *max, defaultValue));

  return controls;
}

bool const AMD::PMPowerCapProvider::registered_ =
    AMD::PMAdvancedProvider::registerProvider(
        std::make_unique<AMD::PMPowerCapProvider>());
