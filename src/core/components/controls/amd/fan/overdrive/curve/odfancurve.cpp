// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#include "odfancurve.h"

#include "core/components/amdutils.h"
#include "core/components/commonutils.h"
#include "core/icommandqueue.h"
#include <algorithm>
#include <cmath>
#include <iterator>

AMD::OdFanCurve::OdFanCurve(CurveDataSource &&curveDataSource,
                            std::optional<StopDataSource> &&stopDataSource) noexcept
: Control(false)
, id_(AMD::OdFanCurve::ItemID)
, curveDataSource_(std::move(curveDataSource))
, stopDataSource_(std::move(stopDataSource))
, triggerManualOpMode_(true)
{
}

void AMD::OdFanCurve::preInit(ICommandQueue &ctlCmds)
{
  if (!curveDataSource_.curve->read(dataSourceLines_))
    return;

  preInitControlPoints_ =
      Utils::AMD::parseOverdriveFanCurve(dataSourceLines_).value();

  if (stopDataSource_) {
    if (!stopDataSource_->enable->read(dataSourceLines_))
      return;

    preInitStop_ = Utils::AMD::parseOverdriveFanStop(dataSourceLines_).value();

    if (!stopDataSource_->temperature->read(dataSourceLines_))
      return;

    preInitStopTemp_ =
        Utils::AMD::parseOverdriveFanStopTemp(dataSourceLines_).value();
  }

  addResetCmds(ctlCmds);
}

void AMD::OdFanCurve::postInit(ICommandQueue &ctlCmds)
{
  if (!isZeroCurve(preInitControlPoints_)) {
    normalizeCurve(preInitControlPoints_, tempRange(), speedRange());
    for (auto const &point : preInitControlPoints_)
      ctlCmds.add({curveDataSource_.curve->source(), controlPointCmd(point)});

    ctlCmds.add({curveDataSource_.curve->source(), "c"});
  }

  if (stopDataSource_) {
    ctlCmds.add(
        {stopDataSource_->enable->source(), std::to_string(preInitStop_)});
    ctlCmds.add({stopDataSource_->enable->source(), "c"});
    ctlCmds.add({stopDataSource_->temperature->source(),
                 std::to_string(preInitStopTemp_.to<int>())});
    ctlCmds.add({stopDataSource_->temperature->source(), "c"});
  }
}

void AMD::OdFanCurve::init()
{
  if (!curveDataSource_.curve->read(dataSourceLines_))
    return;

  controlPoints_ = Utils::AMD::parseOverdriveFanCurve(dataSourceLines_).value();
  if (isZeroCurve(controlPoints_))
    setPointCoordinatesFrom(controlPoints_, defaultCurve());

  normalizeCurve(controlPoints_, tempRange(), speedRange());

  if (stopDataSource_) {
    if (!stopDataSource_->enable->read(dataSourceLines_))
      return;

    stop_ = Utils::AMD::parseOverdriveFanStop(dataSourceLines_).value();

    if (!stopDataSource_->temperature->read(dataSourceLines_))
      return;

    stopTemp_ = Utils::AMD::parseOverdriveFanStopTemp(dataSourceLines_).value();
  }
}

std::string const &AMD::OdFanCurve::ID() const
{
  return id_;
}

void AMD::OdFanCurve::importControl(IControl::Importer &i)
{
  auto &fanCurveImporter = dynamic_cast<AMD::OdFanCurve::Importer &>(i);
  fanCurve(fanCurveImporter.provideFanCurve());

  if (stopDataSource_) {
    stop(fanCurveImporter.provideFanStop());
    stopTemp(fanCurveImporter.provideFanStopTemp());
  }
}

void AMD::OdFanCurve::exportControl(IControl::Exporter &e) const
{
  auto &fanCurveExporter = dynamic_cast<AMD::OdFanCurve::Exporter &>(e);
  fanCurveExporter.takeFanCurveRange(tempRange(), speedRange());
  fanCurveExporter.takeFanCurve(fanCurve());

  if (stopDataSource_) {
    fanCurveExporter.takeFanStop(stop());
    fanCurveExporter.takeFanStopTempRange(stopTempRange());
    fanCurveExporter.takeFanStopTemp(stopTemp());
  }
}

void AMD::OdFanCurve::cleanControl(ICommandQueue &ctlCmds)
{
  addResetCmds(ctlCmds);
  triggerManualOpMode_ = true;
}

void AMD::OdFanCurve::syncControl(ICommandQueue &ctlCmds)
{
  if (!curveDataSource_.curve->read(dataSourceLines_))
    return;
  auto curve = Utils::AMD::parseOverdriveFanCurve(dataSourceLines_).value();
  bool outOfSync = addCurveSyncCmds(ctlCmds, std::move(curve));

  if (stopDataSource_) {
    if (!stopDataSource_->enable->read(dataSourceLines_))
      return;
    auto stop = Utils::AMD::parseOverdriveFanStop(dataSourceLines_).value();

    if (!stopDataSource_->temperature->read(dataSourceLines_))
      return;
    auto temp = Utils::AMD::parseOverdriveFanStopTemp(dataSourceLines_).value();

    outOfSync |= addStopSyncCmds(ctlCmds, stop, temp);
  }

  if (triggerManualOpMode_ && !outOfSync) {
    // NOTE The new fan overdrive interfaces has an implicit operation mode [1]
    // (auto or manual, used for the fan_curve interface) that is selected by
    // the last interface the user interacts with. However, a change to the
    // curve must be submitted to actually trigger the operation mode (either
    // different points or a reset command) [2].
    //
    // Trigger the manual operation mode by sending a reset command when the
    // curve is already in sync.
    //
    // [1] https://www.kernel.org/doc/html/v6.7/gpu/amdgpu/thermal.html#fan-curve
    // [2] https://gitlab.freedesktop.org/drm/amd/-/issues/2402#note_2211197
    addResetCmds(ctlCmds);
    triggerManualOpMode_ = false;
  }
}

std::vector<AMD::OdFanCurve::CurvePoint> AMD::OdFanCurve::defaultCurve() const
{
  // clang-format off
  std::vector<std::pair<units::temperature::celsius_t, units::concentration::percent_t>>
      defaultCurve = {
        {units::temperature::celsius_t(35), units::concentration::percent_t(20)},
        {units::temperature::celsius_t(52), units::concentration::percent_t(22)},
        {units::temperature::celsius_t(67), units::concentration::percent_t(30)},
        {units::temperature::celsius_t(78), units::concentration::percent_t(50)},
        {units::temperature::celsius_t(85), units::concentration::percent_t(82)}
      };
  // clang-format on
  return defaultCurve;
}

std::vector<AMD::OdFanCurve::CurvePoint> AMD::OdFanCurve::fanCurve() const
{
  return toCurvePoints(controlPoints());
}

void AMD::OdFanCurve::fanCurve(std::vector<AMD::OdFanCurve::CurvePoint> points)
{
  Utils::Common::normalizePoints(points, tempRange(), speedRange());
  setPointCoordinatesFrom(controlPoints_, points);
}

std::vector<AMD::OdFanCurve::ControlPoint> const &
AMD::OdFanCurve::controlPoints() const
{
  return controlPoints_;
}

AMD::OdFanCurve::TempRange const &AMD::OdFanCurve::tempRange() const
{
  return curveDataSource_.temperatureRange;
}

AMD::OdFanCurve::SpeedRange const &AMD::OdFanCurve::speedRange() const
{
  return curveDataSource_.speedRange;
}

std::vector<AMD::OdFanCurve::CurvePoint> AMD::OdFanCurve::toCurvePoints(
    std::vector<AMD::OdFanCurve::ControlPoint> const &curve) const
{
  std::vector<AMD::OdFanCurve::CurvePoint> points;
  std::transform(curve.cbegin(), curve.cend(), std::back_inserter(points),
                 [](auto const &point) {
                   return std::make_pair(std::get<1>(point), std::get<2>(point));
                 });
  return points;
}

void AMD::OdFanCurve::setPointCoordinatesFrom(
    std::vector<AMD::OdFanCurve::ControlPoint> &curve,
    std::vector<AMD::OdFanCurve::CurvePoint> const &values) const
{
  if (values.empty())
    return;

  size_t i = 0;
  for (auto &point : curve) {
    auto value = values[i++];
    std::get<1>(point) = value.first;
    std::get<2>(point) = value.second;

    if (i == values.size())
      break;
  }
}

bool AMD::OdFanCurve::isZeroCurve(
    std::vector<AMD::OdFanCurve::ControlPoint> const &curve) const
{
  // NOTE The overdrive curve interface reports all default device curve points
  // as zero point coordinates [1].
  //
  // [1] https://gitlab.freedesktop.org/drm/amd/-/issues/2402#note_2210868
  return std::all_of(curve.cbegin(), curve.cend(), [](auto const &point) {
    return std::get<1>(point) == units::temperature::celsius_t(0) &&
           std::get<2>(point) == units::concentration::percent_t(0);
  });
}

std::string AMD::OdFanCurve::controlPointCmd(ControlPoint const &point) const
{
  std::string cmd;
  cmd.reserve(10);
  cmd.append(std::to_string(std::get<0>(point)))
      .append(" ")
      .append(std::to_string(std::get<1>(point).to<int>()))
      .append(" ")
      .append(std::to_string(std::lround(std::get<2>(point).to<double>() * 100)));
  return cmd;
}

bool AMD::OdFanCurve::stop() const
{
  return stop_;
}

void AMD::OdFanCurve::stop(bool value)
{
  stop_ = value;
}

units::temperature::celsius_t AMD::OdFanCurve::stopTemp() const
{
  return stopTemp_;
}

void AMD::OdFanCurve::stopTemp(units::temperature::celsius_t value)
{
  stopTemp_ = value;
}

AMD::OdFanCurve::TempRange const &AMD::OdFanCurve::stopTempRange() const
{
  return stopDataSource_->temperatureRange;
}

void AMD::OdFanCurve::normalizeCurve(
    std::vector<AMD::OdFanCurve::ControlPoint> &curve,
    AMD::OdFanCurve::TempRange const &tempRange,
    AMD::OdFanCurve::SpeedRange const &speedRange) const
{
  auto normalizedPoints = toCurvePoints(curve);
  Utils::Common::normalizePoints(normalizedPoints, tempRange, speedRange);
  setPointCoordinatesFrom(curve, normalizedPoints);
}

bool AMD::OdFanCurve::addCurveSyncCmds(ICommandQueue &ctlCmds,
                                       std::vector<ControlPoint> &&curve) const
{
  bool commit = false;
  size_t curveIndex = 0;
  for (auto const &point : controlPoints()) {
    auto const [_, temp, speed] = curve[curveIndex++];
    if (temp != std::get<1>(point) || speed != std::get<2>(point)) {
      ctlCmds.add({curveDataSource_.curve->source(), controlPointCmd(point)});
      commit = true;
    }
  }

  if (commit)
    ctlCmds.add({curveDataSource_.curve->source(), "c"});

  return commit;
}

bool AMD::OdFanCurve::addStopSyncCmds(ICommandQueue &ctlCmds, bool hwStop,
                                      units::temperature::celsius_t hwTemp) const
{
  bool sync = false;

  if (stop() != hwStop) {
    ctlCmds.add({stopDataSource_->enable->source(), std::to_string(stop())});
    ctlCmds.add({stopDataSource_->enable->source(), "c"});
    sync |= true;
  }

  if (stopTemp() != hwTemp) {
    ctlCmds.add({stopDataSource_->temperature->source(),
                 std::to_string(stopTemp().to<int>())});
    ctlCmds.add({stopDataSource_->temperature->source(), "c"});
    sync |= true;
  }

  return sync;
}

void AMD::OdFanCurve::addResetCmds(ICommandQueue &ctlCmds) const
{
  ctlCmds.add({curveDataSource_.curve->source(), "r"});

  // NOTE Apparently, there is no need to submit the commit command after the
  // reset one on the new fan overdrive interfaces [1]. This interaction model
  // digress from the overdrive clock-voltage interface model, where the reset
  // command only restores the default values without actually committing them.
  //
  // Submit a commit command just in case this is changed to align with the
  // original overdrive interface interaction model.
  //
  // [1] https://gitlab.freedesktop.org/drm/amd/-/issues/2402#note_2211197
  ctlCmds.add({curveDataSource_.curve->source(), "c"});

  if (stopDataSource_) {
    ctlCmds.add({stopDataSource_->enable->source(), "r"});
    ctlCmds.add({stopDataSource_->enable->source(), "c"});
    ctlCmds.add({stopDataSource_->temperature->source(), "r"});
    ctlCmds.add({stopDataSource_->temperature->source(), "c"});
  }
}
