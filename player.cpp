#include "player.h"
#include "utility.h"
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;
int main(int argc, char *argv[]) {
  if (argc < 3) {
    cout << "player <machine_name> <port_num>" << endl;
    return 1;
  }
  const char *ringmaster_hostname = argv[1];
  const char *ringmaster_port = argv[2];
  int ringmaster_fd =
      connect_to_ringmaster(ringmaster_hostname, ringmaster_port);
  int player_id = get_player_id(ringmaster_fd);
  int my_listen_port = listen_and_send_hostinfo(ringmaster_fd);
  while (1);
}
int listen_and_send_hostinfo(int ringmaster_fd) {
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;
  const char * my_listen_port;
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;
  for (int i = 5000; i < 6000; i++) {
    const char *try_listen_port = to_string(i).c_str();
    status = getaddrinfo(hostname, try_listen_port, &host_info, &host_info_list);
    if (status != 0) {
      cerr << "Error: cannot get address info for host" << endl;
      cerr << "  (" << hostname << "," << try_listen_port << ")" << endl;
      return -1;
    } // if

    socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1) {
      cerr << "Error: cannot create socket" << endl;
      cerr << "  (" << hostname << "," << try_listen_port << ")" << endl;
      return -1;
    } // if

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status =
        bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
      continue;
    } else {
      my_listen_port = try_listen_port;
      break;
    }
  }
  status = listen(socket_fd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl;
    cerr << "  (" << hostname << "," << my_listen_port << ")" << endl;
    return -1;
  }
  cout << "Waiting for connection on port " << my_listen_port << endl;
  char buff[512];
  memset(buff, 0, sizeof(buff));
  status = gethostname(buff, sizeof(buff));
  send_all(ringmaster_fd, buff, sizeof(buff));
  cout << "my hostname is: " << buff << endl;
  memset(buff, 0, sizeof(buff));
  send_all(ringmaster_fd, my_listen_port, sizeof(buff));
  cout << "my port is: " << my_listen_port << endl;
  return socket_fd;
}

int connect_to_ringmaster(const char *ringmaster_hostname,
                          const char *ringmaster_port) {
  int status;
  int ringmaster_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(ringmaster_hostname, ringmaster_port, &host_info,
                       &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << ringmaster_hostname << "," << ringmaster_port << ")"
         << endl;
    return -1;
  } // if

  ringmaster_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                         host_info_list->ai_protocol);
  if (ringmaster_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << ringmaster_hostname << "," << ringmaster_port << ")"
         << endl;
    return -1;
  } // if

  cout << "Connecting to " << ringmaster_hostname << " on port "
       << ringmaster_port << "..." << endl;

  status = connect(ringmaster_fd, host_info_list->ai_addr,
                   host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << ringmaster_hostname << "," << ringmaster_port << ")"
         << endl;
    return -1;
  } // if
  return ringmaster_fd;
}

int get_player_id(int ringmaster_fd) {
  int my_player_id = recv_int(ringmaster_fd);
  cout << "my_player_id is : " << my_player_id << endl;
  return my_player_id;
}
