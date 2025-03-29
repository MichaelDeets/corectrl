// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/components/controls/control.h"
#include "core/idatasource.h"
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <units.h>
#include <utility>
#include <vector>

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
    virtual bool provideFanStop() const = 0;
    virtual units::temperature::celsius_t provideFanStopTemp() const = 0;
  };

  class Exporter : public IControl::Exporter
  {
   public:
    virtual void
    takeFanCurve(std::vector<AMD::OdFanCurve::CurvePoint> const &curve) = 0;
    virtual void takeFanCurveRange(AMD::OdFanCurve::TempRange temp,
                                   AMD::OdFanCurve::SpeedRange speed) = 0;
    virtual void takeFanStop(bool enabled) = 0;
    virtual void takeFanStopTemp(units::temperature::celsius_t value) = 0;
    virtual void takeFanStopTempRange(AMD::OdFanCurve::TempRange value) = 0;
  };

  struct CurveDataSource
  {
    std::unique_ptr<IDataSource<std::vector<std::string>>> curve;
    TempRange temperatureRange;
    SpeedRange speedRange;
  };

  struct StopTemperatureDataSource
  {
    std::unique_ptr<IDataSource<std::vector<std::string>>> temperature;
    TempRange range;
  };

  struct StopDataSource
  {
    std::unique_ptr<IDataSource<std::vector<std::string>>> enable;
    std::optional<StopTemperatureDataSource> stopTemp;
  };

  OdFanCurve(CurveDataSource &&curveDataSource,
             std::optional<StopDataSource> &&stopDataSource = std::nullopt) noexcept;

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

  bool stop() const;
  void stop(bool value);

  units::temperature::celsius_t stopTemp() const;
  void stopTemp(units::temperature::celsius_t value);
  TempRange const &stopTempRange() const;

 private:
  void normalizeCurve(std::vector<ControlPoint> &curve,
                      TempRange const &tempRange,
                      SpeedRange const &speedRange) const;
  bool addCurveSyncCmds(ICommandQueue &ctlCmds,
                        std::vector<ControlPoint> &&curve) const;
  bool addStopSyncCmds(ICommandQueue &ctlCmds, bool hwStop,
                       std::optional<units::temperature::celsius_t> hwTemp) const;
  void addResetCmds(ICommandQueue &ctlCmds) const;

  std::string const id_;

  CurveDataSource const curveDataSource_;
  std::optional<StopDataSource> const stopDataSource_;

  std::vector<std::string> dataSourceLines_;

  std::vector<ControlPoint> preInitControlPoints_;
  std::vector<ControlPoint> controlPoints_;

  bool preInitStop_;
  bool stop_;

  units::temperature::celsius_t preInitStopTemp_;
  units::temperature::celsius_t stopTemp_;

  bool triggerManualOpMode_;
};

} // namespace AMD
