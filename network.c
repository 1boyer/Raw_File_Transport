int sendall(int const s, char const *const buf, int *const len)
{ 
  int total = 0; /* how many bytes we've sent */
  int bytesleft = *len; /* how many we have left to send */
  int n;

  while(total < *len) {
    n = send(s, buf+total, bytesleft, 0);
    if (n == -1) { break; }
    total += n;
    bytesleft -= n;
  }
  *len = total; /* return number actually sent here */
  return n==-1?-1:0; /* return -1 on failure, 0 on success */
}

int recvall(int const s, char *buf, int* const len){
  int total  = 0;
  int bytes_left = *len;
  int n;

  while(total < *len){
    n = recv(s, buf + total, bytes_left, 0);
    if(n == -1 || n == 0){ break; } // 0 may mean end of data stream
    total += n;
    bytes_left -= n;
  }
  *len = total;
  return n==-1?-1:0;
}

int getaddr(char* const s){
  struct addrinfo hints, *res, *current;
  struct sockaddr_in *myaddr;
  int ret;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if(getaddrinfo(s, 0, &hints, &res) == 0){
    current = res;
    while(current){
      myaddr = (struct sockaddr_in *) current->ai_addr;
      if(myaddr->sin_family == AF_INET){
        ret = myaddr->sin_addr.s_addr;
        break;
      }
      ret = -1;
      current = current->ai_next;
    }
    freeaddrinfo(res);
  }
  else{
    ret = -1;
  }

  return ret;
}

