// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021 Juan Palacios <jpalaciosdev@gmail.com>

#include "../sensorgraphitem.h"
#include "core/qmlcomponentregistry.h"
#include "gpuvolt.h"
#include <QtGlobal>
#include <units.h>

namespace AMD::GPUVolt {

bool const registered_ = QMLComponentRegistry::addQuickItemProvider(
    AMD::GPUVolt::ItemID, []() {
      return new SensorGraphItem<units::voltage::millivolt_t, int>(
          AMD::GPUVolt::ItemID, "mV");
    });

char const *const trStrings[] = {
    QT_TRANSLATE_NOOP("SensorGraph", "AMD_GPU_VOLT"),
};

} // namespace AMD::GPUVolt
