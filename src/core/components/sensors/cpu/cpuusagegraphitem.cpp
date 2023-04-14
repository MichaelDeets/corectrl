// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2022 Milan Cermak <krysmanta@post.cz>

#include "../sensorgraphitem.h"
#include "core/qmlcomponentregistry.h"
#include "cpuusage.h"
#include <QtGlobal>
#include <units.h>

namespace CPUUsage {

bool const registered_ = QMLComponentRegistry::addQuickItemProvider(
    CPUUsage::ItemID, []() {
      return new SensorGraphItem<units::dimensionless::scalar_t, unsigned int>(
          CPUUsage::ItemID, "%");
    });

char const *const trStrings[] = {
    QT_TRANSLATE_NOOP("SensorGraph", "CPU_USAGE"),
};

} // namespace CPUUsage
