// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "../sensorgraphitem.h"
#include "core/qmlcomponentregistry.h"
#include "fanspeedperc.h"
#include <QtGlobal>
#include <units.h>

namespace AMD::FanSpeedPerc {

bool const registered_ = QMLComponentRegistry::addQuickItemProvider(
    AMD::FanSpeedPerc::ItemID, []() {
      return new SensorGraphItem<units::dimensionless::scalar_t, unsigned int>(
          AMD::FanSpeedPerc::ItemID, "%");
    });

char const *const trStrings[] = {
    QT_TRANSLATE_NOOP("SensorGraph", "AMD_FAN_SPEED_PERC"),
};

} // namespace AMD::FanSpeedPerc
