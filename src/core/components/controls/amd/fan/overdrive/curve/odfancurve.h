// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/components/controls/control.h"
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <units.h>
#include <utility>
#include <vector>

template<typename...>
class IDataSource;

namespace AMD {

class OdFanCurve : public Control
{
 public:
  static constexpr std::string_view ItemID{"AMD_OD_FAN_CURVE"};

  using CurvePoint =
      std::pair<units::temperature::celsius_t, units::concentration::percent_t>;
  using TempRange =
      std::pair<units::temperature::celsius_t, units::temperature::celsius_t>;
  using SpeedRange =
      std::pair<units::concentration::percent_t, units::concentration::percent_t>;

  class Importer : public IControl::Importer
  {
   public:
    virtual std::vector<AMD::OdFanCurve::CurvePoint> const &
    provideFanCurve() const = 0;
  };

  class Exporter : public IControl::Exporter
  {
   public:
    virtual void
    takeFanCurve(std::vector<AMD::OdFanCurve::CurvePoint> const &curve) = 0;
    virtual void takeFanCurveRange(AMD::OdFanCurve::TempRange temp,
                                   AMD::OdFanCurve::SpeedRange speed) = 0;
  };

  OdFanCurve(std::unique_ptr<IDataSource<std::vector<std::string>>>
                 &&dataSource) noexcept;

  void preInit(ICommandQueue &ctlCmds) final override;
  void postInit(ICommandQueue &ctlCmds) final override;
  void init() final override;

  std::string const &ID() const final override;

 protected:
  void importControl(IControl::Importer &i) final override;
  void exportControl(IControl::Exporter &e) const final override;

  void cleanControl(ICommandQueue &ctlCmds) final override;
  void syncControl(ICommandQueue &ctlCmds) final override;

  std::vector<CurvePoint> defaultCurve() const;
  std::vector<CurvePoint> fanCurve() const;
  void fanCurve(std::vector<CurvePoint> points);

  using ControlPoint = std::tuple<unsigned int, units::temperature::celsius_t,
                                  units::concentration::percent_t>;

  std::vector<ControlPoint> const &controlPoints() const;
  TempRange const &tempRange() const;
  SpeedRange const &speedRange() const;

  std::vector<CurvePoint>
  toCurvePoints(std::vector<ControlPoint> const &curve) const;
  void setPointCoordinatesFrom(std::vector<ControlPoint> &curve,
                               std::vector<CurvePoint> const &values) const;
  bool isZeroCurve(std::vector<ControlPoint> const &curve) const;

  std::string controlPointCmd(ControlPoint const &point) const;

 private:
  void normalizeCurve(std::vector<ControlPoint> &curve,
                      TempRange const &tempRange,
                      SpeedRange const &speedRange) const;
  bool addSyncCmds(ICommandQueue &ctlCmds) const;
  void addResetCmds(ICommandQueue &ctlCmds) const;

  std::string const id_;

  std::unique_ptr<IDataSource<std::vector<std::string>>> const dataSource_;

  TempRange tempRange_;
  SpeedRange speedRange_;

  std::vector<ControlPoint> preInitControlPoints_;
  std::vector<ControlPoint> controlPoints_;
  std::vector<std::string> fanCurveLines_;
  bool triggerManualOpMode_;
};

} // namespace AMD
