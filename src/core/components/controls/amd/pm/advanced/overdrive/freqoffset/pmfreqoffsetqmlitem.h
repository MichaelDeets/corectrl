// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/qmlitem.h"
#include "pmfreqoffsetprofilepart.h"
#include <QObject>
#include <QString>
#include <QVariantList>
#include <string>

namespace AMD {

class PMFreqOffsetQMLItem
: public QMLItem
, public AMD::PMFreqOffsetProfilePart::Importer
, public AMD::PMFreqOffsetProfilePart::Exporter
{
  Q_OBJECT
  Q_PROPERTY(QString instanceID READ instanceID)

 public:
  explicit PMFreqOffsetQMLItem() noexcept;

 signals:
  void controlLabelChanged(QString const &label);
  void offsetRangeChanged(int min, int max);
  void offsetChanged(int value);

 public slots:
  void changeOffset(int value);

 public:
  QString const &instanceID() const;

  void activate(bool active) override;

  std::optional<std::reference_wrapper<Exportable::Exporter>>
  provideExporter(Item const &i) override;
  std::optional<std::reference_wrapper<Importable::Importer>>
  provideImporter(Item const &i) override;

  void takeActive(bool active) override;
  void takePMFreqOffsetControlName(std::string const &mode) override;
  void takePMFreqOffsetValue(units::frequency::megahertz_t value) override;

  bool provideActive() const override;
  units::frequency::megahertz_t providePMFreqOffsetValue() const override;

  std::unique_ptr<Exportable::Exporter>
  initializer(IQMLComponentFactory const &qmlComponentFactory,
              QQmlApplicationEngine &qmlEngine) override;

 private:
  class Initializer;

  void controlName(std::string const &name);
  void offsetRange(units::frequency::megahertz_t min,
                   units::frequency::megahertz_t max);

  QString instanceID_;
  bool active_;
  int offset_;

  static bool register_();
  static bool const registered_;

  static char const *const trStrings[];
};

} // namespace AMD
