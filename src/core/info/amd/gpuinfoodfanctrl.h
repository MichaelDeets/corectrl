// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "../igpuinfo.h"
#include <string>
#include <string_view>

namespace AMD {

/// AMD GPU overdrive fan control info
class GPUInfoOdFanCtrl final : public IGPUInfo::IProvider
{
 public:
  static constexpr std::string_view ID{"odfanctrl"};

  GPUInfoOdFanCtrl() noexcept;

  std::vector<std::pair<std::string, std::string>>
  provideInfo(Vendor vendor, int gpuIndex, IGPUInfo::Path const &path,
              IHWIDTranslator const &hwIDTranslator) const override;

  std::vector<std::string>
  provideCapabilities(Vendor vendor, int gpuIndex,
                      IGPUInfo::Path const &path) const override;

 private:
  static bool registered_;
};

} // namespace AMD
