// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Utils {
namespace CPU {

/// Returns a piece of information from /proc/cpuinfo.
/// @param procCpuInfoLines /proc/cpuinfo data source contents lines
/// @param cpuId cpu id from which the info will be retrieved
/// @param target label that describes the information to retrieve
std::optional<std::string>
parseProcCpuInfo(std::vector<std::string> const &procCpuInfoLines, int cpuId,
                 std::string_view target);

/// CPU Stat captured from /proc/stat.
struct Stat
{
  std::uint64_t user;
  std::uint64_t nice;
  std::uint64_t system;
  std::uint64_t idle;
  std::uint64_t ioWait;
  std::uint64_t irq;
  std::uint64_t softIrq;
  std::uint64_t steal;
  std::uint64_t total;
};

/// Returns the aggregate CPU Stat from /proc/stat.
/// @param procStatLines /proc/stat data.
/// @returns CPU stat or none when the CPU stat cannot be parsed from the input data.
std::optional<Stat> parseProcStat(std::vector<std::string> const &procStatLines);

/// Computes the used CPU in a time interval defined by statT0 and statT1.
/// @param statT0 CPU stat captured in time 0.
/// @param statT1 CPU stat captured in time 1 (some time after statT0).
/// @returns percentage of the CPU used in the time interval.
unsigned int computeCPUUsage(Stat const &statT0, Stat const &statT1);

} // namespace CPU
} // namespace Utils
