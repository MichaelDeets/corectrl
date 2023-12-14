// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#include "odfanautoqmlitem.h"

#include "core/qmlcomponentregistry.h"
#include "odfanauto.h"
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QString>
#include <QtGlobal>
#include <QtQml>
#include <memory>

char const *const AMD::OdFanAutoQMLItem::trStrings[] = {
    QT_TRANSLATE_NOOP("ControlModeQMLItem", "AMD_OD_FAN_AUTO"),
};

class AMD::OdFanAutoQMLItem::Initializer final
: public QMLItem::Initializer
, public IControl::Exporter
{
 public:
  Initializer(IQMLComponentFactory const &qmlComponentFactory,
              QQmlApplicationEngine &qmlEngine,
              AMD::OdFanAutoQMLItem &qmlItem) noexcept
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

 private:
  AMD::OdFanAutoQMLItem &outer_;
};

void AMD::OdFanAutoQMLItem::Initializer::takeActive(bool active)
{
  outer_.takeActive(active);
}

AMD::OdFanAutoQMLItem::OdFanAutoQMLItem() noexcept
{
  setName(tr(OdFanAuto::ItemID.data()));
}

void AMD::OdFanAutoQMLItem::activate(bool active)
{
  takeActive(active);
}

std::optional<std::reference_wrapper<Importable::Importer>>
AMD::OdFanAutoQMLItem::provideImporter(Item const &)
{
  return {};
}

std::optional<std::reference_wrapper<Exportable::Exporter>>
AMD::OdFanAutoQMLItem::provideExporter(Item const &)
{
  return {};
}

bool AMD::OdFanAutoQMLItem::provideActive() const
{
  return active_;
}

void AMD::OdFanAutoQMLItem::takeActive(bool active)
{
  active_ = active;
  setVisible(active);
}

std::unique_ptr<Exportable::Exporter> AMD::OdFanAutoQMLItem::initializer(
    IQMLComponentFactory const &qmlComponentFactory,
    QQmlApplicationEngine &qmlEngine)
{
  return std::make_unique<AMD::OdFanAutoQMLItem::Initializer>(
      qmlComponentFactory, qmlEngine, *this);
}

bool AMD::OdFanAutoQMLItem::register_()
{
  QMLComponentRegistry::addQMLTypeRegisterer([]() {
    qmlRegisterType<AMD::OdFanAutoQMLItem>("CoreCtrl.UIComponents", 1, 0,
                                           AMD::OdFanAuto::ItemID.data());
  });

  QMLComponentRegistry::addQMLItemProvider(
      AMD::OdFanAuto::ItemID, [](QQmlApplicationEngine &engine) {
        QQmlComponent component(
            &engine, QStringLiteral("qrc:/qml/AMDOdFanAutoForm.qml"));
        return qobject_cast<QMLItem *>(component.create());
      });

  return true;
}

bool const AMD::OdFanAutoQMLItem::registered_ =
    AMD::OdFanAutoQMLItem::register_();
