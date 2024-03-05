// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "cpufreqprofilepart.h"

#include "core/profilepartprovider.h"
#include <algorithm>
#include <memory>
#include <utility>

class CPUFreqProfilePart::Initializer final : public CPUFreq::Exporter
{
 public:
  Initializer(CPUFreqProfilePart &outer) noexcept
  : outer_(outer)
  {
  }

  std::optional<std::reference_wrapper<Exportable::Exporter>>
  provideExporter(Item const &) override
  {
    return {};
  }

  void takeActive(bool active) override;
  void takeCPUFreqScalingGovernor(std::string const &governor) override;
  void
  takeCPUFreqScalingGovernors(std::vector<std::string> const &governors) override;
  void takeCPUFreqEPPHint(std::optional<std::string> const &hint) override;
  void
  takeCPUFreqEPPHints(std::optional<std::vector<std::string>> const &hints) override;

 private:
  CPUFreqProfilePart &outer_;
};

void CPUFreqProfilePart::Initializer::takeActive(bool active)
{
  outer_.activate(active);
}

void CPUFreqProfilePart::Initializer::takeCPUFreqScalingGovernor(
    std::string const &governor)
{
  outer_.governor_ = governor;
}

void CPUFreqProfilePart::Initializer::takeCPUFreqScalingGovernors(
    std::vector<std::string> const &governors)
{
  outer_.governors_ = governors;
}

void CPUFreqProfilePart::Initializer::takeCPUFreqEPPHint(
    std::optional<std::string> const &hint)
{
  outer_.eppHint_ = hint;
}

void CPUFreqProfilePart::Initializer::takeCPUFreqEPPHints(
    std::optional<std::vector<std::string>> const &hints)
{
  outer_.eppHints_ = hints;
}

CPUFreqProfilePart::CPUFreqProfilePart() noexcept
: id_(CPUFreq::ItemID)
{
}

std::unique_ptr<Exportable::Exporter>
CPUFreqProfilePart::factory(IProfilePartProvider const &)
{
  return nullptr;
}

std::unique_ptr<Exportable::Exporter> CPUFreqProfilePart::initializer()
{
  return std::make_unique<CPUFreqProfilePart::Initializer>(*this);
}

std::string const &CPUFreqProfilePart::ID() const
{
  return id_;
}

std::optional<std::reference_wrapper<Importable::Importer>>
CPUFreqProfilePart::provideImporter(Item const &)
{
  return {};
}

bool CPUFreqProfilePart::provideActive() const
{
  return active();
}

std::string const &CPUFreqProfilePart::provideCPUFreqScalingGovernor() const
{
  return governor_;
}

std::optional<std::string> const &CPUFreqProfilePart::provideCPUFreqEPPHint() const
{
  return eppHint_;
}

void CPUFreqProfilePart::importProfilePart(IProfilePart::Importer &i)
{
  auto &importer = dynamic_cast<CPUFreqProfilePart::Importer &>(i);
  governor(importer.provideCPUFreqScalingGovernor());
  eppHint(importer.provideCPUFreqEPPHint());
}

void CPUFreqProfilePart::exportProfilePart(IProfilePart::Exporter &e) const
{
  auto &exporter = dynamic_cast<CPUFreqProfilePart::Exporter &>(e);
  exporter.takeCPUFreqScalingGovernor(governor_);
  exporter.takeCPUFreqEPPHint(eppHint_);
}

std::unique_ptr<IProfilePart> CPUFreqProfilePart::cloneProfilePart() const
{
  auto clone = std::make_unique<CPUFreqProfilePart>();
  clone->governors_ = governors_;
  clone->governor_ = governor_;
  clone->eppHint_ = eppHint_;
  clone->eppHints_ = eppHints_;

  return std::move(clone);
}

void CPUFreqProfilePart::governor(std::string const &governor)
{
  // only import known governors
  auto iter = std::find_if(governors_.cbegin(), governors_.cend(),
                           [&](auto const &availableGovernor) {
                             return governor == availableGovernor;
                           });
  if (iter != governors_.cend())
    governor_ = governor;
}

void CPUFreqProfilePart::eppHint(std::optional<std::string> const &hint)
{
  if (!hint || !eppHints_)
    return;

  // only import known hints
  auto iter = std::find_if(
      eppHints_->cbegin(), eppHints_->cend(),
      [&](auto const &availableHint) { return *hint == availableHint; });
  if (iter != eppHints_->cend())
    eppHint_ = hint;
}

bool const CPUFreqProfilePart::registered_ = ProfilePartProvider::registerProvider(
    CPUFreq::ItemID, []() { return std::make_unique<CPUFreqProfilePart>(); });
