// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Juan Palacios <jpalaciosdev@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style.js" as Style

SplitView {
  id: spv
  spacing: 0

  handle: Rectangle {
    implicitWidth: spv.orientation === Qt.Horizontal ? Style.Split.thickness
                                                     : spv.width
    implicitHeight: spv.orientation === Qt.Horizontal ? spv.height
                                                     : Style.Split.thickness
    color: !spv.enabled ? Qt.lighter(Style.Split.background, 1.20)
                        : SplitHandle.hovered ? Qt.lighter(Style.Split.background, 1.15)
                                              : Style.Split.background
    Rectangle {
      color: Style.Split.Handle.background
      width: spv.orientation === Qt.Horizontal ? Style.Split.Handle.thickness
                                               : Style.Split.Handle.length
      height: spv.orientation === Qt.Horizontal ? Style.Split.Handle.length
                                                : Style.Split.Handle.thickness
      x: (parent.width - width) / 2
      y: (parent.height - height) / 2
    }
  }
}
