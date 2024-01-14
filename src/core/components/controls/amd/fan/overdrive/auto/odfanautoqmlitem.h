// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/qmlitem.h"
#include "odfanautoprofilepart.h"
#include <QObject>

namespace AMD {

class OdFanAutoQMLItem
: public QMLItem
, public AMD::OdFanAutoProfilePart::Importer
, public AMD::OdFanAutoProfilePart::Exporter
{
  Q_OBJECT

 public:
  explicit OdFanAutoQMLItem() noexcept;

  void activate(bool active) override;

  std::optional<std::reference_wrapper<Importable::Importer>>
  provideImporter(Item const &i) override;
  std::optional<std::reference_wrapper<Exportable::Exporter>>
  provideExporter(Item const &i) override;

  bool provideActive() const override;

  void takeActive(bool active) override;

  std::unique_ptr<Exportable::Exporter>
  initializer(IQMLComponentFactory const &qmlComponentFactory,
              QQmlApplicationEngine &qmlEngine) override;

 private:
  class Initializer;

  bool active_;

  static bool register_();
  static bool const registered_;

  static char const *const trStrings[];
};

} // namespace AMD
