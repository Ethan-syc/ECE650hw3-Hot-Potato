#include "player.h"
#include "utility.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>
using namespace std;
int main(int argc, char *argv[]) {
  if (argc < 3) {
    cout << "player <machine_name> <port_num>" << endl;
    return 1;
  }
  const char *hostname = argv[1];
  const char *port = argv[2];
  int ringmaster_fd = connect_to_server(hostname, port);
  int player_id = get_player_id(ringmaster_fd);
  int my_listen_fd = listen_and_send_hostinfo(ringmaster_fd);
  pair<string, string> neighbor_host_port_pair =
      recv_neighbor_info(ringmaster_fd);
  int player_num = recv_int(ringmaster_fd);
  cout << "player_num is " << player_num << endl;
  pair<int, int> neighbor_fd_pair =
      form_ring(my_listen_fd, neighbor_host_port_pair, player_id);
  close(my_listen_fd);
  int left_fd = neighbor_fd_pair.first;
  int right_fd = neighbor_fd_pair.second;
  vector<int> fd_vector{left_fd, right_fd, ringmaster_fd};
  int nfds = *max_element(fd_vector.begin(), fd_vector.end());
  fd_set readfds;
  srand((unsigned int)time(NULL) + player_id);
  while (1) {
    int active_fd = select_active_fd(readfds, nfds, fd_vector);
    int control_sig = recv_int(active_fd);
    // cout << "control-sig is " << control_sig << endl;
    // game start
    if (control_sig == 0) {
      int hops = recv_int(active_fd);
      cout << "hops: " << hops << endl;
      if (hops == 0) { // end
        cout << "I'm it" << endl;
        send_trace(ringmaster_fd, "", player_id);
        break;
      } else {
        int neighbor_id =
            send_to_neighbor(hops, neighbor_fd_pair, "", player_id, player_num);
      }
    } else if (control_sig == 1) {
      int hops = recv_int(active_fd);
      cout << "hops: " << hops << endl;
      string curr_trace = recv_trace(active_fd);
      if (hops == 0) { // end
        cout << "I'm it" << endl;
        send_trace(ringmaster_fd, curr_trace, player_id);
        break;
      } else {
        int neighbor_id =
            send_to_neighbor(hops, neighbor_fd_pair, curr_trace, player_id, player_num);
      }
    } else if (control_sig == -1) { // exit
      break;
    }
  }
  for (auto fd : fd_vector) {
    close(fd);
  }
  return 0;
}

int send_to_neighbor(int hops, pair<int, int> neighbor_fd_pair,
                     string curr_trace, int player_id, int player_num) {
  int rand_fd;
  int neighbor_player_id;
  if (rand() % 2 == 0) {
    rand_fd = neighbor_fd_pair.first;
    neighbor_player_id = player_id - 1;
    if (neighbor_player_id < 0) {
      neighbor_player_id += player_num;
    }
  } else {
    rand_fd = neighbor_fd_pair.second;
    neighbor_player_id = player_id + 1;
    if (neighbor_player_id == player_num) {
      neighbor_player_id = 0;
    }
  }
  cout << "Sending potato to " << neighbor_player_id << endl;
  int status  = send_int(rand_fd, 1); // normal_sig
  // cout << "send sig status " << status << endl;
  status  = send_int(rand_fd, hops-1);
  // cout << "send size status " << status << endl;
  send_trace(rand_fd, curr_trace, player_id);
}
pair<string, string> recv_neighbor_info(int ringmaster_fd) {
  char hostname[512];
  char port[512];
  recv_all(ringmaster_fd, hostname, sizeof(hostname));
  recv_all(ringmaster_fd, port, sizeof(port));
  cout << "my neighbor is " << hostname << " on port " << port << endl;
  return make_pair(string(hostname), string(port));
}

pair<int, int> recv_neighbor_player_id(int ringmaster_fd) {
  int left_player_id = recv_int(ringmaster_fd);
  int right_player_id = recv_int(ringmaster_fd);
  cout << "my left neighbor's id is " << left_player_id << endl;
  cout << "my right neighbor's id is " << right_player_id << endl;
  return make_pair(left_player_id, right_player_id);
}

int accept_from_client(int socket_fd) {
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int new_fd =
      accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (new_fd == -1) {
    cerr << "Error: cannot accept connection on socket" << endl;
  }
  // cout << "accept succeed" << endl;
  return new_fd;
}

pair<int, int> form_ring(int my_listen_fd,
                         pair<string, string> neighbor_host_port_pair,
                         int player_id) {
  int status;
  int right_fd;
  int left_fd;
  if (player_id == 0) {
    right_fd = connect_to_server(neighbor_host_port_pair.first.c_str(),
                                 neighbor_host_port_pair.second.c_str());
    left_fd = accept_from_client(my_listen_fd);
  } else {
    left_fd = accept_from_client(my_listen_fd);
    right_fd = connect_to_server(neighbor_host_port_pair.first.c_str(),
                                 neighbor_host_port_pair.second.c_str());
  }
  return make_pair(left_fd, right_fd);
}

int listen_and_send_hostinfo(int ringmaster_fd) {
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  const char *hostname = NULL;
  char my_listen_port[512];
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;
  for (int i = 5000; i < 6000; i++) {
    const char *try_listen_port = to_string(i).c_str();
    status =
        getaddrinfo(hostname, try_listen_port, &host_info, &host_info_list);
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
      freeaddrinfo(host_info_list);
      continue;
    } else {
      strncpy(my_listen_port, try_listen_port, sizeof(my_listen_port));
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
  send_all(ringmaster_fd, my_listen_port, sizeof(my_listen_port));
  cout << "my port is: " << my_listen_port << endl;
  freeaddrinfo(host_info_list);
  return socket_fd;
}

int connect_to_server(const char *hostname, const char *port) {
  int status;
  int ringmaster_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } // if

  ringmaster_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                         host_info_list->ai_protocol);
  if (ringmaster_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  } // if

  cout << "Connecting to " << hostname << " on port " << port << "..." << endl;
  while (1) {
    status = connect(ringmaster_fd, host_info_list->ai_addr,
                     host_info_list->ai_addrlen);
    if (status == -1) {
      cerr << "Error: cannot connect to socket" << endl;
      cerr << "  (" << hostname << "," << port << ")" << endl;
      continue;
    } else {
      break;
    }
  }
  // cout << "connect succedd" << endl;
  freeaddrinfo(host_info_list);
  return ringmaster_fd;
}

int get_player_id(int ringmaster_fd) {
  int my_player_id = recv_int(ringmaster_fd);
  cout << "my_player_id is : " << my_player_id << endl;
  return my_player_id;
}
