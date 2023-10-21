// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2022 Milan Cermak <krysmanta@post.cz>

#pragma once

#include <string_view>

/// A sensor that monitors the temperature of Intel CPUs.
///
/// This sensor uses the coretemp driver:
/// https://docs.kernel.org/hwmon/coretemp.html
namespace CPUCoreTemp {

static constexpr std::string_view ItemID{"CPU_CORE_TEMP"};

} // namespace CPUCoreTemp
