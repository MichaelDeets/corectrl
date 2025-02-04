// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style.js" as Style

ComboBox {
  id: comboBox
  textRole: 'text'
  implicitWidth: modelWidth + rightPadding + leftPadding
  wheelEnabled: true

  property real modelWidth

  function updateWidth() {
    modelWidth = 0;

    for (var i = 0; i < model.count; i++) {
      metrics.text = model.get(i).text
      modelWidth = Math.max(metrics.width, modelWidth)
    }

    metrics.text = "##" // add extra space for the ComboBox decoration
    modelWidth += metrics.width
  }

  TextMetrics {
    id: metrics
  }
}
