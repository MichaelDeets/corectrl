// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Juan Palacios <jpalaciosdev@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CoreCtrl.UIComponents 1.0

AMD_PM_FREQ_OFFSET {
  id: pmFreqOffset
  objectName: "AMD_PM_FREQ_OFFSET"

  width: contents.width
  height: contents.height

  onControlLabelChanged: label => freqOffset.title = label
  onOffsetRangeChanged: (min, max) => {
    freqOffset.min = min
    freqOffset.max = max
  }
  onOffsetChanged: value => freqOffset.value = value

  CPane {
    id: contents
    padding: 0

    RowLayout {
      FreqOffsetControl {
        id: freqOffset
        Layout.fillHeight: true

        onOffsetChanged: value => pmFreqOffset.changeOffset(value)
      }
    }
  }
}
