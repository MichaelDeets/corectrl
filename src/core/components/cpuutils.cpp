// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "cpuutils.h"

#include "common/stringutils.h"
#include <algorithm>
#include <iterator>
#include <regex>

namespace Utils {
namespace CPU {

std::optional<std::string>
parseProcCpuInfo(std::vector<std::string> const &procCpuInfoLines, int cpuId,
                 std::string_view target)
{
  static constexpr std::string_view cpuIdStr{"processor"};

  auto lineIt = std::find_if(
      procCpuInfoLines.cbegin(), procCpuInfoLines.cend(),
      [=](std::string const &line) {
        if (line.find(cpuIdStr) != std::string::npos) {
          int value;
          auto idPos = line.find_first_not_of("\t: ", cpuIdStr.size());
          return idPos != std::string::npos &&
                 Utils::String::toNumber<int>(value, line.substr(idPos)) &&
                 value == cpuId;
        }
        return false;
      });

  if (lineIt != procCpuInfoLines.cend()) {
    lineIt = std::next(lineIt);

    for (; lineIt != procCpuInfoLines.cend(); lineIt = std::next(lineIt)) {

      if (lineIt->empty())
        break; // target not found on this processor section

      auto targetPos = lineIt->find(target);
      if (targetPos != std::string::npos) {
        auto dataPos = lineIt->find_first_not_of("\t: ", target.size());
        if (dataPos != std::string::npos)
          return lineIt->substr(dataPos);
      }
    }
  }

  return {};
}

std::optional<Stat> parseProcStat(std::vector<std::string> const &procStatLines)
{
  // CPU aggregate line columns:
  //   cpu user nice system idle io_wait irq soft_irq steal guest guest_nice
  //
  // NOTE guest and guest_nice values are already included into user and nice.
  // https://github.com/torvalds/linux/blob/master/kernel/sched/cputime.c#L143?h=5.0
  static std::regex const regex(
      R"(^cpu\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+\d+\s+\d+\s*$)",
      std::regex::icase);

  for (auto &line : procStatLines) {
    std::smatch result;
    if (std::regex_match(line, result, regex)) {
      std::uint64_t user{0}, nice{0}, system{0}, idle{0}, ioWait{0}, irq{0},
          softIrq{0}, steal{0}, total{0};

      if (Utils::String::toNumber<std::uint64_t>(user, result[1]) &&
          Utils::String::toNumber<std::uint64_t>(nice, result[2]) &&
          Utils::String::toNumber<std::uint64_t>(system, result[3]) &&
          Utils::String::toNumber<std::uint64_t>(idle, result[4]) &&
          Utils::String::toNumber<std::uint64_t>(ioWait, result[5]) &&
          Utils::String::toNumber<std::uint64_t>(irq, result[6]) &&
          Utils::String::toNumber<std::uint64_t>(softIrq, result[7]) &&
          Utils::String::toNumber<std::uint64_t>(steal, result[8])) {

        total = user + nice + system + idle + ioWait + irq + softIrq + steal;
        return Stat{user, nice,    system, idle, ioWait,
                    irq,  softIrq, steal,  total};
      }
    }
  }

  return {};
}

unsigned int computeCPUUsage(Stat const &statT0, Stat const &statT1)
{
  auto total = statT1.total - statT0.total;
  auto idle = (statT1.idle + statT1.ioWait) - (statT0.idle + statT0.ioWait);
  auto used = total - idle;

  return (100.0 * used) / total;
}

} // namespace CPU
} // namespace Utils
