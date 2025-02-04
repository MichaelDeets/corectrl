// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Juan Palacios <jpalaciosdev@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "Style.js" as Style

TabButton {
  id: btn
  property bool active: true

  contentItem: Text {
    text: btn.text
    font: btn.font
    color: btn.checked ? Style.Theme.accent
                       : Style.Theme.foreground
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    elide: Text.ElideRight
  }

  background: ColumnLayout {
    anchors.fill: parent
    spacing: 0

    // content
    Rectangle {
      color: btn.hovered ? Style.TabBar.Button.background_alt
                         : Style.TabBar.Button.background
      Layout.fillWidth: true
      Layout.fillHeight: true
    }
    // active accent
    Rectangle {
      visible: btn.checked
      color: btn.enabled && btn.active ? Style.Theme.accent
                                       : Style.Theme.accent_alt
      Layout.preferredHeight: Style.TabBar.Button.accent_size
      Layout.fillWidth: true
    }
  }
}
