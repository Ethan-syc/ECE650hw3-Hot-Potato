#include "ringmaster.h"
#include "utility.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>
#include <vector>
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
  cout << "num_players is :" << num_players << endl;
  cout << "num_hops is :" << num_hops << endl;
  if (num_players <= 1 || num_hops < 0 || num_hops > 512) {
    cout << "num_players must be greater than 1 and num_hops must be greater "
            "than or equal to zero "
            " and less than or "
            "equal to 512 "
         << endl;
  }
  int socket_fd = listen_to_port(hostname, ringmaster_listen_port);
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
    pair<string, string> player_host_port_pair{hostname_str, port_str};
    player_host_port_vector.push_back(player_host_port_pair);
  }
  close(socket_fd);
  send_player_info_to_neighbor(player_fd_vector, player_host_port_vector);
  int start_player_id = start_game(player_fd_vector, num_players, num_hops);
  cout << "start player id is " << start_player_id << endl;
  int nfds = *max_element(player_fd_vector.begin(), player_fd_vector.end());
  fd_set readfds;
  int active_fd = select_active_fd(readfds, nfds, player_fd_vector);
  string trace = recv_trace(active_fd);
  cout << "Trace of potato:" << endl << trace.substr(1) << endl;
  return 0;
}

int start_game(const vector<int> &player_fd_vector, const int num_players,
               const int num_hops) {
  srand((unsigned int)time(NULL));
  int random = rand() % num_players;
  send_int(player_fd_vector[random], 0); // control-sig, 0 menas game starts
  send_int(player_fd_vector[random], num_hops);
}

void send_player_info_to_neighbor(
    vector<int> player_fd_vector,
    vector<pair<string, string>> player_host_port_vector) {
  for (size_t i = 1; i < player_fd_vector.size(); i++) {
    char hostname[512];
    strncpy(hostname, player_host_port_vector[i].first.c_str(),
            sizeof(hostname));
    char port[512];
    strncpy(port, player_host_port_vector[i].second.c_str(), sizeof(port));
    cout << hostname << " " << port << endl;
    send_all(player_fd_vector[i - 1], hostname, sizeof(hostname));
    send_all(player_fd_vector[i - 1], port, sizeof(port));
    send_int(player_fd_vector[i - 1], player_fd_vector.size());
  }
  char hostname[512];
  strncpy(hostname, player_host_port_vector[0].first.c_str(), sizeof(hostname));
  char port[512];
  strncpy(port, player_host_port_vector[0].second.c_str(), sizeof(port));
  send_all(player_fd_vector.back(), hostname, sizeof(hostname));
  send_all(player_fd_vector.back(), port, sizeof(port));
  send_int(player_fd_vector.back(), player_fd_vector.size());
}

int listen_to_port(const char *hostname, const char *ringmaster_listen_port) {
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;

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
  }
  cout << "Waiting for connection on port " << ringmaster_listen_port << endl;
  freeaddrinfo(host_info_list);
  return socket_fd;
}
