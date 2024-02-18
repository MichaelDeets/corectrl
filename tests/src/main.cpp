// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include <catch2/catch_all.hpp>
#include <catch2/trompeloeil.hpp>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

void setupLogger()
{
  auto logger = spdlog::create<spdlog::sinks::null_sink_st>("null_logger");
  spdlog::set_default_logger(logger);
}

int main(int argc, char *argv[])
{
  setupLogger();
  return Catch::Session().run(argc, argv);
}
