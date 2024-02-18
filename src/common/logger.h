// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include <filesystem>
#include <iostream>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

void setupLogger(std::filesystem::path const &logFilePath)
{
  try {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
    console_sink->set_pattern("[%d-%m-%C %H:%M:%S.%e][%L] %v");
    console_sink->set_level(spdlog::level::info);

    auto file_sink =
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath);
    file_sink->set_pattern("[%d-%m-%C %H:%M:%S.%e][%L][%s:%#] %v");
    file_sink->set_level(spdlog::level::trace);

    auto logger = std::make_shared<spdlog::logger>(
        "logger", spdlog::sinks_init_list({console_sink, file_sink}));
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::debug);
    spdlog::set_default_logger(logger);
  }
  catch (spdlog::spdlog_ex const &e) {
    std::cout << "Logger initialization failed: " << e.what() << std::endl;
  }
}
