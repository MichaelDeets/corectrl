// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Juan Palacios <jpalaciosdev@gmail.com>

#include "pmfreqoffsetxmlparser.h"

#include "core/profilepartxmlparserprovider.h"
#include "pmfreqoffset.h"
#include <algorithm>
#include <memory>

class AMD::PMFreqOffsetXMLParser::Initializer final
: public AMD::PMFreqOffsetProfilePart::Exporter
{
 public:
  Initializer(AMD::PMFreqOffsetXMLParser &outer) noexcept
  : outer_(outer)
  {
  }

  std::optional<std::reference_wrapper<Exportable::Exporter>>
  provideExporter(Item const &) override
  {
    return {};
  }

  void takeActive(bool active) override;
  void takePMFreqOffsetControlName(std::string const &name) override;
  void takePMFreqOffsetValue(units::frequency::megahertz_t value) override;

 private:
  AMD::PMFreqOffsetXMLParser &outer_;
};

void AMD::PMFreqOffsetXMLParser::Initializer::takeActive(bool active)
{
  outer_.active_ = outer_.activeDefault_ = active;
}

void AMD::PMFreqOffsetXMLParser::Initializer::takePMFreqOffsetControlName(
    std::string const &name)
{
  outer_.controlName_ = name;
  outer_.nodeID_ = name;
  std::transform(outer_.nodeID_.cbegin(), outer_.nodeID_.cend(),
                 outer_.nodeID_.begin(), ::tolower);
}

void AMD::PMFreqOffsetXMLParser::Initializer::takePMFreqOffsetValue(
    units::frequency::megahertz_t value)
{
  outer_.offset_ = outer_.offsetDefault_ = value;
}

AMD::PMFreqOffsetXMLParser::PMFreqOffsetXMLParser() noexcept
: ProfilePartXMLParser(AMD::PMFreqOffset::ItemID, *this, *this)
{
}

std::string const &AMD::PMFreqOffsetXMLParser::instanceID() const
{
  return controlName_;
}

std::unique_ptr<Exportable::Exporter>
AMD::PMFreqOffsetXMLParser::factory(IProfilePartXMLParserProvider const &)
{
  return nullptr;
}

std::unique_ptr<Exportable::Exporter> AMD::PMFreqOffsetXMLParser::initializer()
{
  return std::make_unique<AMD::PMFreqOffsetXMLParser::Initializer>(*this);
}

std::optional<std::reference_wrapper<Exportable::Exporter>>
AMD::PMFreqOffsetXMLParser::provideExporter(Item const &)
{
  return {};
}

std::optional<std::reference_wrapper<Importable::Importer>>
AMD::PMFreqOffsetXMLParser::provideImporter(Item const &)
{
  return {};
}

void AMD::PMFreqOffsetXMLParser::takeActive(bool active)
{
  active_ = active;
}

bool AMD::PMFreqOffsetXMLParser::provideActive() const
{
  return active_;
}

void AMD::PMFreqOffsetXMLParser::takePMFreqOffsetControlName(std::string const &)
{
}

void AMD::PMFreqOffsetXMLParser::takePMFreqOffsetValue(
    units::frequency::megahertz_t value)
{
  offset_ = value;
}

units::frequency::megahertz_t
AMD::PMFreqOffsetXMLParser::providePMFreqOffsetValue() const
{
  return offset_;
}

void AMD::PMFreqOffsetXMLParser::appendTo(pugi::xml_node &parentNode)
{
  auto node = parentNode.append_child(ID().c_str());

  node.append_attribute("active") = active_;
  node.append_attribute("id") = nodeID_.c_str();
  node.append_attribute("value") = offset_.to<int>();
}

void AMD::PMFreqOffsetXMLParser::resetAttributes()
{
  active_ = activeDefault_;
  offset_ = offsetDefault_;
}

void AMD::PMFreqOffsetXMLParser::loadPartFrom(pugi::xml_node const &parentNode)
{
  auto node = parentNode.find_child([&](pugi::xml_node const &node) {
    if (node.name() != ID())
      return false;

    return node.attribute("id").as_string() == nodeID_;
  });

  active_ = node.attribute("active").as_bool(activeDefault_);
  offset_ = units::frequency::megahertz_t(
      node.attribute("value").as_int(offsetDefault_.to<int>()));
}

bool const AMD::PMFreqOffsetXMLParser::registered_ =
    ProfilePartXMLParserProvider::registerProvider(
        AMD::PMFreqOffset::ItemID,
        []() { return std::make_unique<AMD::PMFreqOffsetXMLParser>(); });
