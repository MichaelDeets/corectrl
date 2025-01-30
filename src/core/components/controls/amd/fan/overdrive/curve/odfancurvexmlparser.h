// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/profilepartxmlparser.h"
#include "odfancurveprofilepart.h"
#include <optional>
#include <string_view>
#include <vector>

namespace AMD {

class OdFanCurveXMLParser final
: public ProfilePartXMLParser
, public AMD::OdFanCurveProfilePart::Exporter
, public AMD::OdFanCurveProfilePart::Importer
{
 public:
  OdFanCurveXMLParser() noexcept;

  std::unique_ptr<Exportable::Exporter> factory(
      IProfilePartXMLParserProvider const &profilePartParserProvider) override;
  std::unique_ptr<Exportable::Exporter> initializer() override;

  std::optional<std::reference_wrapper<Exportable::Exporter>>
  provideExporter(Item const &i) override;
  std::optional<std::reference_wrapper<Importable::Importer>>
  provideImporter(Item const &i) override;

  void takeActive(bool active) override;
  bool provideActive() const override;

  void takeFanCurve(std::vector<OdFanCurve::CurvePoint> const &curve) override;
  std::vector<OdFanCurve::CurvePoint> const &provideFanCurve() const override;

  void takeFanStop(bool enabled) override;
  bool provideFanStop() const override;

  void takeFanStopTemp(units::temperature::celsius_t value) override;
  units::temperature::celsius_t provideFanStopTemp() const override;

  void appendTo(pugi::xml_node &parentNode) override;

 protected:
  void resetAttributes() override;
  void loadPartFrom(pugi::xml_node const &parentNode) override;

 private:
  static constexpr std::string_view CurveNodeName{"CURVE"};
  static constexpr std::string_view PointNodeName{"POINT"};

  class Initializer;

  bool active_;
  bool activeDefault_;

  std::vector<OdFanCurve::CurvePoint> curve_;
  std::vector<OdFanCurve::CurvePoint> curveDefault_;

  std::optional<bool> stop_{std::nullopt};
  std::optional<bool> stopDefault_{std::nullopt};
  std::optional<units::temperature::celsius_t> stopTemp_{std::nullopt};
  std::optional<units::temperature::celsius_t> stopTempDefault_{std::nullopt};

  static bool const registered_;
};

} // namespace AMD
