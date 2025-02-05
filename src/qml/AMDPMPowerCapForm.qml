// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CoreCtrl.UIComponents 1.0
import "Style.js" as Style

AMD_PM_POWERCAP {
  id: powerCap
  objectName: "AMD_PM_POWERCAP"

  width: contents.width
  height: contents.height

  onValueChanged: value => {
    powerValue.value = value
    valueInput.value = value
  }
  onRangeChanged: (min, max) => {
    powerValue.from = min
    powerValue.to = max
  }

  TextMetrics {
    id: tFMetrics
    text: "999"
  }

  CPane {
    id: contents

    RowLayout {

      Label { text: qsTr("Power limit") + " (W)" }

      Slider {
        id: powerValue

        from: 0
        to: 0
        stepSize: 1

        onPressedChanged: pressed => {
          if (!pressed)
            powerCap.changeValue(powerValue.value)
        }

        onMoved: valueInput.value = value
      }

      CIntInput {
        id: valueInput

        Layout.preferredWidth: tFMetrics.width + padding * 2

        minValue: powerValue.from
        maxValue: powerValue.to

        onValueChanged: powerCap.changeValue(value)
      }
    }
  }
}
