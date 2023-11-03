// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

#include "nlprocexecsocket.h"

#include "processevent.h"

extern "C" {
#include "processeventconnector.h"
}

NLProcExecSocket::FDHandle::~FDHandle()
{
  process_event_connector_close(fd);
}

NLProcExecSocket::NLProcExecSocket()
{
  sockFd_.fd = process_event_connector_new();
  if (sockFd_.fd < 0)
    throw std::runtime_error("Cannot create netlink socket");

  if (process_event_connector_set_timeout(sockFd_.fd, 5) < 0)
    throw std::runtime_error("Cannot set socket timeout");

  if (process_event_connector_install_filter(sockFd_.fd) < 0)
    throw std::runtime_error("Cannot install socket filters");

  if (process_event_connector_bind(sockFd_.fd) < 0)
    throw BindError("Cannot bind to socket");

  if (process_event_connector_subscribe(sockFd_.fd, true) < 0)
    throw std::runtime_error("Cannot subscribe to proc events");
}

NLProcExecSocket::~NLProcExecSocket()
{
  process_event_connector_subscribe(sockFd_.fd, false);
}

ProcessEvent NLProcExecSocket::waitForEvent() const
{
  auto event = process_event_connector_read_event(sockFd_.fd);
  switch (event.type) {
    case process_event_type::PROCESS_EVENT_EXEC:
      return ProcessEvent{ProcessEvent::Type::EXEC, event.pid};
      break;
    case process_event_type::PROCESS_EVENT_EXIT:
      return ProcessEvent{ProcessEvent::Type::EXIT, event.pid};
      break;
    default:
      return ProcessEvent{ProcessEvent::Type::IGNORE, -1};
  }
}
