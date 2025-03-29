// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Juan Palacios <jpalaciosdev@gmail.com>

#include "pmfreqoffsetprofilepart.h"

#include "core/profilepartprovider.h"
#include <algorithm>
#include <memory>

class AMD::PMFreqOffsetProfilePart::Initializer final
: public AMD::PMFreqOffset::Exporter
{
 public:
  Initializer(AMD::PMFreqOffsetProfilePart &outer) noexcept
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
  void takePMFreqOffsetRange(units::frequency::megahertz_t min,
                             units::frequency::megahertz_t max) override;
  void takePMFreqOffsetValue(units::frequency::megahertz_t value) override;

 private:
  AMD::PMFreqOffsetProfilePart &outer_;
};

void AMD::PMFreqOffsetProfilePart::Initializer::takeActive(bool active)
{
  outer_.activate(active);
}

void AMD::PMFreqOffsetProfilePart::Initializer::takePMFreqOffsetControlName(
    std::string const &name)
{
  outer_.controlName_ = name;
}

void AMD::PMFreqOffsetProfilePart::Initializer::takePMFreqOffsetRange(
    units::frequency::megahertz_t min, units::frequency::megahertz_t max)
{
  outer_.range_ = std::make_pair(min, max);
}

void AMD::PMFreqOffsetProfilePart::Initializer::takePMFreqOffsetValue(
    units::frequency::megahertz_t value)
{
  outer_.offset_ = value;
}

AMD::PMFreqOffsetProfilePart::PMFreqOffsetProfilePart() noexcept
: id_(AMD::PMFreqOffset::ItemID)
{
}

std::string const &AMD::PMFreqOffsetProfilePart::ID() const
{
  return id_;
}

std::string const &AMD::PMFreqOffsetProfilePart::instanceID() const
{
  return controlName_;
}

std::unique_ptr<Exportable::Exporter>
AMD::PMFreqOffsetProfilePart::factory(IProfilePartProvider const &)
{
  return nullptr;
}

std::unique_ptr<Exportable::Exporter> AMD::PMFreqOffsetProfilePart::initializer()
{
  return std::make_unique<AMD::PMFreqOffsetProfilePart::Initializer>(*this);
}

std::optional<std::reference_wrapper<Importable::Importer>>
AMD::PMFreqOffsetProfilePart::provideImporter(Item const &)
{
  return {};
}

bool AMD::PMFreqOffsetProfilePart::provideActive() const
{
  return active();
}

units::frequency::megahertz_t
AMD::PMFreqOffsetProfilePart::providePMFreqOffsetValue() const
{
  return offset_;
}

void AMD::PMFreqOffsetProfilePart::importProfilePart(IProfilePart::Importer &i)
{
  auto &importer = dynamic_cast<AMD::PMFreqOffsetProfilePart::Importer &>(i);
  offset_ = std::clamp(importer.providePMFreqOffsetValue(), range_.first,
                       range_.second);
}

void AMD::PMFreqOffsetProfilePart::exportProfilePart(IProfilePart::Exporter &e) const
{
  auto &exporter = dynamic_cast<AMD::PMFreqOffsetProfilePart::Exporter &>(e);
  exporter.takePMFreqOffsetControlName(controlName_);
  exporter.takePMFreqOffsetValue(offset_);
}

std::unique_ptr<IProfilePart> AMD::PMFreqOffsetProfilePart::cloneProfilePart() const
{
  auto clone = std::make_unique<AMD::PMFreqOffsetProfilePart>();
  clone->controlName_ = controlName_;
  clone->range_ = range_;
  clone->offset_ = offset_;

  return std::move(clone);
}

bool const AMD::PMFreqOffsetProfilePart::registered_ =
    ProfilePartProvider::registerProvider(AMD::PMFreqOffset::ItemID, []() {
      return std::make_unique<AMD::PMFreqOffsetProfilePart>();
    });
