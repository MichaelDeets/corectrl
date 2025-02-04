// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

import QtQuick 2.15
import CoreCtrl.UIComponents 1.0
import "Style.js" as Style

AMD_PM_FREQ_MODE {
  id: freqMode
  objectName: "AMD_PM_FREQ_MODE"

  width: modeSelector.width
  height: modeSelector.height

  onModesChanged: modes => modeSelector.setModes(modes)
  onModeChanged: mode => modeSelector.select(mode)

  ModeSelector {
    id: modeSelector
    headerTitle: qsTr("Frequency")
    headerBackground: Style.ModeSelector.Body.background
    contentParentObject: "AMD_PM_FREQ_MODE_Plug"

    onSelectionChanged: mode => freqMode.changeMode(mode)
    onChildAdded: child => freqMode.setupChild(child)
  }
}
