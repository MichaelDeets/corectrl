// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include <units.h>

class IHelperControl
{
 public:
  using MinExitTimeout = units::unit_value_t<units::time::milliseconds, 1000>;

  virtual void init(units::time::millisecond_t autoExitTimeout) = 0;
  virtual void stop() = 0;

  virtual ~IHelperControl() = default;
};
