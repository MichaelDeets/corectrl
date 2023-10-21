// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/components/controls/controlmode.h"
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

namespace AMD {

class PMPerfMode : public ControlMode
{
 public:
  static constexpr std::string_view ItemID{"AMD_PM_PERFMODE"};

  PMPerfMode(std::vector<std::unique_ptr<IControl>> &&controls) noexcept
  : ControlMode(AMD::PMPerfMode::ItemID, std::move(controls), true)
  {
  }
};

} // namespace AMD
