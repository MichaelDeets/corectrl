// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2022 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "../igpuinfo.h"
#include "core/idatasource.h"
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace AMD {

class GPUInfoUniqueID final : public IGPUInfo::IProvider
{
 public:
  GPUInfoUniqueID(
      std::unique_ptr<IDataSource<std::string, std::filesystem::path const>>
          &&dataSource) noexcept;

  std::vector<std::pair<std::string, std::string>>
  provideInfo(Vendor vendor, int gpuIndex, IGPUInfo::Path const &path,
              IHWIDTranslator const &hwIDTranslator) const override;

  std::vector<std::string>
  provideCapabilities(Vendor vendor, int,
                      IGPUInfo::Path const &path) const override;

 private:
  std::unique_ptr<IDataSource<std::string, std::filesystem::path const>> const
      dataSource_;
  static bool registered_;
};

} // namespace AMD
