// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021 Juan Palacios <jpalaciosdev@gmail.com>

#include "../sensorgraphitem.h"
#include "core/qmlcomponentregistry.h"
#include "junctiontemp.h"
#include <QtGlobal>
#include <units.h>

namespace AMD::JunctionTemp {

bool const registered_ = QMLComponentRegistry::addQuickItemProvider(
    AMD::JunctionTemp::ItemID, []() {
      return new SensorGraphItem<units::temperature::celsius_t, int>(
          AMD::JunctionTemp::ItemID, "\u00B0C");
    });

char const *const trStrings[] = {
    QT_TRANSLATE_NOOP("SensorGraph", "AMD_GPU_JUNCTION_TEMP"),
};

} // namespace AMD::JunctionTemp
