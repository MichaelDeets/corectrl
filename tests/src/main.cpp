// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>
#include <easylogging++.h>

INITIALIZE_EASYLOGGINGPP

void setupLogger()
{
  el::Configurations c;
  c.setToDefault();
  c.setGlobally(el::ConfigurationType::Enabled, "false");
  c.setGlobally(el::ConfigurationType::ToFile, "false");

  el::Loggers::setDefaultConfigurations(c, true);
}

int main(int argc, char *argv[])
{
  setupLogger();
  return Catch::Session().run(argc, argv);
}
