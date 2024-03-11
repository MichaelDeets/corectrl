// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

import QtQuick 2.15
import CoreCtrl.UIComponents 1.0

AMD_FAN_MODE {
  id: fMode
  objectName: "AMD_FAN_MODE"

  width: modeSelector.width
  height: modeSelector.height

  onModesChanged: modes => modeSelector.setModes(modes)
  onModeChanged: mode => modeSelector.select(mode)

  ModeSelector {
    id: modeSelector
    headerTitle: qsTr("Ventilation")
    contentParentObject: "AMD_FAN_MODE_Plug"

    onSelectionChanged: mode => fMode.changeMode(mode)
    onChildAdded: child => fMode.setupChild(child)
  }
}
