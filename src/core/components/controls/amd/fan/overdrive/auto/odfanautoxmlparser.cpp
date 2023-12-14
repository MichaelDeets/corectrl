// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#include "odfanautoxmlparser.h"

#include "core/profilepartxmlparserprovider.h"
#include "odfanauto.h"
#include <memory>

class AMD::OdFanAutoXMLParser::Initializer final
: public AMD::OdFanAutoProfilePart::Exporter
{
 public:
  Initializer(AMD::OdFanAutoXMLParser &outer) noexcept
  : outer_(outer)
  {
  }

  std::optional<std::reference_wrapper<Exportable::Exporter>>
  provideExporter(Item const &) override
  {
    return {};
  }

  void takeActive(bool active) override;

 private:
  AMD::OdFanAutoXMLParser &outer_;
};

void AMD::OdFanAutoXMLParser::Initializer::takeActive(bool active)
{
  outer_.active_ = outer_.activeDefault_ = active;
}

AMD::OdFanAutoXMLParser::OdFanAutoXMLParser() noexcept
: ProfilePartXMLParser(AMD::OdFanAuto::ItemID, *this, *this)
{
}

std::unique_ptr<Exportable::Exporter>
AMD::OdFanAutoXMLParser::factory(IProfilePartXMLParserProvider const &)
{
  return nullptr;
}

std::unique_ptr<Exportable::Exporter> AMD::OdFanAutoXMLParser::initializer()
{
  return std::make_unique<AMD::OdFanAutoXMLParser::Initializer>(*this);
}

std::optional<std::reference_wrapper<Exportable::Exporter>>
AMD::OdFanAutoXMLParser::provideExporter(Item const &)
{
  return {};
}

std::optional<std::reference_wrapper<Importable::Importer>>
AMD::OdFanAutoXMLParser::provideImporter(Item const &)
{
  return {};
}

void AMD::OdFanAutoXMLParser::takeActive(bool active)
{
  active_ = active;
}

bool AMD::OdFanAutoXMLParser::provideActive() const
{
  return active_;
}

void AMD::OdFanAutoXMLParser::appendTo(pugi::xml_node &parentNode)
{
  auto pmFixedNode = parentNode.append_child(ID().c_str());
  pmFixedNode.append_attribute("active") = active_;
}

void AMD::OdFanAutoXMLParser::resetAttributes()
{
  active_ = activeDefault_;
}

void AMD::OdFanAutoXMLParser::loadPartFrom(pugi::xml_node const &parentNode)
{
  auto pmFixedNode = parentNode.find_child(
      [&](pugi::xml_node const &node) { return node.name() == ID(); });

  active_ = pmFixedNode.attribute("active").as_bool(activeDefault_);
}

bool const AMD::OdFanAutoXMLParser::registered_ =
    ProfilePartXMLParserProvider::registerProvider(AMD::OdFanAuto::ItemID, []() {
      return std::make_unique<AMD::OdFanAutoXMLParser>();
    });
