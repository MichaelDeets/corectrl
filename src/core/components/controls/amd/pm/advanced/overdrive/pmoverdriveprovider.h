// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/components/controls/igpucontrolprovider.h"
#include <memory>
#include <vector>

namespace AMD {

class PMOverdriveProvider final : public IGPUControlProvider::IProvider
{
 public:
  std::vector<std::unique_ptr<IControl>>
  provideGPUControls(IGPUInfo const &gpuInfo,
                     ISWInfo const &swInfo) const override;

  static bool
  registerProvider(std::unique_ptr<IGPUControlProvider::IProvider> &&provider);

 private:
  static std::vector<std::unique_ptr<IGPUControlProvider::IProvider>> &
  providers_();

  static bool const registered_;
};

} // namespace AMD
