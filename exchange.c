#define BUF_SIZE 8192

const char header[] = "filename %s %d\n";
const char sendheader[] = "SEND";
const char bad_header[] = "Bad Header: Fuck off!\n";

int recv_file(int fdes, int sock, int filesize){
  char buf[BUF_SIZE];
  unsigned int total = 0, bytes_written = 0, bytes_left = 0, len;
  int n;

  while(total < filesize){
    len = BUF_SIZE;
    if(recvall(sock, buf, &len) == -1) break;
    total += len;
    bytes_left = len;
    bytes_written = 0;
    while(bytes_written < len){
      if((n = write(fdes, buf+bytes_written, bytes_left)) == -1){
        fprintf(stderr, "Unable to write to file! Errno (%d)\n", errno);
        return -1;
      }
      bytes_written += n;
      bytes_left -= n;
    }
    if(len == 0 && total < filesize){
      fprintf(stderr, "File transfer failed!\n");
      return -1;
    }
  }

  return 0;
}

int send_file(int fdes, int sock, int filesize){
  char buf[BUF_SIZE];
  unsigned int total = 0, bytes_read = 0, bytes_left = 0, len;
  int n;

  while(total < filesize){   
    if((n = read(fdes, buf, BUF_SIZE)) == -1){
      fprintf(stderr, "Unable to write to file! Errno (%d)\n", errno);
      return -1;
    }

    len = n;
    if(len && sendall(sock, buf, &len) == -1) break;
    total += len;

    if(len == 0 && total < filesize){
      fprintf(stderr, "File transfer failed!\n");
      return -1;
    }
  }

  return 0;
}

void exchange(int sock, char* filename){
  int len, fdes;
  unsigned int filesize;
  char buf[512], *ptr, *file;

  if(filename == 0){
    if((len = recv(sock, &buf, 512, 0)) == -1){
      fprintf(stderr, "Error recving data! Dying...\n");
      exit(-1);
    }
    else{
      buf[512] = 0;
      ptr = strtok(buf, " \n"); 
      if(ptr == 0 || strcmp(ptr, "filename")){
        len = sizeof(bad_header);
        sendall(sock, bad_header, &len);
        exit(-1);
      }
      ptr = strtok(0, " \n");
      file = ptr;
      if(ptr == 0 || (fdes = open(file, O_WRONLY | O_CREAT | O_EXCL | O_TRUNC , S_IRUSR | S_IWUSR)) == -1){
        fprintf(stderr, "Unable to open file %s! Dying... \n", ptr);
        len = sizeof(bad_header);
        sendall(sock, bad_header, &len);
        exit(-1);
      }
      ptr = strtok(0, " \n");
      if(ptr == 0){
        len = sizeof(bad_header);
        sendall(sock, bad_header, &len);
        unlink(file);
        exit(-1);
      }
      filesize = strtol(ptr, 0, 10);

      send(sock, sendheader, sizeof(sendheader), 0);
      printf("Downloading file [%s] (%d bytes)...\n", file, filesize);

      if(recv_file(fdes, sock, filesize) == -1){
        fprintf(stderr, "Unable to receive file! Dying...\n", ptr);
        unlink(file);
        exit(-1);
      }
      else{
        printf("Successfully transferred file!\n");
      }
    }
  }
  else{ // Send Mode
    if((fdes = open(filename, O_RDWR , S_IRWXU)) == -1){
      fprintf(stderr, "Unable to open file %s! Dying... \n", filename);
      exit(-1);
    }

    filesize = lseek(fdes, 0, SEEK_END); /* Calculate filesize */
    lseek(fdes, 0, SEEK_SET);

    snprintf(buf, 512, header, filename, filesize);
    send(sock, buf, strlen(buf), 0);
    recv(sock, buf, 512, 0);
    if(strcmp(buf, sendheader)){
      exit(-1);
    }

    printf("Uploading file [%s] (%d bytes)...\n", filename, filesize);
    if(send_file(fdes, sock, filesize) == -1){ /* Send file or Die */
      fprintf(stderr, "Unable to send file! Dying...\n", ptr);
      exit(-1);
    }
    else{
      printf("Successfully transferred file!\n");
    }
  }

  close(fdes);
}
