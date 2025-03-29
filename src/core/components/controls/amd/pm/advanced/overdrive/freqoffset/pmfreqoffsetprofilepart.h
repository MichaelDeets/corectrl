// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2025 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/profilepart.h"
#include "pmfreqoffset.h"
#include <string>
#include <utility>

namespace AMD {

class PMFreqOffsetProfilePart final
: public ProfilePart
, public PMFreqOffset::Importer
{
 public:
  class Importer : public IProfilePart::Importer
  {
   public:
    virtual units::frequency::megahertz_t providePMFreqOffsetValue() const = 0;
  };

  class Exporter : public IProfilePart::Exporter
  {
   public:
    virtual void takePMFreqOffsetControlName(std::string const &mode) = 0;
    virtual void takePMFreqOffsetValue(units::frequency::megahertz_t value) = 0;
  };

  PMFreqOffsetProfilePart() noexcept;

  std::string const &ID() const override;
  std::string const &instanceID() const override;

  std::unique_ptr<Exportable::Exporter>
  factory(IProfilePartProvider const &profilePartProvider) override;
  std::unique_ptr<Exportable::Exporter> initializer() override;

  std::optional<std::reference_wrapper<Importable::Importer>>
  provideImporter(Item const &i) override;

  bool provideActive() const override;

  units::frequency::megahertz_t providePMFreqOffsetValue() const override;

 protected:
  void importProfilePart(IProfilePart::Importer &i) override;
  void exportProfilePart(IProfilePart::Exporter &e) const override;
  std::unique_ptr<IProfilePart> cloneProfilePart() const override;

 private:
  class Initializer;

  std::string const id_;
  std::string controlName_;
  units::frequency::megahertz_t offset_;
  std::pair<units::frequency::megahertz_t, units::frequency::megahertz_t> range_;

  static bool const registered_;
};
} // namespace AMD
