// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/components/controls/igpucontrolprovider.h"
#include "core/idatasource.h"
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace AMD {

class OdFanAutoProvider final : public IGPUControlProvider::IProvider
{
 public:
  std::vector<std::unique_ptr<IControl>>
  provideGPUControls(IGPUInfo const &gpuInfo,
                     ISWInfo const &swInfo) const override;

 private:
  std::optional<std::unique_ptr<IDataSource<std::vector<std::string>>>>
  createOdFanTargetTempDataSource(IGPUInfo const &gpuInfo) const;
  std::optional<std::unique_ptr<IDataSource<std::vector<std::string>>>>
  createOdFanMinPWMDataSource(IGPUInfo const &gpuInfo) const;
  std::optional<std::unique_ptr<IDataSource<std::vector<std::string>>>>
  createOdFanAcousticTargetDataSource(IGPUInfo const &gpuInfo) const;
  std::optional<std::unique_ptr<IDataSource<std::vector<std::string>>>>
  createOdFanAcousticLimitDataSource(IGPUInfo const &gpuInfo) const;
  std::unique_ptr<IDataSource<std::vector<std::string>>>
  createDataSource(std::filesystem::path &&path) const;

  static bool const registered_;
};

} // namespace AMD
