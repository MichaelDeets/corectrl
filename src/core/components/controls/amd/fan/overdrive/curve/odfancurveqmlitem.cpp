// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#include "odfancurveqmlitem.h"

#include "core/qmlcomponentregistry.h"
#include "odfancurve.h"
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QString>
#include <QtQml>
#include <cmath>
#include <cstddef>
#include <memory>
#include <utility>

char const *const AMD::OdFanCurveQMLItem::trStrings[] = {
    QT_TRANSLATE_NOOP("ControlModeQMLItem", "AMD_OD_FAN_CURVE"),
};

class AMD::OdFanCurveQMLItem::Initializer final
: public QMLItem::Initializer
, public AMD::OdFanCurve::Exporter
{
 public:
  Initializer(IQMLComponentFactory const &qmlComponentFactory,
              QQmlApplicationEngine &qmlEngine,
              AMD::OdFanCurveQMLItem &qmlItem) noexcept
  : QMLItem::Initializer(qmlComponentFactory, qmlEngine)
  , outer_(qmlItem)
  {
  }

  std::optional<std::reference_wrapper<Exportable::Exporter>>
  provideExporter(Item const &) override
  {
    return {};
  }

  void takeActive(bool active) override;
  void
  takeFanCurve(std::vector<AMD::OdFanCurve::CurvePoint> const &curve) override;
  void takeFanCurveRange(AMD::OdFanCurve::TempRange temp,
                         AMD::OdFanCurve::SpeedRange speed) override;
  void takeFanStop(bool enabled) override;
  void takeFanStopTemp(units::temperature::celsius_t value) override;
  void takeFanStopTempRange(AMD::OdFanCurve::TempRange value) override;

 private:
  AMD::OdFanCurveQMLItem &outer_;
};

void AMD::OdFanCurveQMLItem::Initializer::takeActive(bool active)
{
  outer_.takeActive(active);
}

void AMD::OdFanCurveQMLItem::Initializer::takeFanCurve(
    std::vector<AMD::OdFanCurve::CurvePoint> const &curve)
{
  outer_.takeFanCurve(curve);
}

void AMD::OdFanCurveQMLItem::Initializer::takeFanCurveRange(
    AMD::OdFanCurve::TempRange temp, AMD::OdFanCurve::SpeedRange speed)
{
  outer_.curveRange(temp, speed);
}

void AMD::OdFanCurveQMLItem::Initializer::takeFanStop(bool enabled)
{
  outer_.takeFanStop(enabled);
}

void AMD::OdFanCurveQMLItem::Initializer::takeFanStopTemp(
    units::temperature::celsius_t value)
{
  outer_.takeFanStopTemp(value);
}

void AMD::OdFanCurveQMLItem::Initializer::takeFanStopTempRange(
    AMD::OdFanCurve::TempRange value)
{
  outer_.stopTempRange(value);
}

AMD::OdFanCurveQMLItem::OdFanCurveQMLItem() noexcept
{
  setName(tr(AMD::OdFanCurve::ItemID.data()));
}

void AMD::OdFanCurveQMLItem::updateCurvePoint(QPointF const &oldPoint,
                                              QPointF const &newPoint)
{
  if (oldPoint != newPoint) {
    auto oPoint = std::make_pair(
        units::temperature::celsius_t(std::round(oldPoint.x())),
        units::concentration::percent_t(std::round(oldPoint.y())));
    auto nPoint = std::make_pair(
        units::temperature::celsius_t(std::round(newPoint.x())),
        units::concentration::percent_t(std::round(newPoint.y())));

    for (size_t i = 0; i < curve_.size(); ++i) {
      if (curve_[i] == oPoint) {
        curve_[i] = nPoint;
        qCurve_.replace(static_cast<int>(i), newPoint);

        emit curveChanged(qCurve_);
        emit settingsChanged();
        break;
      }
    }
  }
}

void AMD::OdFanCurveQMLItem::changeStop(bool enabled)
{
  if (stop_ != enabled) {
    stop_ = enabled;

    emit stopChanged(enabled);
    emit settingsChanged();
  }
}

void AMD::OdFanCurveQMLItem::changeStopTemp(int value)
{
  if (stopTemp_ != value) {
    stopTemp_ = value;

    emit stopTempChanged(value);
    emit settingsChanged();
  }
}

void AMD::OdFanCurveQMLItem::activate(bool active)
{
  takeActive(active);
}

QVariantList const &AMD::OdFanCurveQMLItem::curve() const
{
  return qCurve_;
}

qreal AMD::OdFanCurveQMLItem::minTemp() const
{
  return minTemp_;
}

qreal AMD::OdFanCurveQMLItem::maxTemp() const
{
  return maxTemp_;
}

qreal AMD::OdFanCurveQMLItem::minSpeed() const
{
  return minSpeed_;
}

qreal AMD::OdFanCurveQMLItem::maxSpeed() const
{
  return maxSpeed_;
}

bool AMD::OdFanCurveQMLItem::stop() const
{
  return stop_;
}

int AMD::OdFanCurveQMLItem::stopTemp() const
{
  return stopTemp_;
}

std::optional<std::reference_wrapper<Importable::Importer>>
AMD::OdFanCurveQMLItem::provideImporter(Item const &)
{
  return {};
}

std::optional<std::reference_wrapper<Exportable::Exporter>>
AMD::OdFanCurveQMLItem::provideExporter(Item const &)
{
  return {};
}

bool AMD::OdFanCurveQMLItem::provideActive() const
{
  return active_;
}

std::vector<AMD::OdFanCurve::CurvePoint> const &
AMD::OdFanCurveQMLItem::provideFanCurve() const
{
  return curve_;
}

bool AMD::OdFanCurveQMLItem::provideFanStop() const
{
  return stop_;
}

units::temperature::celsius_t AMD::OdFanCurveQMLItem::provideFanStopTemp() const
{
  return units::temperature::celsius_t(stopTemp_);
}

void AMD::OdFanCurveQMLItem::takeActive(bool active)
{
  active_ = active;
  setVisible(active);
}

void AMD::OdFanCurveQMLItem::takeFanCurve(
    std::vector<AMD::OdFanCurve::CurvePoint> const &curve)
{
  if (curve_ != curve) {
    curve_ = curve;

    qCurve_.clear();
    for (auto const &[temp, speed] : curve_)
      qCurve_.push_back(QPointF(temp.to<qreal>(), speed.to<qreal>() * 100));

    emit curveChanged(qCurve_);
  }
}

void AMD::OdFanCurveQMLItem::takeFanStop(bool enabled)
{
  if (stop_ != enabled) {
    stop_ = enabled;

    emit stopChanged(stop_);
  }
}

void AMD::OdFanCurveQMLItem::takeFanStopTemp(units::temperature::celsius_t value)
{
  auto newValue = value.to<int>();

  if (stopTemp_ != newValue) {
    stopTemp_ = newValue;

    emit stopTempChanged(stopTemp_);
  }
}

std::unique_ptr<Exportable::Exporter> AMD::OdFanCurveQMLItem::initializer(
    IQMLComponentFactory const &qmlComponentFactory,
    QQmlApplicationEngine &qmlEngine)
{
  return std::make_unique<AMD::OdFanCurveQMLItem::Initializer>(
      qmlComponentFactory, qmlEngine, *this);
}

void AMD::OdFanCurveQMLItem::curveRange(AMD::OdFanCurve::TempRange temp,
                                        AMD::OdFanCurve::SpeedRange speed)
{
  minTemp_ = temp.first.to<qreal>();
  maxTemp_ = temp.second.to<qreal>();
  minSpeed_ = speed.first.to<qreal>() * 100;
  maxSpeed_ = speed.second.to<qreal>() * 100;

  emit curveRangeChanged(minTemp_, maxTemp_, minSpeed_, maxSpeed_);
}

void AMD::OdFanCurveQMLItem::stopTempRange(AMD::OdFanCurve::TempRange)
{
  emit stopAvailable();
}

bool AMD::OdFanCurveQMLItem::register_()
{
  QMLComponentRegistry::addQMLTypeRegisterer([]() {
    qmlRegisterType<AMD::OdFanCurveQMLItem>("CoreCtrl.UIComponents", 1, 0,
                                            AMD::OdFanCurve::ItemID.data());
  });

  QMLComponentRegistry::addQMLItemProvider(
      AMD::OdFanCurve::ItemID, [](QQmlApplicationEngine &engine) {
        QQmlComponent component(
            &engine, QStringLiteral("qrc:/qml/AMDOdFanCurveForm.qml"));
        return qobject_cast<QMLItem *>(component.create());
      });

  return true;
}

bool const AMD::OdFanCurveQMLItem::registered_ =
    AMD::OdFanCurveQMLItem::register_();
