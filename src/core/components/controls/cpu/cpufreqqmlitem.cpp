// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "cpufreqqmlitem.h"

#include "core/qmlcomponentregistry.h"
#include "cpufreq.h"
#include <QQmlApplicationEngine>
#include <QtGlobal>
#include <QtQml>
#include <memory>

char const *const CPUFreqQMLItem::trStrings[] = {
    QT_TRANSLATE_NOOP("ControlModeQMLItem", "CPU_CPUFREQ"),

    // XXX add cpufreq scaling governors here
    QT_TRANSLATE_NOOP("CPUFreqQMLItem", "performance"),
    QT_TRANSLATE_NOOP("CPUFreqQMLItem", "powersave"),
    QT_TRANSLATE_NOOP("CPUFreqQMLItem", "schedutil"),
    QT_TRANSLATE_NOOP("CPUFreqQMLItem", "ondemand"),
    QT_TRANSLATE_NOOP("CPUFreqQMLItem", "conservative"),

    // XXX add CPU EPP available hints here
    QT_TRANSLATE_NOOP("CPUFreqQMLItem", "default"),
    QT_TRANSLATE_NOOP("CPUFreqQMLItem", "performance"),
    QT_TRANSLATE_NOOP("CPUFreqQMLItem", "balance_performance"),
    QT_TRANSLATE_NOOP("CPUFreqQMLItem", "balance_power"),
    QT_TRANSLATE_NOOP("CPUFreqQMLItem", "power"),
};

class CPUFreqQMLItem::Initializer final
: public QMLItem::Initializer
, public CPUFreq::Exporter
{
 public:
  Initializer(IQMLComponentFactory const &qmlComponentFactory,
              QQmlApplicationEngine &qmlEngine, CPUFreqQMLItem &qmlItem) noexcept
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

  void takeCPUFreqScalingGovernor(std::string const &governor) override;
  void
  takeCPUFreqScalingGovernors(std::vector<std::string> const &governors) override;

  void takeCPUFreqEPPHint(std::optional<std::string> const &hint) override;
  void takeCPUFreqEPPHints(
      std::optional<std::vector<std::string>> const &hints) override;

 private:
  CPUFreqQMLItem &outer_;
};

void CPUFreqQMLItem::Initializer::takeActive(bool active)
{
  outer_.takeActive(active);
}

void CPUFreqQMLItem::Initializer::takeCPUFreqScalingGovernor(
    std::string const &governor)
{
  outer_.takeCPUFreqScalingGovernor(governor);
}

void CPUFreqQMLItem::Initializer::takeCPUFreqScalingGovernors(
    std::vector<std::string> const &governors)
{
  outer_.takeCPUFreqScalingGovernors(governors);
}

void CPUFreqQMLItem::Initializer::takeCPUFreqEPPHint(
    std::optional<std::string> const &hint)
{
  outer_.takeCPUFreqEPPHint(hint);
}

void CPUFreqQMLItem::Initializer::takeCPUFreqEPPHints(
    std::optional<std::vector<std::string>> const &hints)
{
  outer_.takeCPUFreqEPPHints(hints);
}

CPUFreqQMLItem::CPUFreqQMLItem() noexcept
{
  setName(tr(CPUFreq::ItemID.data()));
}

void CPUFreqQMLItem::changeScalingGovernor(QString const &governor)
{
  auto newScalingGovernor = governor.toStdString();
  if (scalingGovernor_ != newScalingGovernor) {
    std::swap(scalingGovernor_, newScalingGovernor);
    emit scalingGovernorChanged(governor);
    emit toggleEppHint(enableEpp_ && scalingGovernor_ == eppScalingGovernor_);
    emit settingsChanged();
  }
}

void CPUFreqQMLItem::changeEPPHint(QString const &hint)
{
  auto newHint = hint.toStdString();
  if (eppHint_ && eppHint_ != newHint) {
    std::swap(*eppHint_, newHint);
    emit eppHintChanged(hint);
    emit settingsChanged();
  }
}

void CPUFreqQMLItem::activate(bool active)
{
  takeActive(active);
}

std::optional<std::reference_wrapper<Importable::Importer>>
CPUFreqQMLItem::provideImporter(Item const &)
{
  return {};
}

std::optional<std::reference_wrapper<Exportable::Exporter>>
CPUFreqQMLItem::provideExporter(Item const &)
{
  return {};
}

bool CPUFreqQMLItem::provideActive() const
{
  return active_;
}

std::string const &CPUFreqQMLItem::provideCPUFreqScalingGovernor() const
{
  return scalingGovernor_;
}

std::optional<std::string> const &CPUFreqQMLItem::provideCPUFreqEPPHint() const
{
  return eppHint_;
}

void CPUFreqQMLItem::takeActive(bool active)
{
  active_ = active;
  setVisible(active);
}

void CPUFreqQMLItem::takeCPUFreqScalingGovernor(std::string const &governor)
{
  if (scalingGovernor_ != governor) {
    scalingGovernor_ = governor;
    emit scalingGovernorChanged(QString::fromStdString(scalingGovernor_));
    emit toggleEppHint(enableEpp_ && scalingGovernor_ == eppScalingGovernor_);
  }
}

void CPUFreqQMLItem::takeCPUFreqEPPHint(std::optional<std::string> const &hint)
{
  if (hint && eppHint_ != hint) {
    eppHint_ = hint;
    emit eppHintChanged(QString::fromStdString(*eppHint_));
  }
}

std::unique_ptr<Exportable::Exporter>
CPUFreqQMLItem::initializer(IQMLComponentFactory const &qmlComponentFactory,
                            QQmlApplicationEngine &qmlEngine)
{
  return std::make_unique<CPUFreqQMLItem::Initializer>(qmlComponentFactory,
                                                       qmlEngine, *this);
}

void CPUFreqQMLItem::takeCPUFreqScalingGovernors(
    std::vector<std::string> const &governors)
{
  QList<QString> governorTextVector;
  for (auto governor : governors) {
    governorTextVector.push_back(QString::fromStdString(governor));
    governorTextVector.push_back(tr(governor.data()));
  }
  emit scalingGovernorsChanged(governorTextVector);
}

void CPUFreqQMLItem::takeCPUFreqEPPHints(
    std::optional<std::vector<std::string>> const &hints)
{
  if (!hints)
    return;

  enableEpp_ = true;

  QList<QString> hintTextVector;
  for (auto hint : *hints) {
    hintTextVector.push_back(QString::fromStdString(hint));
    hintTextVector.push_back(tr(hint.data()));
  }
  emit eppHintsChanged(hintTextVector);
}

bool CPUFreqQMLItem::register_()
{
  QMLComponentRegistry::addQMLTypeRegisterer([]() {
    qmlRegisterType<CPUFreqQMLItem>("CoreCtrl.UIComponents", 1, 0,
                                    CPUFreq::ItemID.data());
  });

  QMLComponentRegistry::addQMLItemProvider(
      CPUFreq::ItemID, [](QQmlApplicationEngine &engine) {
        QQmlComponent component(&engine,
                                QStringLiteral("qrc:/qml/CPUFreqForm.qml"));
        return qobject_cast<QMLItem *>(component.create());
      });

  return true;
}

bool const CPUFreqQMLItem::registered_ = CPUFreqQMLItem::register_();
