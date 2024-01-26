// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#include "odfancurve.h"

#include "core/components/amdutils.h"
#include "core/components/commonutils.h"
#include "core/icommandqueue.h"
#include <algorithm>
#include <cmath>
#include <iterator>

AMD::OdFanCurve::OdFanCurve(
    std::unique_ptr<IDataSource<std::vector<std::string>>> &&dataSource) noexcept
: Control(false)
, id_(AMD::OdFanCurve::ItemID)
, dataSource_(std::move(dataSource))
, triggerManualOpMode_(true)
{
}

void AMD::OdFanCurve::preInit(ICommandQueue &ctlCmds)
{
  if (!dataSource_->read(fanCurveLines_))
    return;

  preInitControlPoints_ =
      Utils::AMD::parseOverdriveFanCurve(fanCurveLines_).value();
  addResetCmds(ctlCmds);
}

void AMD::OdFanCurve::postInit(ICommandQueue &ctlCmds)
{
  if (isZeroCurve(preInitControlPoints_))
    return;

  // Restore pre-init points.
  normalizeCurve(preInitControlPoints_, tempRange(), speedRange());
  for (auto const &point : preInitControlPoints_)
    ctlCmds.add({dataSource_->source(), controlPointCmd(point)});

  ctlCmds.add({dataSource_->source(), "c"});
}

void AMD::OdFanCurve::init()
{
  if (!dataSource_->read(fanCurveLines_))
    return;

  tempRange_ =
      Utils::AMD::parseOverdriveFanCurveTempRange(fanCurveLines_).value();
  speedRange_ =
      Utils::AMD::parseOverdriveFanCurveSpeedRange(fanCurveLines_).value();

  controlPoints_ = Utils::AMD::parseOverdriveFanCurve(fanCurveLines_).value();
  if (isZeroCurve(controlPoints_))
    setPointCoordinatesFrom(controlPoints_, defaultCurve());

  normalizeCurve(controlPoints_, tempRange_, speedRange_);
}

std::string const &AMD::OdFanCurve::ID() const
{
  return id_;
}

void AMD::OdFanCurve::importControl(IControl::Importer &i)
{
  auto &fanCurveImporter = dynamic_cast<AMD::OdFanCurve::Importer &>(i);
  fanCurve(fanCurveImporter.provideFanCurve());
}

void AMD::OdFanCurve::exportControl(IControl::Exporter &e) const
{
  auto &fanCurveExporter = dynamic_cast<AMD::OdFanCurve::Exporter &>(e);
  fanCurveExporter.takeFanCurveRange(tempRange(), speedRange());
  fanCurveExporter.takeFanCurve(fanCurve());
}

void AMD::OdFanCurve::cleanControl(ICommandQueue &ctlCmds)
{
  addResetCmds(ctlCmds);
  triggerManualOpMode_ = true;
}

void AMD::OdFanCurve::syncControl(ICommandQueue &ctlCmds)
{
  if (!dataSource_->read(fanCurveLines_))
    return;

  bool outOfSync = addSyncCmds(ctlCmds);

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
  return tempRange_;
}

AMD::OdFanCurve::SpeedRange const &AMD::OdFanCurve::speedRange() const
{
  return speedRange_;
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

void AMD::OdFanCurve::normalizeCurve(
    std::vector<AMD::OdFanCurve::ControlPoint> &curve,
    AMD::OdFanCurve::TempRange const &tempRange,
    AMD::OdFanCurve::SpeedRange const &speedRange) const
{
  auto normalizedPoints = toCurvePoints(curve);
  Utils::Common::normalizePoints(normalizedPoints, tempRange, speedRange);
  setPointCoordinatesFrom(curve, normalizedPoints);
}

bool AMD::OdFanCurve::addSyncCmds(ICommandQueue &ctlCmds) const
{
  bool commit = false;
  auto const curve = Utils::AMD::parseOverdriveFanCurve(fanCurveLines_).value();
  size_t curveIndex = 0;
  for (auto const &point : controlPoints()) {
    auto const [_, temp, speed] = curve[curveIndex++];
    if (temp != std::get<1>(point) || speed != std::get<2>(point)) {
      ctlCmds.add({dataSource_->source(), controlPointCmd(point)});
      commit = true;
    }
  }

  if (commit)
    ctlCmds.add({dataSource_->source(), "c"});

  return commit;
}

void AMD::OdFanCurve::addResetCmds(ICommandQueue &ctlCmds) const
{
  ctlCmds.add({dataSource_->source(), "r"});

  // NOTE Apparently, there is no need to submit the commit command after the
  // reset one on the new fan overdrive interfaces [1]. This interaction model
  // digress from the overdrive clock-voltage interface model, where the reset
  // command only restores the default values without actually committing them.
  //
  // Submit a commit command just in case this is changed to align with the
  // original overdrive interface interaction model.
  //
  // [1] https://gitlab.freedesktop.org/drm/amd/-/issues/2402#note_2211197
  ctlCmds.add({dataSource_->source(), "c"});
}
