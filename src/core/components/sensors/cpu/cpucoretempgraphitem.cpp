// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2022 Milan Cermak <krysmanta@post.cz>

#include "../sensorgraphitem.h"
#include "core/qmlcomponentregistry.h"
#include "cpucoretemp.h"
#include <QtGlobal>
#include <units.h>

namespace CPUCoreTemp {

bool const registered_ = QMLComponentRegistry::addQuickItemProvider(
    CPUCoreTemp::ItemID, []() {
      return new SensorGraphItem<units::temperature::celsius_t, int>(
          CPUCoreTemp::ItemID, "\u00B0C");
    });

char const *const trStrings[] = {
    QT_TRANSLATE_NOOP("SensorGraph", "CPU_CORE_TEMP"),
};

} // namespace CPUCoreTemp
