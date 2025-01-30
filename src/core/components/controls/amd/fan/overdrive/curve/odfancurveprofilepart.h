// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/profilepart.h"
#include "odfancurve.h"
#include <optional>
#include <string>
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
    virtual bool provideFanStop() const = 0;
    virtual units::temperature::celsius_t provideFanStopTemp() const = 0;
  };

  class Exporter : public IProfilePart::Exporter
  {
   public:
    virtual void
    takeFanCurve(std::vector<AMD::OdFanCurve::CurvePoint> const &curve) = 0;
    virtual void takeFanStop(bool enabled) = 0;
    virtual void takeFanStopTemp(units::temperature::celsius_t value) = 0;
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
  bool provideFanStop() const override;
  units::temperature::celsius_t provideFanStopTemp() const override;

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
  std::optional<bool> stop_{std::nullopt};
  std::optional<units::temperature::celsius_t> stopTemp_{std::nullopt};
  std::optional<OdFanCurve::TempRange> stopTempRange_{std::nullopt};

  static bool const registered_;
};

} // namespace AMD
