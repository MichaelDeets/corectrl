// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef enum process_event_type_t {
  PROCESS_EVENT_EXEC,
  PROCESS_EVENT_EXIT,
  PROCESS_EVENT_OTHER,
} process_event_type;

typedef struct process_event_t
{
  process_event_type type;
  int pid;
} process_event;

int process_event_connector_new();
int process_event_connector_close(int socket_fd);
int process_event_connector_set_timeout(int socket_fd, unsigned int seconds);
int process_event_connector_bind(int socket_fd);
int process_event_connector_subscribe(int socket_fd, bool subscribe);
int process_event_connector_install_filter(int socket_fd);
process_event process_event_connector_read_event(int socket_fd);
