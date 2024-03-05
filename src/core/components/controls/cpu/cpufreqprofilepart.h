// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/profilepart.h"
#include "cpufreq.h"
#include <optional>
#include <string>
#include <vector>

class CPUFreqProfilePart final
: public ProfilePart
, public CPUFreq::Importer
{
 public:
  class Importer : public IProfilePart::Importer
  {
   public:
    virtual std::string const &provideCPUFreqScalingGovernor() const = 0;
    virtual std::optional<std::string> const &provideCPUFreqEPPHint() const = 0;
  };

  class Exporter : public IProfilePart::Exporter
  {
   public:
    virtual void takeCPUFreqScalingGovernor(std::string const &governor) = 0;
    virtual void takeCPUFreqEPPHint(std::optional<std::string> const &hint) = 0;
  };

  CPUFreqProfilePart() noexcept;

  std::unique_ptr<Exportable::Exporter>
  factory(IProfilePartProvider const &profilePartProvider) override;
  std::unique_ptr<Exportable::Exporter> initializer() override;

  std::string const &ID() const final override;

  std::optional<std::reference_wrapper<Importable::Importer>>
  provideImporter(Item const &i) override;

  bool provideActive() const override;
  std::string const &provideCPUFreqScalingGovernor() const override;
  std::optional<std::string> const &provideCPUFreqEPPHint() const override;

 protected:
  void importProfilePart(IProfilePart::Importer &i) override;
  void exportProfilePart(IProfilePart::Exporter &e) const override;
  std::unique_ptr<IProfilePart> cloneProfilePart() const override;

 private:
  void governor(std::string const &governor);
  void eppHint(std::optional<std::string> const &hint);

  class Initializer;

  std::string const id_;
  std::string governor_;
  std::vector<std::string> governors_;

  std::optional<std::string> eppHint_;
  std::optional<std::vector<std::string>> eppHints_;

  static bool const registered_;
};
