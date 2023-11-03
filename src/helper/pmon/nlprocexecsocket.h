// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include <stdexcept>

struct ProcessEvent;

class NLProcExecSocket final
{
 public:
  class BindError : public std::runtime_error
  {
   public:
    using std::runtime_error::runtime_error;
  };

  NLProcExecSocket();
  ~NLProcExecSocket();

  ProcessEvent waitForEvent() const;

 private:
  struct FDHandle
  {
    int fd{-1};
    ~FDHandle();
  } sockFd_;
};
