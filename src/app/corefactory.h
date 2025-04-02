// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include "core/info/vendor.h"
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

class IHelperControl;
class ISysModelSyncer;
class ISession;
class IUIFactory;

class CoreFactory final
{
 public:
  struct Components
  {
    std::unique_ptr<IHelperControl> helperControl;
    std::shared_ptr<ISysModelSyncer> sysSyncer;
    std::unique_ptr<ISession> session;
    std::unique_ptr<IUIFactory> uiFactory;
  };

  CoreFactory() noexcept;
  std::optional<Components> build(std::string &&appName) const;

 private:
  std::tuple<std::filesystem::path, std::filesystem::path>
  standardDirectories() const;
  void createAppDirectories(std::string const &appDirectory,
                            std::filesystem::path const &config,
                            std::filesystem::path const &cache) const;

  std::vector<Vendor> gpuVendors_;
};
