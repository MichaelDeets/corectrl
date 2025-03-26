// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#include "odfancurveprofilepart.h"

#include "core/components/commonutils.h"
#include "core/profilepartprovider.h"
#include <algorithm>
#include <memory>

class AMD::OdFanCurveProfilePart::Initializer final
: public AMD::OdFanCurve::Exporter
{
 public:
  Initializer(AMD::OdFanCurveProfilePart &outer) noexcept
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
  void takeFanCurveRange(AMD::OdFanCurve::TempRange temp,
                         AMD::OdFanCurve::SpeedRange speed) override;
  void takeFanStop(bool enabled) override;
  void takeFanStopTemp(units::temperature::celsius_t value) override;
  void takeFanStopTempRange(AMD::OdFanCurve::TempRange value) override;

 private:
  AMD::OdFanCurveProfilePart &outer_;
};

void AMD::OdFanCurveProfilePart::Initializer::takeActive(bool active)
{
  outer_.activate(active);
}

void AMD::OdFanCurveProfilePart::Initializer::takeFanCurve(
    std::vector<AMD::OdFanCurve::CurvePoint> const &curve)
{
  outer_.curve_ = curve;
}

void AMD::OdFanCurveProfilePart::Initializer::takeFanCurveRange(
    AMD::OdFanCurve::TempRange temp, AMD::OdFanCurve::SpeedRange speed)
{
  outer_.tempRange_ = temp;
  outer_.speedRange_ = speed;
}

void AMD::OdFanCurveProfilePart::Initializer::takeFanStop(bool enable)
{
  outer_.stop_ = enable;
}

void AMD::OdFanCurveProfilePart::Initializer::takeFanStopTemp(
    units::temperature::celsius_t value)
{
  outer_.stopTemp_ = value;
}

void AMD::OdFanCurveProfilePart::Initializer::takeFanStopTempRange(
    AMD::OdFanCurve::TempRange value)
{
  outer_.stopTempRange_ = value;
}

AMD::OdFanCurveProfilePart::OdFanCurveProfilePart() noexcept
: id_(AMD::OdFanCurve::ItemID)
{
}

std::unique_ptr<Exportable::Exporter>
AMD::OdFanCurveProfilePart::factory(IProfilePartProvider const &)
{
  return nullptr;
}

std::unique_ptr<Exportable::Exporter> AMD::OdFanCurveProfilePart::initializer()
{
  return std::make_unique<AMD::OdFanCurveProfilePart::Initializer>(*this);
}

std::string const &AMD::OdFanCurveProfilePart::ID() const
{
  return id_;
}

std::optional<std::reference_wrapper<Importable::Importer>>
AMD::OdFanCurveProfilePart::provideImporter(Item const &)
{
  return {};
}

bool AMD::OdFanCurveProfilePart::provideActive() const
{
  return active();
}

std::vector<AMD::OdFanCurve::CurvePoint> const &
AMD::OdFanCurveProfilePart::provideFanCurve() const
{
  return curve_;
}

bool AMD::OdFanCurveProfilePart::provideFanStop() const
{
  return *stop_;
}

units::temperature::celsius_t AMD::OdFanCurveProfilePart::provideFanStopTemp() const
{
  return *stopTemp_;
}

void AMD::OdFanCurveProfilePart::importProfilePart(IProfilePart::Importer &i)
{
  auto &importer = dynamic_cast<AMD::OdFanCurveProfilePart::Importer &>(i);
  curve(importer.provideFanCurve());

  if (stop_)
    stop_ = importer.provideFanStop();

  if (stopTemp_)
    stopTemp(importer.provideFanStopTemp());
}

void AMD::OdFanCurveProfilePart::exportProfilePart(IProfilePart::Exporter &e) const
{
  auto &exporter = dynamic_cast<AMD::OdFanCurveProfilePart::Exporter &>(e);
  exporter.takeFanCurve(curve_);

  if (stop_)
    exporter.takeFanStop(*stop_);

  if (stopTemp_)
    exporter.takeFanStopTemp(*stopTemp_);
}

std::unique_ptr<IProfilePart> AMD::OdFanCurveProfilePart::cloneProfilePart() const
{
  auto clone = std::make_unique<AMD::OdFanCurveProfilePart>();
  clone->curve_ = curve_;
  clone->tempRange_ = tempRange_;
  clone->speedRange_ = speedRange_;
  clone->stop_ = stop_;
  clone->stopTemp_ = stopTemp_;
  clone->stopTempRange_ = stopTempRange_;

  return std::move(clone);
}

void AMD::OdFanCurveProfilePart::curve(
    std::vector<OdFanCurve::CurvePoint> const &curve)
{
  curve_ = curve;
  Utils::Common::normalizePoints(curve_, tempRange_, speedRange_);
}

void AMD::OdFanCurveProfilePart::stopTemp(units::temperature::celsius_t value)
{
  stopTemp_ = std::clamp(value, stopTempRange_->first, stopTempRange_->second);
}

bool const AMD::OdFanCurveProfilePart::registered_ =
    ProfilePartProvider::registerProvider(AMD::OdFanCurve::ItemID, []() {
      return std::make_unique<AMD::OdFanCurveProfilePart>();
    });
