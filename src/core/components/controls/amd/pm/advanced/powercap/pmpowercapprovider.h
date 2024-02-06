// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/components/controls/igpucontrolprovider.h"
#include <filesystem>
#include <optional>
#include <units.h>

namespace AMD {

class PMPowerCapProvider final : public IGPUControlProvider::IProvider
{
 public:
  std::vector<std::unique_ptr<IControl>>
  provideGPUControls(IGPUInfo const &gpuInfo,
                     ISWInfo const &swInfo) const override;

 private:
  std::optional<units::power::microwatt_t>
  readPowerFrom(std::filesystem::path const &path) const;

  static bool const registered_;
};

} // namespace AMD
