// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.15
import "Style.js" as Style
import "Settings.js" as Settings

ApplicationWindow {
  id: appWindow

  title: appInfo.name

  font.family: Style.g_text.family
  font.pointSize: Style.g_text.size
  font.weight: Font.Normal
  font.capitalization: Font.MixedCase

  Universal.theme: Universal.Dark
  Universal.accent: Style.Theme.accent
  Universal.foreground: Style.Theme.foreground
  Universal.background: Style.Theme.background

  StackLayout {
    id: components

    anchors.fill: parent
    currentIndex: tabBar.currentIndex

    Profiles {}
    System {}
  }

  onVisibleChanged: components.visible = visible

  footer: TabBar {
    id: tabBar
    font.bold: true
    font.capitalization: Font.AllUppercase
    hoverEnabled: Style.g_hover

    TabButtonMainFooter {
      text: qsTr("Profiles")
    }
    TabButtonMainFooter {
      text: qsTr("System")
    }

    background: Rectangle {} // HACK: avoid empty space bellow buttons
  }

  onClosing: close => {
    if (!systemTray.isAvailable() || !systemTray.isVisible())
      Qt.quit()
  }
}
