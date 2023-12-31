// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "../sensorgraphitem.h"
#include "activity.h"
#include "core/qmlcomponentregistry.h"
#include <QtGlobal>
#include <units.h>

namespace AMD::Activity {

bool const registered_ = QMLComponentRegistry::addQuickItemProvider(
    AMD::Activity::ItemID, []() {
      return new SensorGraphItem<units::dimensionless::scalar_t, unsigned int>(
          AMD::Activity::ItemID, "%");
    });

char const *const trStrings[] = {
    QT_TRANSLATE_NOOP("SensorGraph", "AMD_ACTIVITY"),
};

} // namespace AMD::Activity
