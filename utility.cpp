#include "utility.h"
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
  return recv(fd, const_cast<char*>(buf), len, MSG_WAITALL);
}
void send_int(int fd, int data_to_send) {
  uint32_t network_byte_order = htonl(data_to_send);
  send_all(fd, reinterpret_cast<char *>(&network_byte_order),
          sizeof(network_byte_order));
}
int recv_int(int fd) {
  uint32_t network_byte_order;
  recv_all(fd, reinterpret_cast<char *>(&network_byte_order), sizeof(network_byte_order));
  return ntohl(network_byte_order);
}
