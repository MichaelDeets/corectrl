// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "commonutils.h"

#include "common/mathutils.h"
#include <algorithm>
#include <cstddef>
#include <iterator>

namespace Utils::Common {

void normalizePoints(
    std::vector<std::pair<units::temperature::celsius_t,
                          units::concentration::percent_t>> &points,
    std::pair<units::temperature::celsius_t, units::temperature::celsius_t> tempRange,
    std::pair<units::concentration::percent_t, units::concentration::percent_t>
        speedRange)
{
  std::vector<double> temps;

  if (std::any_of(points.cbegin(), points.cend(), [&](auto const &point) {
        return point.first < tempRange.first || point.first > tempRange.second;
      })) {
    std::transform(
        points.cbegin(), points.cend(), std::back_inserter(temps),
        [](auto const &point) { return point.first.template to<double>(); });

    auto [minTemp, maxTemp] = std::minmax_element(temps.cbegin(), temps.cend());

    Utils::Math::linearNorm(
        temps, std::make_pair(std::min(0.0, *minTemp), std::max(90.0, *maxTemp)),
        std::make_pair(tempRange.first.to<double>(),
                       tempRange.second.to<double>()));
  }

  for (size_t i = 0; i < points.size(); ++i) {
    auto &[temp, pwm] = points.at(i);
    pwm = std::clamp(pwm, speedRange.first, speedRange.second);

    // ensure that point.pwm >= prevPoint.pwm
    if (points.size() > 1 && i > 0) {
      auto const &[_, prevPwm] = points.at(i - 1);
      if (pwm < prevPwm)
        pwm = prevPwm;
    }

    if (!temps.empty())
      temp = units::temperature::celsius_t(temps[i]);
  }
}

} // namespace Utils::Common
