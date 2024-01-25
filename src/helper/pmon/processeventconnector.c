// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2023 Juan Palacios <jpalaciosdev@gmail.com>

#include "processeventconnector.h"

#include <linux/bpf_common.h>
#include <linux/cn_proc.h>
#include <linux/connector.h>
#include <linux/filter.h>
#include <linux/netlink.h>
#include <linux/version.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define SUBSCRIBE_MSG_SIZE                                                     \
  NLMSG_LENGTH(sizeof(struct cn_msg) + sizeof(enum proc_cn_mcast_op))
#define READ_EVENT_MSG_SIZE                                                    \
  NLMSG_LENGTH(sizeof(struct cn_msg) + sizeof(struct proc_event))

int process_event_connector_new()
{
  return socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
}

int process_event_connector_close(int socket_fd)
{
  if (socket_fd >= 0)
    return close(socket_fd);

  return 0;
}

int process_event_connector_set_timeout(int socket_fd, unsigned int seconds)
{
  struct timeval duration;
  duration.tv_sec = seconds;
  duration.tv_usec = 0;

  return setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &duration,
                    sizeof(duration));
}

int process_event_connector_bind(int socket_fd)
{
  struct sockaddr_nl sa_nl;
  sa_nl.nl_family = AF_NETLINK;
  sa_nl.nl_groups = CN_IDX_PROC;
  sa_nl.nl_pid = 0; // 0 = lets the kernel to handle nl_pid

  return bind(socket_fd, (struct sockaddr *)(&sa_nl), sizeof(sa_nl));
}

int process_event_connector_subscribe(int socket_fd, bool subscribe)
{
  typedef struct nlcn_msg_t
  {
    struct nlmsghdr nl_hdr __attribute__((aligned(NLMSG_ALIGNTO)));
    struct cn_msg cn_msg;
  } nlcn_msg;

  char msg_buffer[SUBSCRIBE_MSG_SIZE] = {0};
  nlcn_msg *msg = (nlcn_msg *)msg_buffer;

  msg->nl_hdr.nlmsg_len = SUBSCRIBE_MSG_SIZE;
  msg->nl_hdr.nlmsg_pid = 0;
  msg->nl_hdr.nlmsg_type = NLMSG_DONE;

  msg->cn_msg.id.idx = CN_IDX_PROC;
  msg->cn_msg.id.val = CN_VAL_PROC;
  msg->cn_msg.len = sizeof(enum proc_cn_mcast_op);

  enum proc_cn_mcast_op *mcast = (enum proc_cn_mcast_op *)msg->cn_msg.data;
  *mcast = subscribe ? PROC_CN_MCAST_LISTEN : PROC_CN_MCAST_IGNORE;

  if (send(socket_fd, msg, SUBSCRIBE_MSG_SIZE, 0) < 0)
    return -1;

  return 0;
}

int process_event_connector_install_filter(int socket_fd)
{
  struct sock_filter filter[] = {
      // clang-format off

      // check message from kernel
      BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct nlmsghdr, nlmsg_pid)),
      BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 0, 1, 0),
      BPF_STMT(BPF_RET | BPF_K, 0x0), // drop message

      // check message type NLMSG_DONE
      BPF_STMT(BPF_LD | BPF_H | BPF_ABS, offsetof(struct nlmsghdr, nlmsg_type)),
      BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, htons(NLMSG_DONE), 1, 0),
      BPF_STMT(BPF_RET | BPF_K, 0x0), // drop message

      // check proc connector event CN_IDX_PROC
      BPF_STMT(BPF_LD | BPF_W | BPF_ABS, NLMSG_LENGTH(0) +
                                         offsetof(struct cn_msg, id) +
                                         offsetof(struct cb_id, idx)),
      BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, htonl(CN_IDX_PROC), 1, 0),
      BPF_STMT(BPF_RET | BPF_K, 0x0), // drop message

      // check proc connector event CN_VAL_PROC
      BPF_STMT(BPF_LD | BPF_W | BPF_ABS, NLMSG_LENGTH(0) +
                                         offsetof(struct cn_msg, id) +
                                         offsetof(struct cb_id, val)),
      BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, htonl(CN_VAL_PROC), 1, 0),
      BPF_STMT(BPF_RET | BPF_K, 0x0), // drop message

      // accept exec messages from processes
      BPF_STMT(BPF_LD | BPF_W | BPF_ABS, NLMSG_LENGTH(0) +
                                         offsetof(struct cn_msg, data) +
                                         offsetof(struct proc_event, what)),
      BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, htonl(PROC_EVENT_EXEC), 0, 6),

      // processes have process_pid == process_tgid (thread group leaders)
      BPF_STMT(BPF_LD | BPF_W | BPF_ABS, NLMSG_LENGTH(0) +
                                         offsetof(struct cn_msg, data) +
                                         offsetof(struct proc_event, event_data.exec.process_pid)),
      BPF_STMT(BPF_ST, 0),
      BPF_STMT(BPF_LDX | BPF_W | BPF_MEM, 0),
      BPF_STMT(BPF_LD | BPF_W | BPF_ABS, NLMSG_LENGTH(0) +
                                         offsetof(struct cn_msg, data) +
                                         offsetof(struct proc_event, event_data.exec.process_tgid)),
      BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_X, 0, 0, 9),
      BPF_STMT(BPF_RET | BPF_K, 0xffffffff),

      // accept exit messages from processes
      BPF_STMT(BPF_LD | BPF_W | BPF_ABS, NLMSG_LENGTH(0) +
                                         offsetof(struct cn_msg, data) +
                                         offsetof(struct proc_event, what)),
      BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, htonl(PROC_EVENT_EXIT), 0, 6),

      // processes have process_pid == process_tgid
      BPF_STMT(BPF_LD | BPF_W | BPF_ABS, NLMSG_LENGTH(0) +
                                         offsetof(struct cn_msg, data) +
                                         offsetof(struct proc_event, event_data.exit.process_pid)),
      BPF_STMT(BPF_ST, 0),
      BPF_STMT(BPF_LDX | BPF_W | BPF_MEM, 0),
      BPF_STMT(BPF_LD | BPF_W | BPF_ABS, NLMSG_LENGTH(0) +
                                         offsetof(struct cn_msg, data) +
                                         offsetof(struct proc_event, event_data.exit.process_tgid)),
      BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_X, 0, 0, 1),
      BPF_STMT(BPF_RET | BPF_K, 0xffffffff),

      // WORKAROUND Process monitor not working anymore with Linux 6.7 and
      // 6.6.13. Might be caused by a process connector regression.
      BPF_STMT(BPF_RET | BPF_K, 0xffffffff),

      // clang-format on
  };

  struct sock_fprog fprog;
  memset(&fprog, 0, sizeof(fprog));
  fprog.filter = filter;
  fprog.len = sizeof(filter) / sizeof(*filter);

  return setsockopt(socket_fd, SOL_SOCKET, SO_ATTACH_FILTER, &fprog,
                    sizeof(fprog));
}

process_event process_event_connector_read_event(int socket_fd)
{
  typedef struct nlcn_msg_t
  {
    struct nlmsghdr nl_hdr __attribute__((aligned(NLMSG_ALIGNTO)));
    struct cn_msg cn_msg;
  } nlcn_msg;

  char msg_buffer[READ_EVENT_MSG_SIZE] = {0};
  nlcn_msg *msg = (nlcn_msg *)msg_buffer;

  ssize_t const rc = recv(socket_fd, msg, READ_EVENT_MSG_SIZE, 0);
  if (rc > 0) {
    struct proc_event *event = ((struct proc_event *)msg->cn_msg.data);
    switch (event->what) {
      case PROC_EVENT_EXEC:
        return (process_event){.type = PROCESS_EVENT_EXEC,
                               .pid = event->event_data.exec.process_pid};

      case PROC_EVENT_EXIT:
        return (process_event){.type = PROCESS_EVENT_EXIT,
                               .pid = event->event_data.exit.process_pid};

      default:
        return (process_event){.type = PROCESS_EVENT_OTHER, .pid = -1};
    }
  }

  return (process_event){.type = PROCESS_EVENT_OTHER, .pid = -1};
}
