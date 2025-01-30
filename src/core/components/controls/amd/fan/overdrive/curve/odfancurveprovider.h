// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/components/controls/igpucontrolprovider.h"
#include "odfancurve.h"
#include <optional>

namespace AMD {

class OdFanCurveProvider final : public IGPUControlProvider::IProvider
{
 public:
  std::vector<std::unique_ptr<IControl>>
  provideGPUControls(IGPUInfo const &gpuInfo,
                     ISWInfo const &swInfo) const override;

 private:
  std::optional<AMD::OdFanCurve::CurveDataSource>
  createCurveDataSource(IGPUInfo const &gpuInfo) const;
  std::optional<AMD::OdFanCurve::StopDataSource>
  createStopDataSource(IGPUInfo const &gpuInfo) const;

  static bool const registered_;
};

} // namespace AMD
