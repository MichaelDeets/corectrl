// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2022 Milan Cermak <krysmanta@post.cz>

#pragma once

#include <string_view>

/// A sensor that monitors the CPU usage.
namespace CPUUsage {

static constexpr std::string_view ItemID{"CPU_USAGE"};

} // namespace CPUUsage
