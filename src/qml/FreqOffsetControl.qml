// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Juan Palacios <jpalaciosdev@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "Style.js" as Style

Pane {
  id: control

  property alias title: title.text
  property alias min: offsetSld.from
  property alias max: offsetSld.to
  property alias value: offsetSld.value

  signal offsetChanged(int value)

  padding: Style.g_padding
  background: Rectangle {
    border.color: Style.FVControl.border_color
    border.width: 2
    color: "#00000000"
  }

  TextMetrics {
    id: tFMetrics
    text: "-000"
  }

  ColumnLayout {
    anchors.fill: parent

    Label {
      id: title
      font.pointSize: 11
      font.bold: true
    }

    CPane {
      padding: Style.FVControl.inner_padding
      bottomPadding: 0

      ColumnLayout {
        Layout.fillWidth: true

        Label {
          text: qsTr("OFFSET")
          Layout.alignment: Qt.AlignHCenter
        }

        Label {
          text: "MHz"
          font.pointSize: 8
          Layout.alignment: Qt.AlignHCenter
        }

        Slider {
          id: offsetSld

          Layout.alignment: Qt.AlignHCenter
          orientation: Qt.Vertical

          stepSize: 1

          onPressedChanged: pressed => {
            if (!pressed)
              control.offsetChanged(value)
          }
        }

        CIntInput {
          value: offsetSld.value
          minValue: offsetSld.from
          maxValue: offsetSld.to
          Layout.preferredWidth: tFMetrics.width + padding * 2
          Layout.alignment: Qt.AlignHCenter

          onValueChanged: control.offsetChanged(value)
        }
      }
    }
  }
}
