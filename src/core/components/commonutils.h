// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include <units.h>
#include <utility>
#include <vector>

namespace Utils::Common {

/// Normalizes points into temperature and percentage ranges.
/// @note Temperature normalization is only performed when needed.
/// @param tempRange target temperature range
/// @param speedRange target speed range
void normalizePoints(
    std::vector<std::pair<units::temperature::celsius_t,
                          units::concentration::percent_t>> &points,
    std::pair<units::temperature::celsius_t, units::temperature::celsius_t> tempRange,
    std::pair<units::concentration::percent_t, units::concentration::percent_t>
        speedRange = std::make_pair(units::concentration::percent_t(0),
                                    units::concentration::percent_t(100)));

} // namespace Utils::Common
