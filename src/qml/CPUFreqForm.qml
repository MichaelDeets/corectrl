// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import CoreCtrl.UIComponents 1.0
import "Style.js" as Style

CPU_CPUFREQ {
  id: cpuFreq
  objectName: "CPU_CPUFREQ"

  width: contents.width
  height: contents.height

  onScalingGovernorsChanged: governors => {
    scalingGovernorListModel.clear()

    for (var i = 0; i < governors.length; i+=2) {
      var element = scalingGovernorListElement.createObject()
      element.governor= governors[i]
      element.text = governors[i + 1]
      scalingGovernorListModel.append(element)
    }

    cbScalingGovernor.updateWidth()
  }

  onScalingGovernorChanged: governor => {
    for (var i = 0; i < scalingGovernorListModel.count; ++i) {
      if (scalingGovernorListModel.get(i).governor === governor) {
        cbScalingGovernor.lastIndex = i
        cbScalingGovernor.currentIndex = i
        break;
      }
    }
  }

  onEppHintsChanged: hints => {
    eppHintListModel.clear()

    for (var i = 0; i < hints.length; i+=2) {
      var element = eppHintListElement.createObject()
      element.hint = hints[i]
      element.text = hints[i + 1]
      eppHintListModel.append(element)
    }

    cbEppHint.updateWidth()
  }

  onEppHintChanged: hint => {
    for (var i = 0; i < eppHintListModel.count; ++i) {
      if (eppHintListModel.get(i).hint === hint) {
        cbEppHint.lastIndex = i
        cbEppHint.currentIndex = i
        break;
      }
    }
  }

  onToggleEppHint: enable => {
    lbEppHint.visible = enable
    cbEppHint.visible = enable
  }

  ListModel { id: scalingGovernorListModel }
  Component {
    id: scalingGovernorListElement

    ListElement {
      property string text
      property string governor
    }
  }

  ListModel { id: eppHintListModel }
  Component {
    id: eppHintListElement

    ListElement {
      property string text
      property string hint
    }
  }

  Pane {
    id: contents
    padding: Style.g_padding

    GridLayout {
      columns: 2
      columnSpacing: 10

      Label {
        text: qsTr("Frequency governor")
        Layout.alignment: Qt.AlignRight
      }

      CComboBox {
        id: cbScalingGovernor
        model: scalingGovernorListModel
        Layout.alignment: Qt.AlignLeft
        Layout.fillWidth: true

        property int lastIndex: 0

        onActivated: {
          if (lastIndex !== currentIndex) {
            lastIndex = currentIndex

            var governor = model.get(currentIndex).governor
            cpuFreq.changeScalingGovernor(governor)
          }
        }
      }

      Label {
        id: lbEppHint
        text: qsTr("Energy Performance Preference")
        Layout.alignment: Qt.AlignRight
        visible: false
      }

      CComboBox {
        id: cbEppHint
        model: eppHintListModel
        Layout.alignment: Qt.AlignLeft
        Layout.fillWidth: true
        visible: false

        property int lastIndex: 0

        onActivated: {
          if (lastIndex !== currentIndex) {
            lastIndex = currentIndex

            var hint = model.get(currentIndex).hint
            cpuFreq.changeEPPHint(hint)
          }
        }
      }
    }
  }
}
