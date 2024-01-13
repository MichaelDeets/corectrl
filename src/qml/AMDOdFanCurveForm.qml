// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import CoreCtrl.UIComponents 1.0
import "Style.js" as Style

AMD_OD_FAN_CURVE {
  id: fanCurve
  objectName: "AMD_OD_FAN_CURVE"

  width: contents.width
  height: contents.height

  onCurveChanged: {
    curveControl.removeCurve("curve")
    curveControl.addCurve("curve", Material.accent, curve)
  }

  onCurveRangeChanged: {
    curveControl.configureAxes(qsTr("Temperature"), "\u00B0C", tempMin, tempMax,
                               qsTr("Speed"), "%", speedMin, speedMax)
  }

  TextMetrics {
    id: tFMetrics
    text: "100"
  }

  Pane {
    id: contents
    padding: Style.g_padding

    ColumnLayout {
      spacing: 8

      CurveControl {
        id: curveControl
        minXDistance: 5
        clampPointsYCoordinate: true
        width: 480
        height: 240

        onCurveChanged: fanCurve.updateCurvePoint(oldPoint, newPoint)
      }
    }
  }
}
