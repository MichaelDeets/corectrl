// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include <catch2/catch.hpp>

#include "core/components/cpuutils.h"

namespace Tests {
namespace Utils {
namespace CPU {

TEST_CASE("CPU utils tests", "[Utils][CPU]")
{
  SECTION("parseProcCpuInfo")
  {
    // clang-format off
    std::vector<std::string> input{"processor	: 0",
                                   "target	: data"};
    // clang-format on

    SECTION("Returns target data")
    {
      auto data = ::Utils::CPU::parseProcCpuInfo(input, 0, "target");
      REQUIRE(data.has_value());
      REQUIRE(*data == "data");
    }

    SECTION("Returns nothing when input doesn't have the requested processor")
    {
      auto data = ::Utils::CPU::parseProcCpuInfo(input, 10, "target");
      REQUIRE_FALSE(data.has_value());
    }

    SECTION("Returns nothing when input doesn't have the requested target on "
            "the processor")
    {
      auto data = ::Utils::CPU::parseProcCpuInfo(input, 0, "wrong_target");
      REQUIRE_FALSE(data.has_value());
    }
  }

  SECTION("parseProcStat")
  {
    SECTION("Returns the CPU aggregate stats from /proc/stat info")
    {
      std::vector<std::string> input{
          "cpu 141434 264 40474 2021588 12439 5752 2750 0 0 0"};
      auto stat = ::Utils::CPU::parseProcStat(input);
      REQUIRE(stat.has_value());
      REQUIRE(stat->user == 141434);
      REQUIRE(stat->nice == 264);
      REQUIRE(stat->system == 40474);
      REQUIRE(stat->idle == 2021588);
      REQUIRE(stat->ioWait == 12439);
      REQUIRE(stat->irq == 5752);
      REQUIRE(stat->softIrq == 2750);
      REQUIRE(stat->steal == 0);
      REQUIRE(stat->total ==
              141434 + 264 + 40474 + 2021588 + 12439 + 5752 + 2750 + 0);
    }

    SECTION("Returns nothing when some stat values are missing from the cpu "
            "aggregate line")
    {
      std::vector<std::string> input{"cpu 141434 264 40474 2021588 0 0 0"};
      auto stat = ::Utils::CPU::parseProcStat(input);
      REQUIRE_FALSE(stat.has_value());
    }

    SECTION("Returns nothing when the cpu aggregate line is missing")
    {
      std::vector<std::string> input{"other data"};
      auto stat = ::Utils::CPU::parseProcStat(input);
      REQUIRE_FALSE(stat.has_value());
    }
  }

  SECTION("computeCPUUsage")
  {
    SECTION("Returns CPU usage")
    {
      std::uint64_t total = 723998 + 5703 + 159187 + 9095956 + 22936 + 23633 +
                            10115 + 0;
      auto statT0 = ::Utils::CPU::Stat{723998, 5703,  159187, 9095956, 22936,
                                       23633,  10115, 0,      total};
      total = 725077 + 5703 + 159248 + 9095956 + 22936 + 23638 + 10116 + 0;
      auto statT1 = ::Utils::CPU::Stat{725077, 5703,  159248, 9095956, 22936,
                                       23638,  10116, 0,      total};
      auto usage = ::Utils::CPU::computeCPUUsage(statT0, statT1);
      REQUIRE(usage == 100);

      total = 664215 + 2940 + 147770 + 6752849 + 19569 + 21752 + 9330 + 0;
      statT0 = ::Utils::CPU::Stat{664215, 2940, 147770, 6752849, 19569,
                                  21752,  9330, 0,      total};
      total = 664215 + 2940 + 147770 + 6754574 + 19583 + 21752 + 9330 + 0;
      statT1 = ::Utils::CPU::Stat{664215, 2940, 147770, 6754574, 19583,
                                  21752,  9330, 0,      total};
      usage = ::Utils::CPU::computeCPUUsage(statT0, statT1);
      REQUIRE(usage == 0);
    }
  }
}

} // namespace CPU
} // namespace Utils
} // namespace Tests
