#include "EchoAssignment.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <arpa/inet.h>

// !IMPORTANT: allowed system calls.
// !DO NOT USE OTHER NETWORK SYSCALLS (send, recv, select, poll, epoll, fork
// etc.)
//  * socket
//  * bind
//  * listen
//  * accept
//  * read
//  * write
//  * close
//  * getsockname
//  * getpeername
// See below for their usage.
// https://github.com/ANLAB-KAIST/KENSv3/wiki/Misc:-External-Resources#linux-manuals

int EchoAssignment::serverMain(const char *bind_ip, int port,
                               const char *server_hello) {
  // Your server code
  // !IMPORTANT: do not use global variables and do not define/use functions
  // !IMPORTANT: for all system calls, when an error happens, your program must
  // return. e.g., if an read() call return -1, return -1 for serverMain.


  // 1. 소켓 생성
  
  int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (server_socket == -1) {
      perror("Socket creation failed");
      return -1;
  }

  // 2. 서버 주소 설정
  struct sockaddr_in server_address;
  int addrlen = sizeof(server_address);
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  server_address.sin_addr.s_addr = inet_addr(bind_ip);

  // 3. 바인딩
  if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
      perror("Binding failed");
      close(server_socket);
      return -1;
  }

  // 4. 리스닝 모드로 변경
  if (listen(server_socket, 5) < 0) {
      perror("Listening failed");
      close(server_socket);
      return -1;
  }

  

  while (1) {
      // 5. 클라이언트 연결 수락
      // printf("5. 서버가 클라이언트 연결 수락...\n");
      int client_socket = accept(server_socket, (struct sockaddr*)&server_address, (socklen_t*)&addrlen);
      // printf("5. 서버가 클라이언트 연결 수락!!!\n");
      if (client_socket == -1) {
          perror("Client connection failed");
          close(server_socket);
          return -1;
          printf("연결 오류...\n");
      }

      // 6. 데이터를 받아 처리하고 응답
      char buffer[1024];
      strcpy(buffer, "hello\n");
      int valread;
      valread = read(client_socket, buffer, sizeof(buffer));
      if (valread == -1) {
          perror("Read error");
          close(client_socket);
          continue; // Read 실패 시 다음 연결 시도
      }
      
      // Request를 처리하고 응답 작성
      char response[1024]={0};
      
      if (strcmp(buffer, "hello\n") == 0) {
          // Request: hello
          
          if (write(client_socket, server_hello, strlen(server_hello)) == -1) {
            perror("Write failed");
            close(client_socket);
          }
          printf("[server part 1] %s ... %s\n", buffer, server_hello);
      } else if (strcmp(buffer, "whoami\n") == 0) {

          if (write(client_socket, bind_ip, strlen(bind_ip)) == -1) {
            perror("Write failed");
            close(client_socket);
          }
          printf("[server part 2] %s ... %s\n", buffer, bind_ip);

      } else if (strcmp(buffer, "whoru\n") == 0) {
        
          if (write(client_socket, bind_ip, strlen(bind_ip)) == -1) {

            perror("Write failed");
            close(client_socket);
          }

          printf("[server part 3] %s ... %s\n", buffer,  bind_ip);
      } else {
          // 기타 Request: 에코
          if (write(client_socket, buffer, strlen(buffer)) == -1) {
            perror("Write failed");
            close(client_socket);
          }
          printf("[server part] %s \n", buffer);
      }
      
      // 8. 소켓 닫기
      close(client_socket);
  }

  // 9. 서버 소켓 닫기 (실제로는 무한 루프에서ㄴ 빠져나가지 않음)
  close(server_socket);
  return 0;
}

int EchoAssignment::clientMain(const char *server_ip, int port, const char *command) {

    // 1. 소켓 생성
    int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == -1) {
        perror("Socket creation failed");
        close(client_socket);
        return -1;
    }

    // 2. 서버 주소 설정
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(server_ip);

    // 3. 서버에 연결
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        close(client_socket);
        return -1;
    }

    // command = "blabla";
    // 4. 명령을 서버로 전송
    ssize_t bytes_sent = write(client_socket, command, strlen(command));
    if (bytes_sent <= 0) {
        perror("Write error");
        close(client_socket);
        return -1;
    }

    // 5. 서버로부터 응답 받기
    char buffer[1024]={0};
    ssize_t valread = read(client_socket, buffer, sizeof(buffer));
    if (valread <= 0) {
        perror("Read error");
        close(client_socket);
        return -1;
    }

    // 6. 응답 출력
    printf("[client PART] %s ... %s\n",command, buffer);

    // 7. 응답을 서버에 로깅 또는 제출
    // 서버의 IP 주소와 응답 데이터를 submitAnswer 메서드를 사용하여 로깅 또는 제출합니다.
    submitAnswer(server_ip, buffer);

    // 8. 소켓 닫기
    close(client_socket);

    return 0;
}

static void print_usage(const char *program) {
  printf("Usage: %s <mode> <ip-address> <port-number> <command/server-hello>\n"
         "Modes:\n  c: client\n  s: server\n"
         "Client commands:\n"
         "  hello : server returns <server-hello>\n"
         "  whoami: server returns <client-ip>\n"
         "  whoru : server returns <server-ip>\n"
         "  others: server echos\n"
         "Note: each command is terminated by newline character (\\n)\n"
         "Examples:\n"
         "  server: %s s 0.0.0.0 9000 hello-client\n"
         "  client: %s c 127.0.0.1 9000 whoami\n",
         program, program, program);
}

int EchoAssignment::Main(int argc, char *argv[]) {

  if (argc == 0)
    return 1;

  if (argc != 5) {
    print_usage(argv[0]);
    return 1;
  }

  int port = atoi(argv[3]);
  if (port == 0) {
    printf("Wrong port number\n");
    print_usage(argv[0]);
  }

  switch (*argv[1]) {
  case 'c':
    return clientMain(argv[2], port, argv[4]);
  case 's':
    return serverMain(argv[2], port, argv[4]);
  default:
    print_usage(argv[0]);
    return 1;
  }
}
