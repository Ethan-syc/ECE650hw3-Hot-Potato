#include "utility.h"
#include <iostream>
using namespace std;
int select_active_fd(fd_set readfds, int nfds, const vector<int> &fd_vector) {
  FD_ZERO(&readfds);
  for (auto fd : fd_vector) {
    FD_SET(fd, &readfds);
  }
  int active_fd_num = select(nfds + 1, &readfds, NULL, NULL, NULL);
  // if (active_fd_num != 1) {
  //   cout << "weird, only 1 fd could be active" << endl;
  // }
  // cout << "select succeed" << endl;
  for (auto fd : fd_vector) {
    if (FD_ISSET(fd, &readfds)) {
      return fd;
    }
  }
  return 0;
}

string recv_trace(int fd) {
  int buff_size = recv_int(fd);
  char *buff = (char *)malloc(buff_size * sizeof(*buff));
  recv_all(fd, buff, buff_size);
  string trace = string(buff);
  free(buff);
  return trace;
}

void send_trace(int fd, string curr_trace, int player_id) {
  string new_trace = curr_trace.append(",").append(to_string(player_id));
  const char *buff = new_trace.c_str();
  int strlen_with_null_terminator = strlen(buff) + 1;
  send_int(fd, strlen_with_null_terminator);
  send_all(fd, buff, strlen_with_null_terminator);
}

int send_all(int fd, const char *buf, int len) {
  int total = 0;       // how many bytes we've sent
  int bytesleft = len; // how many we have left to send
  int n;

  while (total < len) {
    n = send(fd, buf + total, bytesleft, 0);
    if (n == -1) {
      break;
    }
    total += n;
    bytesleft -= n;
  }

  return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}
int send_all(int fd, char *buf, int len) {
  int total = 0;       // how many bytes we've sent
  int bytesleft = len; // how many we have left to send
  int n;

  while (total < len) {
    n = send(fd, buf + total, bytesleft, 0);
    if (n == -1) {
      break;
    }
    total += n;
    bytesleft -= n;
  }

  return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}
int recv_all(int fd, char *buf, int len) {
  return recv(fd, buf, len, MSG_WAITALL);
}
int recv_all(int fd, const char *buf, int len) {
  return recv(fd, const_cast<char *>(buf), len, MSG_WAITALL);
}
int send_int(int fd, int data_to_send) {
  uint32_t network_byte_order = htonl(data_to_send);
  return send_all(fd, reinterpret_cast<char *>(&network_byte_order),
                  sizeof(network_byte_order));
}
int recv_int(int fd) {
  uint32_t network_byte_order;
  if (0 == recv_all(fd, reinterpret_cast<char *>(&network_byte_order),
                    sizeof(network_byte_order))) {
    return -1;
  } else {
    return ntohl(network_byte_order);
  }
}
