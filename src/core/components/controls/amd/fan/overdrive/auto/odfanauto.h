// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/components/controls/control.h"
#include "core/idatasource.h"
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace AMD {

class OdFanAuto : public Control
{
 public:
  static constexpr std::string_view ItemID{"AMD_OD_FAN_AUTO"};

  OdFanAuto(std::unique_ptr<IDataSource<std::vector<std::string>>>
                &&dataSource) noexcept;

  void preInit(ICommandQueue &ctlCmds) final override;
  void postInit(ICommandQueue &ctlCmds) final override;
  void init() final override;

  std::string const &ID() const final override;

 protected:
  void importControl(IControl::Importer &i) final override;
  void exportControl(IControl::Exporter &e) const final override;

  void cleanControl(ICommandQueue &ctlCmds) final override;
  void syncControl(ICommandQueue &ctlCmds) final override;

 private:
  void addResetCmds(ICommandQueue &ctlCmds) const;

  std::string const id_;
  bool triggerAutoOpMode_;
  std::unique_ptr<IDataSource<std::vector<std::string>>> const dataSource_;
};

} // namespace AMD
