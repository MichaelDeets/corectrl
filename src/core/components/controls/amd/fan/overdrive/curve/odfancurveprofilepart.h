// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/profilepart.h"
#include "odfancurve.h"
#include <string>
#include <utility>
#include <vector>

namespace AMD {

class OdFanCurveProfilePart final
: public ProfilePart
, public AMD::OdFanCurve::Importer
{
 public:
  class Importer : public IProfilePart::Importer
  {
   public:
    virtual std::vector<AMD::OdFanCurve::CurvePoint> const &
    provideFanCurve() const = 0;
  };

  class Exporter : public IProfilePart::Exporter
  {
   public:
    virtual void
    takeFanCurve(std::vector<AMD::OdFanCurve::CurvePoint> const &curve) = 0;
  };

  OdFanCurveProfilePart() noexcept;

  std::unique_ptr<Exportable::Exporter>
  factory(IProfilePartProvider const &profilePartProvider) override;
  std::unique_ptr<Exportable::Exporter> initializer() override;

  std::string const &ID() const override;

  std::optional<std::reference_wrapper<Importable::Importer>>
  provideImporter(Item const &i) override;

  bool provideActive() const override;
  std::vector<OdFanCurve::CurvePoint> const &provideFanCurve() const override;

 protected:
  void importProfilePart(IProfilePart::Importer &i) override;
  void exportProfilePart(IProfilePart::Exporter &e) const override;
  std::unique_ptr<IProfilePart> cloneProfilePart() const override;

 private:
  void curve(std::vector<OdFanCurve::CurvePoint> const &points);
  class Initializer;

  std::string const id_;
  std::vector<OdFanCurve::CurvePoint> curve_;
  OdFanCurve::TempRange tempRange_;
  OdFanCurve::SpeedRange speedRange_;

  static bool const registered_;
};

} // namespace AMD
