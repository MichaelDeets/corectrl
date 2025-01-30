// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/qmlitem.h"
#include "odfancurveprofilepart.h"
#include <QObject>
#include <QPointF>
#include <QVariantList>
#include <QtGlobal>
#include <vector>

namespace AMD {

class OdFanCurveQMLItem
: public QMLItem
, public AMD::OdFanCurveProfilePart::Importer
, public AMD::OdFanCurveProfilePart::Exporter
{
  Q_OBJECT
  Q_PROPERTY(qreal minTemp READ minTemp)
  Q_PROPERTY(qreal maxTemp READ maxTemp)
  Q_PROPERTY(qreal minSpeed READ minSpeed)
  Q_PROPERTY(qreal maxSpeed READ maxSpeed)
  Q_PROPERTY(bool stop READ stop WRITE changeStop NOTIFY stopChanged)
  Q_PROPERTY(int stopTemp READ stopTemp WRITE changeStopTemp NOTIFY stopTempChanged)

 public:
  explicit OdFanCurveQMLItem() noexcept;

 signals:
  void curveChanged(QVariantList const &curve);
  void curveRangeChanged(qreal tempMin, qreal tempMax, qreal speedMin,
                         qreal speedMax);
  void stopAvailable();
  void stopChanged(bool enabled);
  void stopTempChanged(int value);

 public slots:
  void updateCurvePoint(QPointF const &oldPoint, QPointF const &newPoint);
  void changeStop(bool enabled);
  void changeStopTemp(int value);

 public:
  void activate(bool active) override;
  QVariantList const &curve() const;
  qreal minTemp() const;
  qreal maxTemp() const;
  qreal minSpeed() const;
  qreal maxSpeed() const;
  bool stop() const;
  int stopTemp() const;

  std::optional<std::reference_wrapper<Importable::Importer>>
  provideImporter(Item const &i) override;
  std::optional<std::reference_wrapper<Exportable::Exporter>>
  provideExporter(Item const &i) override;

  bool provideActive() const override;
  std::vector<OdFanCurve::CurvePoint> const &provideFanCurve() const override;
  bool provideFanStop() const override;
  units::temperature::celsius_t provideFanStopTemp() const override;

  void takeActive(bool active) override;
  void takeFanCurve(std::vector<OdFanCurve::CurvePoint> const &curve) override;
  void takeFanStop(bool enabled) override;
  void takeFanStopTemp(units::temperature::celsius_t value) override;

  std::unique_ptr<Exportable::Exporter>
  initializer(IQMLComponentFactory const &qmlComponentFactory,
              QQmlApplicationEngine &qmlEngine) override;

 private:
  void curveRange(AMD::OdFanCurve::TempRange temp,
                  AMD::OdFanCurve::SpeedRange speed);
  void stopTempRange(AMD::OdFanCurve::TempRange value);

  class Initializer;

  bool active_;

  std::vector<OdFanCurve::CurvePoint> curve_;
  QVariantList qCurve_;

  qreal minTemp_;
  qreal maxTemp_;
  qreal minSpeed_;
  qreal maxSpeed_;

  bool stop_;
  int stopTemp_;

  static bool register_();
  static bool const registered_;

  static char const *const trStrings[];
};

} // namespace AMD
