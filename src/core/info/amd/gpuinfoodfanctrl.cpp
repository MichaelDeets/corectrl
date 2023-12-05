// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#include "gpuinfoodfanctrl.h"

#include "common/fileutils.h"
#include "core/info/infoproviderregistry.h"
#include <easylogging++.h>
#include <exception>
#include <filesystem>
#include <memory>

AMD::GPUInfoOdFanCtrl::GPUInfoOdFanCtrl() noexcept
{
}

std::vector<std::pair<std::string, std::string>>
AMD::GPUInfoOdFanCtrl::provideInfo(Vendor, int, IGPUInfo::Path const &,
                                   IHWIDTranslator const &) const
{
  return {};
}

std::vector<std::string>
AMD::GPUInfoOdFanCtrl::provideCapabilities(Vendor vendor, int,
                                           IGPUInfo::Path const &path) const
{
  std::vector<std::string> cap;

  if (vendor == Vendor::AMD) {
    auto fanCtlPath = path.sys / "gpu_od" / "fan_ctrl";
    try {
      if (Utils::File::isDirectoryPathValid(fanCtlPath) &&
          !std::filesystem::is_empty(fanCtlPath)) {
        cap.emplace_back(GPUInfoOdFanCtrl::ID);
      }
    }
    catch (std::exception const &e) {
      LOG(ERROR) << e.what();
    }
  }

  return cap;
}

bool AMD::GPUInfoOdFanCtrl::registered_ =
    InfoProviderRegistry::add(std::make_unique<AMD::GPUInfoOdFanCtrl>());
