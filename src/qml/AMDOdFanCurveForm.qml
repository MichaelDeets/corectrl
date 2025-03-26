// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CoreCtrl.UIComponents 1.0
import "Style.js" as Style

AMD_OD_FAN_CURVE {
  id: fanCurve
  objectName: "AMD_OD_FAN_CURVE"

  width: contents.width
  height: contents.height

  onCurveChanged: curve => {
    curveControl.removeCurve("curve")
    curveControl.addCurve("curve", Style.Theme.accent, curve)
  }

  onCurveRangeChanged: (tempMin, tempMax, speedMin, speedMax) => {
    curveControl.configureAxes(qsTr("Temperature"), "\u00B0C", tempMin, tempMax,
                               qsTr("Speed"), "%", speedMin, speedMax)
  }

  onStopAvailable: { p.showFanStopControl() }
  onStopTempAvailable: { p.showFanStopTempControl() }

  onStopChanged: enabled => {
    if (p.fanStopTempAvailable) {
      if (enabled)
        p.showStopCurve()
      else
        p.hideStopCurve()
    }

    swStop.checked = enabled
  }

  onStopTempChanged: value => {
    if (p.fanStopTempAvailable && fanCurve.stop) {
      p.hideStopCurve()
      p.showStopCurve()
    }

    iStopTemp.value = value
  }

  QtObject { // private stuff
    id: p

    property bool fanStopAvailable: false
    property bool fanStopTempAvailable: false

    function showFanStopControl() {
      fanStopAvailable = true
      rlStop.visible = fanStopAvailable
      // The stop temperature input field is a child of rlStop and
      // it will be set to visible automatically when rsStop is set
      // to visible.
      // Make sure that iStopTemp visible state is in sync with
      // fanStopTempAvailable state.
      iStopTemp.visible = fanStopTempAvailable
    }

    function showFanStopTempControl() {
      fanStopTempAvailable = true
      iStopTemp.visible = fanStopTempAvailable
    }

    function hideStopCurve() {
      curveControl.removeCurve("stop")
    }

    function showStopCurve() {
      var points = []
      points.push(Qt.point(fanCurve.stopTemp,
                           fanCurve.minTemp +
                          (fanCurve.maxTemp - fanCurve.minTemp) * .15))
      curveControl.addCurve("stop",
                            Style.CurveControl.curve_amd_start_color,
                            points, true)
    }
  }

  TextMetrics {
    id: tFMetrics
    text: "100"
  }

  CPane {
    id: contents

    ColumnLayout {
      spacing: 8

      CurveControl {
        id: curveControl
        minXDistance: 5
        clampPointsYCoordinate: true
        width: 480
        height: 240

        onCurveChanged: (name, oldPoint, newPoint) => {
          if (name === "curve")
            fanCurve.updateCurvePoint(oldPoint, newPoint)
          else if (name === "stop")
            fanCurve.stopTemp = parseInt(Math.round(newPoint.x))
        }
      }

      RowLayout {
        id: rlStop
        visible: false
        spacing: 8

        Item {
          Layout.fillWidth: true
        }

        Item {
          implicitWidth: swStop.width / (1 + Style.g_tweakScale)

          Switch {
            id: swStop

            scale: Style.g_tweakScale
            anchors.centerIn: parent

            onToggled: fanCurve.stop = checked
          }
        }

        Label {
          text: qsTr("Fan stop") + (p.fanStopTempAvailable ? " (\u00B0C)" : "")
          enabled: swStop.checked
        }

        CIntInput {
          id: iStopTemp

          minValue: 0
          maxValue: 100

          enabled: swStop.checked
          Layout.preferredWidth: tFMetrics.width + padding * 2

          onValueChanged: fanCurve.stopTemp = value
        }
      }
    }
  }
}
