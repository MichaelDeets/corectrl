// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtCharts 2.15
import "Style.js" as Style

ChartView {
  property real minXDistance: 1.0
  property bool clampPointsYCoordinate: false

  property alias xTickCount: xAxis.tickCount
  property alias xMinorTickCount: xAxis.minorTickCount
  property alias yTickCount: yAxis.tickCount
  property alias yMinorTickCount: yAxis.minorTickCount

  id: chart

  antialiasing: true
  legend.visible: false
  backgroundColor: "#00000000"

  margins.top: 0
  margins.bottom: 0
  margins.left: 0
  margins.right: 0

  axes: [ValueAxis {
          id: xAxis
          labelFormat: Style.CurveControl.axis_label_format
          minorGridVisible: true
          minorTickCount: 1
          tickCount: 5
         },
         ValueAxis {
          id: yAxis
          labelFormat: Style.CurveControl.axis_label_format
          minorGridVisible: true
          minorTickCount: 1
          tickCount: 3
         }]

  signal curveChanged(var name, var oldPoint, var newPoint)

  function configureAxes(xName, xUnit, xMin, xMax, yName, yUnit, yMin, yMax) {

    p.axisInfo = {
      x: {
        name: xName,
        unit: xUnit,
        min: xMin,
        max: xMax
      },
      y: {
        name: yName,
        unit: yUnit,
        min: yMin,
        max: yMax
      }
    }

    p.refreshAxes(p.axisInfo, enabled)

    // update outer range line series points
    p.curves.forEach(series => {
      const oldFirst = series.line.at(0)
      const oldLast = series.line.at(line.count - 1)
      series.line.replace(oldFirst.x, oldFirst.y, xMin - 1, oldFirst.y)
      series.line.replace(oldLast.x, oldLast.y, xMax + 1, oldLast.y)
    })
  }

  function addCurve(name, color, points) {
    // remove existing curve
    if (p.curves.has(name))
      removeCurve(name)

    const line = p.createLineSeries(name, color, points)
    const controls = p.createControlSeries(name, color, points)

    p.curves.set(name, {
      line: line,
      points: controls
    })
  }

  function removeCurve(name) {
    if (p.curves.has(name)) {
      const {line, points} = p.curves.get(name)
      chart.removeSeries(line)
      chart.removeSeries(points)
      p.curves.delete(name)
    }
  }

  onEnabledChanged: { p.onEnableChanged(enabled) }

  QtObject { // private stuff
    id: p

    property var curves: new Map()
    property var selection: undefined
    property var axisInfo: undefined

    function createLineSeries(name, color, points) {
      var series = chart.createSeries(ChartView.SeriesTypeLine,
                                      name, xAxis, yAxis)
      series.name = name
      series.color = color
      series.opacity = chart.enabled ? Style.CurveControl.curve_opacity
                                     : Style.CurveControl.curve_opacity_alt
      series.width = 2

      // line first outer range point
      series.append(axisInfo.x.min - 1, points[0].y)

      for (var i = 0; i < points.length; ++i)
        series.append(points[i].x, points[i].y)

      // line last outer range point
      series.append(axisInfo.x.max + 1, points[points.length - 1].y)

      return series
    }

    function createControlSeries(name, color, points) {
      var series = chart.createSeries(ChartView.SeriesTypeScatter,
                                      name, xAxis, yAxis)
      series.name = name
      series.color = color
      series.opacity = chart.enabled ? Style.CurveControl.curve_opacity
                                     : Style.CurveControl.curve_opacity_alt
      series.borderColor = color
      series.markerSize = 10
      series.borderWidth = 0

      for (var i = 0; i < points.length; ++i)
        series.append(points[i].x, points[i].y)

      return series
    }

    function onEnableChanged(enabled) {
      refreshAxes(axisInfo, enabled)

      // series opacity
      var opacity = enabled ? Style.CurveControl.curve_opacity
                            : Style.CurveControl.curve_opacity_alt
      curves.forEach(series => {
        series.line.opacity = opacity
        series.points.opacity = opacity
      })
    }

    function hasCurves() {
      return curves.size > 0
    }

    function hasSelection() {
      return selection !== undefined
    }

    function mapToChart(point) {
      // use the first available point series to map the point
      for (const series of curves.values())
        return chart.mapToValue(point, series.points)
    }

    function refreshAxes({x, y}, enabled) {
      xAxis.min = x.min
      xAxis.max = x.max
      xAxis.titleText = "<font color='"+ (enabled ? Style.CurveControl.axis_title_color
                                                  : Style.CurveControl.axis_title_color_alt)
                        + "'>" + x.name + " (" + x.unit + ")</font>"
      xAxis.labelsColor = enabled ? Style.CurveControl.axis_label_color
                                  : Style.CurveControl.axis_label_color_alt
      xAxis.color = enabled ? Style.CurveControl.axis_color
                            : Style.CurveControl.axis_color_alt
      xAxis.gridLineColor = enabled ? Style.CurveControl.axis_grid_color
                                    : Style.CurveControl.axis_grid_color_alt
      xAxis.minorGridLineColor = enabled ? Style.CurveControl.axis_grid_minor_color
                                         : Style.CurveControl.axis_grid_minor_color_alt
      yAxis.min = y.min
      yAxis.max = y.max
      yAxis.titleText = "<font color='"+ (enabled ? Style.CurveControl.axis_title_color
                                                  : Style.CurveControl.axis_title_color_alt)
                        + "'>" + y.name + " (" + y.unit + ")</font>"
      yAxis.labelsColor = enabled ? Style.CurveControl.axis_label_color
                                  : Style.CurveControl.axis_label_color_alt
      yAxis.color = enabled ? Style.CurveControl.axis_color
                            : Style.CurveControl.axis_color_alt
      yAxis.gridLineColor = enabled ? Style.CurveControl.axis_grid_color
                                    : Style.CurveControl.axis_grid_color_alt
      yAxis.minorGridLineColor = enabled ? Style.CurveControl.axis_grid_minor_color
                                         : Style.CurveControl.axis_grid_minor_color_alt
    }

    function updateCurvePoint(curve, oldPoint, newPoint) {
      curve.line.replace(oldPoint.x, oldPoint.y, newPoint.x, newPoint.y)
      curve.points.replace(oldPoint.x, oldPoint.y, newPoint.x, newPoint.y)
    }

    function updateLineOuterRangePoint(curve, pointIndex, oldY, newY) {
      if (pointIndex === 0)
        curve.line.replace(axisInfo.x.min - 1, oldY, axisInfo.x.min - 1, newY)
      else if (pointIndex === curve.points.count - 1)
        curve.line.replace(axisInfo.x.max + 1, oldY, axisInfo.x.max + 1, newY)
    }

    function moveSelection(point) {
      // clamp point coordinates to axes range
      point.x = Math.min(Math.max(point.x, axisInfo.x.min), axisInfo.x.max)
      point.y = Math.min(Math.max(point.y, axisInfo.y.min), axisInfo.y.max)

      const curve = curves.get(selection.curve)

      // limit x movement between points using minXDistance
      if (selection.index > 0)
        point.x = Math.max(point.x, selection.prevX + minXDistance)
      if (selection.index < curve.points.count - 1)
        point.x = Math.min(point.x, selection.nextX - minXDistance)

      if (clampPointsYCoordinate)
        clampOtherPointsYCoordinate(curve, selection)

      updateCurvePoint(curve, selection.point, point)
      updateLineOuterRangePoint(curve, selection.index, selection.point.y, point.y)

      // emit curveChanged signal
      curveChanged(selection.curve, selection.point, point)

      selection.point = point

      return point
    }

    function clampOtherPointsYCoordinate(curve, selection) {
      var points = curve.points
      for (var i = 0; i < points.count; ++i) {
        // skip selected point
        if (selection.index === i)
          continue

        var point = points.at(i)
        if ((i < selection.index && point.y > selection.point.y) ||
            (i > selection.index && point.y < selection.point.y)) {
          point.y = selection.point.y
          var oldPoint = points.at(i)
          updateCurvePoint(curve, oldPoint, point)
          curveChanged(selection.curve, oldPoint, point)
        }
      }
    }

    function findCloserIndex(points, target) {
      var distRange = (xAxis.max - xAxis.min) / 20
      var closerPointIndex = -1
      var minDist = 10000
      for (var i = 0; i < points.count; ++i) {
        var point = points.at(i)
        var distance = Math.sqrt(Math.pow(point.x - target.x, 2) +
                                 Math.pow(point.y - target.y, 2))

        if (distance < minDist && distance < distRange) {
          minDist = distance
          closerPointIndex = i
        }
      }

      return closerPointIndex
    }

    function closerPointTo(point) {
      for (const [curve, series] of curves) {
        const selectedPointIndex = findCloserIndex(series.points, point)
        if (selectedPointIndex !== -1)
          return {
            curve: curve,
            point: series.points.at(selectedPointIndex),
            index: selectedPointIndex
          }
      }

      return undefined
    }

    function select(pointInfo) {
      if (pointInfo !== undefined) {
        const { curve, point, index } = pointInfo
        const points = curves.get(curve).points

        selection = {
          curve,
          point,
          index,
          prevX: index > 0 ? points.at(index - 1).x : undefined,
          nextX: index < points.count - 1 ? points.at(index + 1).x : undefined
        }
      }
      else {
        selection = undefined
      }
    }
  }

  ToolTip {
    id: tooltip
    delay: 0
    timeout: -1
    margins: -1

    function updatePosition(x, y) {
      x = Math.max(0, Math.min(x, Math.round(parent.width - width)))
      y = Math.max(0, Math.min(y + 25, Math.round(parent.height - height)))

      tooltip.x = x
      tooltip.y = y
    }
  }

  MouseArea {
    id: mouseArea
    anchors.fill: parent

    onPressed: mouse => {
      if (p.hasCurves()) {
        var outerPoint = p.mapToChart(Qt.point(mouse.x, mouse.y))

        var pointInfo = p.closerPointTo(outerPoint)
        p.select(pointInfo)

        // update tooltip
        if (pointInfo !== undefined) {
          const { point } = pointInfo

          tooltip.updatePosition(mouse.x, mouse.y)
          tooltip.text = Math.round(point.x) + p.axisInfo.x.unit + " " + Math.round(point.y) + p.axisInfo.y.unit
          tooltip.visible = true
        }
      }
    }

    onReleased: mouse => {
      if (p.hasCurves()) {
        p.select(undefined)
        tooltip.visible = false
      }
    }

    onPositionChanged: mouse => {
      if (p.hasCurves() && p.hasSelection()) {
        var point = p.mapToChart(Qt.point(mouse.x, mouse.y))
        point = p.moveSelection(point)

        // update tooltip
        tooltip.updatePosition(mouse.x, mouse.y)
        tooltip.text = Math.round(point.x) + p.axisInfo.x.unit + " " + Math.round(point.y) + p.axisInfo.y.unit
      }
    }
  }
}
