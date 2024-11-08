// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2021 Juan Palacios <jpalaciosdev@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CoreCtrl.UIComponents 1.0

AMD_PM_FREQ_RANGE {
  id: pmFreqRange
  objectName: "AMD_PM_FREQ_RANGE"

  width: contents.width
  height: contents.height

  onControlLabelChanged: label => freqState.title = label

  onStateRangeChanged: (min, max) => {
    p.min = min
    p.max = max

    freqState.setFreqRange(min, max)
  }

  onStatesChanged: states => freqState.setFStates(states, p.min, p.max)
  onSomeStateChanged: (index, freq) => freqState.updateFState(index, freq)

  QtObject {
    id: p

    property int min: 0
    property int max: 0
  }

  Pane {
    id: contents
    padding: 0

    RowLayout {
      FreqStateControl {
        id: freqState
        Layout.fillHeight: true

        onSomeStateChanged: (index, freq) => pmFreqRange.changeState(index, freq)
      }
    }
  }
}
