#include "utility.h"
#include "ringmaster.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <utility>
#include <string>
using namespace std;
int main(int argc, char const *argv[]) {
  if (argc < 4) {
    cout << "ringmaster <ringmaster_listen_port_num> <num_players> <num_hops>"
         << endl;
    return 1;
  }
  const char *hostname = NULL;
  const char *ringmaster_listen_port = argv[1];
  int num_players = atoi(argv[2]);
  int num_hops = atoi(argv[3]);
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;


  cout << "num_players is :" << num_players << endl;
  cout << "num_hops is :" << num_hops << endl;

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

  status = getaddrinfo(hostname, ringmaster_listen_port, &host_info,
                       &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << ringmaster_listen_port << ")" << endl;
    return -1;
  } // if

  socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << ringmaster_listen_port << ")" << endl;
    return -1;
  } // if

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << hostname << "," << ringmaster_listen_port << ")" << endl;
    return -1;
  } // if

  status = listen(socket_fd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl;
    cerr << "  (" << hostname << "," << ringmaster_listen_port << ")" << endl;
    return -1;
  } // if

  cout << "Waiting for connection on port " << ringmaster_listen_port << endl;
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  vector<int> player_fd_vector;
  vector<pair<string, string>> player_host_port_vector;
  for (size_t i = 0; i < num_players; i++) {
    int client_connection_fd;
    client_connection_fd =
        accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
      cerr << "Error: cannot accept connection on socket" << endl;
      return -1;
    }
    player_fd_vector.push_back(client_connection_fd);
    send_int(client_connection_fd, i);
    char hostname[512];
    memset(hostname, 0, sizeof(hostname));
    recv_all(client_connection_fd, hostname, sizeof(hostname));
    string hostname_str = string(hostname);
    cout << "player " << i << " hostname is: " << hostname << endl;
    char port[512];
    memset(port, 0, sizeof(port));
    recv_all(client_connection_fd, port, sizeof(port));
    string port_str = string(port);
    cout << "player " << i << " port is: " << port << endl;
    pair<string, string> player_host_port_pair(hostname_str, port_str);
    player_host_port_vector.push_back(player_host_port_pair);
  }
  
  return 0;
}
