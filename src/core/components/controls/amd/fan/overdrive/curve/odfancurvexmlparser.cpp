// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#include "odfancurvexmlparser.h"

#include "core/profilepartxmlparserprovider.h"
#include "odfancurve.h"
#include <cmath>
#include <memory>

class AMD::OdFanCurveXMLParser::Initializer final
: public AMD::OdFanCurveProfilePart::Exporter
{
 public:
  Initializer(AMD::OdFanCurveXMLParser &outer) noexcept
  : outer_(outer)
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

 private:
  AMD::OdFanCurveXMLParser &outer_;
};

void AMD::OdFanCurveXMLParser::Initializer::takeActive(bool active)
{
  outer_.active_ = outer_.activeDefault_ = active;
}

void AMD::OdFanCurveXMLParser::Initializer::takeFanCurve(
    std::vector<AMD::OdFanCurve::CurvePoint> const &curve)
{
  outer_.curve_ = outer_.curveDefault_ = curve;
}

AMD::OdFanCurveXMLParser::OdFanCurveXMLParser() noexcept
: ProfilePartXMLParser(AMD::OdFanCurve::ItemID, *this, *this)
{
}

std::unique_ptr<Exportable::Exporter>
AMD::OdFanCurveXMLParser::factory(IProfilePartXMLParserProvider const &)
{
  return nullptr;
}

std::unique_ptr<Exportable::Exporter> AMD::OdFanCurveXMLParser::initializer()
{
  return std::make_unique<AMD::OdFanCurveXMLParser::Initializer>(*this);
}

std::optional<std::reference_wrapper<Exportable::Exporter>>
AMD::OdFanCurveXMLParser::provideExporter(Item const &)
{
  return {};
}

std::optional<std::reference_wrapper<Importable::Importer>>
AMD::OdFanCurveXMLParser::provideImporter(Item const &)
{
  return {};
}

void AMD::OdFanCurveXMLParser::takeActive(bool active)
{
  active_ = active;
}

bool AMD::OdFanCurveXMLParser::provideActive() const
{
  return active_;
}
void AMD::OdFanCurveXMLParser::takeFanCurve(
    std::vector<AMD::OdFanCurve::CurvePoint> const &curve)
{
  curve_ = curve;
}

std::vector<AMD::OdFanCurve::CurvePoint> const &
AMD::OdFanCurveXMLParser::provideFanCurve() const
{
  return curve_;
}

void AMD::OdFanCurveXMLParser::appendTo(pugi::xml_node &parentNode)
{
  auto pmFixedNode = parentNode.append_child(ID().c_str());
  pmFixedNode.append_attribute("active") = active_;
  auto curveNode = pmFixedNode.append_child(CurveNodeName.data());
  for (auto const &[temp, speed] : curve_) {
    auto pointNode = curveNode.append_child(PointNodeName.data());
    pointNode.append_attribute("temp") = temp.to<int>();
    pointNode.append_attribute("speed") = std::lround(speed.to<double>() * 100);
  }
}

void AMD::OdFanCurveXMLParser::resetAttributes()
{
  active_ = activeDefault_;
  curve_ = curveDefault_;
}

void AMD::OdFanCurveXMLParser::loadPartFrom(pugi::xml_node const &parentNode)
{
  auto pmFixedNode = parentNode.find_child(
      [&](pugi::xml_node const &node) { return node.name() == ID(); });

  active_ = pmFixedNode.attribute("active").as_bool(activeDefault_);

  auto curveNode = pmFixedNode.find_child(
      [&](pugi::xml_node const &node) { return node.name() == CurveNodeName; });

  if (!curveNode) {
    curve_ = curveDefault_;
  }
  else {
    curve_.clear();
    for (auto pointNode : curveNode.children(PointNodeName.data())) {
      auto tempAttr = pointNode.attribute("temp");
      auto speedAttr = pointNode.attribute("speed");
      if (tempAttr && speedAttr) {
        curve_.emplace_back(units::temperature::celsius_t(tempAttr.as_int()),
                            units::concentration::percent_t(speedAttr.as_uint()));
      }
      else { // malformed point data -> restore defaults
        curve_ = curveDefault_;
        break;
      }
    }

    if (curve_.size() < 2) // two or more points are needed
      curve_ = curveDefault_;
  }
}

bool const AMD::OdFanCurveXMLParser::registered_ =
    ProfilePartXMLParserProvider::registerProvider(AMD::OdFanCurve::ItemID, []() {
      return std::make_unique<AMD::OdFanCurveXMLParser>();
    });
