// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#include "odfanautoprofilepart.h"

#include "core/profilepartprovider.h"
#include <memory>

class AMD::OdFanAutoProfilePart::Initializer final
: public AMD::OdFanAuto::Exporter
{
 public:
  Initializer(AMD::OdFanAutoProfilePart &outer) noexcept
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
  AMD::OdFanAutoProfilePart &outer_;
};

void AMD::OdFanAutoProfilePart::Initializer::takeActive(bool active)
{
  outer_.activate(active);
}

AMD::OdFanAutoProfilePart::OdFanAutoProfilePart() noexcept
: id_(AMD::OdFanAuto::ItemID)
{
}

std::unique_ptr<Exportable::Exporter>
AMD::OdFanAutoProfilePart::factory(IProfilePartProvider const &)
{
  return nullptr;
}

std::unique_ptr<Exportable::Exporter> AMD::OdFanAutoProfilePart::initializer()
{
  return std::make_unique<AMD::OdFanAutoProfilePart::Initializer>(*this);
}

std::string const &AMD::OdFanAutoProfilePart::ID() const
{
  return id_;
}

std::optional<std::reference_wrapper<Importable::Importer>>
AMD::OdFanAutoProfilePart::provideImporter(Item const &)
{
  return {};
}

bool AMD::OdFanAutoProfilePart::provideActive() const
{
  return active();
}

void AMD::OdFanAutoProfilePart::importProfilePart(IProfilePart::Importer &)
{
}

void AMD::OdFanAutoProfilePart::exportProfilePart(IProfilePart::Exporter &) const
{
}

std::unique_ptr<IProfilePart> AMD::OdFanAutoProfilePart::cloneProfilePart() const
{
  return std::make_unique<AMD::OdFanAutoProfilePart>();
}

bool const AMD::OdFanAutoProfilePart::registered_ =
    ProfilePartProvider::registerProvider(AMD::OdFanAuto::ItemID, []() {
      return std::make_unique<AMD::OdFanAutoProfilePart>();
    });
