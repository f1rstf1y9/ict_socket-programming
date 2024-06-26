#include "Common.h"

#define SERVERPORT 9000
#define BUFSIZE    512

int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0); // IPv4, TCP
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr; // 소켓 구조체 생성
	memset(&serveraddr, 0, sizeof(serveraddr)); // 메모리 초기화
	serveraddr.sin_family = AF_INET; // 어떤 프로토콜을 사용할 것인지
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); // IP 주소 정보, htonl() : host to network long, 호스트 바이트 형태를 네트워크 바이트 형태로 변환
	serveraddr.sin_port = htons(SERVERPORT); // 포트 번호, htons() : host to network short
	retval = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN); // 입력에 대한 대기
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr; // 구조체 생성
	int addrlen;
	char buf[BUFSIZE + 1]; // 데이터를 주고받을 버퍼에 대한 변수

	while (1) { // 계속 무한루프를 돌면서 accept에 대한 대기
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen); // 클라이언트에 대한 주소 등 정보를 받아옴
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP Server] Client Connection: IP Address=%s, Port Number=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// 클라이언트와 데이터 통신
		while (1) {
			// 데이터 받기
			retval = recv(client_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// 받은 데이터 출력
			buf[retval] = '\0'; // 입력된 string의 끝에 문장의 종료를 의미하는 NULL값을 넣어줌
			printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);

			// 데이터 보내기

      // 입력받은 문자열을 소문자로 바꿔서 보냄
      for (int i = 0; i < retval; i++) {
        buf[i] = toupper(buf[i]);
      }
			retval = send(client_sock, buf, retval, 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
		}

		// 소켓 닫기
		closesocket(client_sock);
		printf("[TCP Server] Client Disconnection: IP Address=%s, Port Number=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}