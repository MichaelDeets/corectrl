// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Juan Palacios <jpalaciosdev@gmail.com>

#include "pmfreqoffsetqmlitem.h"

#include "core/qmlcomponentregistry.h"
#include "pmfreqoffset.h"
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QString>
#include <QtGlobal>
#include <QtQml>
#include <cmath>
#include <memory>

char const *const AMD::PMFreqOffsetQMLItem::trStrings[] = {
    QT_TRANSLATE_NOOP("AMD::PMFreqOffsetQMLItem", "SCLK"),
    QT_TRANSLATE_NOOP("AMD::PMFreqOffsetQMLItem", "MCLK"),
};

class AMD::PMFreqOffsetQMLItem::Initializer final
: public QMLItem::Initializer
, public AMD::PMFreqOffset::Exporter
{
 public:
  Initializer(IQMLComponentFactory const &qmlComponentFactory,
              QQmlApplicationEngine &qmlEngine,
              AMD::PMFreqOffsetQMLItem &qmlItem) noexcept
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
  void takePMFreqOffsetControlName(std::string const &name) override;
  void takePMFreqOffsetRange(units::frequency::megahertz_t min,
                             units::frequency::megahertz_t max) override;
  void takePMFreqOffsetValue(units::frequency::megahertz_t value) override;

 private:
  AMD::PMFreqOffsetQMLItem &outer_;
};

void AMD::PMFreqOffsetQMLItem::Initializer::takeActive(bool active)
{
  outer_.takeActive(active);
}

void AMD::PMFreqOffsetQMLItem::Initializer::takePMFreqOffsetControlName(
    std::string const &name)
{
  outer_.takePMFreqOffsetControlName(name);
}

void AMD::PMFreqOffsetQMLItem::Initializer::takePMFreqOffsetRange(
    units::frequency::megahertz_t min, units::frequency::megahertz_t max)
{
  outer_.offsetRange(min, max);
}

void AMD::PMFreqOffsetQMLItem::Initializer::takePMFreqOffsetValue(
    units::frequency::megahertz_t value)
{
  outer_.takePMFreqOffsetValue(value);
}

AMD::PMFreqOffsetQMLItem::PMFreqOffsetQMLItem() noexcept
{
  setName(tr(AMD::PMFreqOffset::ItemID.data()));
}

void AMD::PMFreqOffsetQMLItem::changeOffset(int value)
{
  if (offset_ != value) {
    offset_ = value;
    emit offsetChanged(offset_);
    emit settingsChanged();
  }
}

QString const &AMD::PMFreqOffsetQMLItem::instanceID() const
{
  return instanceID_;
}

void AMD::PMFreqOffsetQMLItem::activate(bool active)
{
  takeActive(active);
}

std::optional<std::reference_wrapper<Exportable::Exporter>>
AMD::PMFreqOffsetQMLItem::provideExporter(Item const &)
{
  return {};
}

std::optional<std::reference_wrapper<Importable::Importer>>
AMD::PMFreqOffsetQMLItem::provideImporter(Item const &)
{
  return {};
}

void AMD::PMFreqOffsetQMLItem::takeActive(bool active)
{
  active_ = active;
  setVisible(active);
}

void AMD::PMFreqOffsetQMLItem::takePMFreqOffsetControlName(std::string const &name)
{
  controlName(name);
}

void AMD::PMFreqOffsetQMLItem::takePMFreqOffsetValue(
    units::frequency::megahertz_t value)
{
  auto newValue = value.to<int>();
  if (offset_ != newValue) {
    offset_ = newValue;
    emit offsetChanged(offset_);
  }
}

bool AMD::PMFreqOffsetQMLItem::provideActive() const
{
  return active_;
}

units::frequency::megahertz_t
AMD::PMFreqOffsetQMLItem::providePMFreqOffsetValue() const
{
  return units::frequency::megahertz_t(offset_);
}

std::unique_ptr<Exportable::Exporter> AMD::PMFreqOffsetQMLItem::initializer(
    IQMLComponentFactory const &qmlComponentFactory,
    QQmlApplicationEngine &qmlEngine)
{
  return std::make_unique<AMD::PMFreqOffsetQMLItem::Initializer>(
      qmlComponentFactory, qmlEngine, *this);
}

void AMD::PMFreqOffsetQMLItem::controlName(std::string const &name)
{
  instanceID_ = QString::fromStdString(name);
  emit controlLabelChanged(tr(name.c_str()));
}

void AMD::PMFreqOffsetQMLItem::offsetRange(units::frequency::megahertz_t min,
                                           units::frequency::megahertz_t max)
{
  emit offsetRangeChanged(min.to<int>(), max.to<int>());
}

bool AMD::PMFreqOffsetQMLItem::register_()
{
  QMLComponentRegistry::addQMLTypeRegisterer([]() {
    qmlRegisterType<AMD::PMFreqOffsetQMLItem>("CoreCtrl.UIComponents", 1, 0,
                                              AMD::PMFreqOffset::ItemID.data());
  });

  QMLComponentRegistry::addQMLItemProvider(
      AMD::PMFreqOffset::ItemID, [](QQmlApplicationEngine &engine) {
        QQmlComponent component(
            &engine, QStringLiteral("qrc:/qml/AMDPMFreqOffsetForm.qml"));
        return qobject_cast<QMLItem *>(component.create());
      });

  return true;
}

bool const AMD::PMFreqOffsetQMLItem::registered_ =
    AMD::PMFreqOffsetQMLItem::register_();
