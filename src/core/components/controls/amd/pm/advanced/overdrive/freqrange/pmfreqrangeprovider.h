// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/components/controls/igpucontrolprovider.h"
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace AMD {

class PMFreqRangeProvider final : public IGPUControlProvider::IProvider
{
 public:
  std::vector<std::unique_ptr<IControl>>
  provideGPUControls(IGPUInfo const &gpuInfo,
                     ISWInfo const &swInfo) const override;

 private:
  std::optional<std::unique_ptr<IControl>>
  createControl(std::string controlName,
                std::filesystem::path const &ppOdClkVoltPath,
                std::vector<std::string> const &ppOdClkVoltLines) const;

  static bool const registered_;
};

} // namespace AMD
