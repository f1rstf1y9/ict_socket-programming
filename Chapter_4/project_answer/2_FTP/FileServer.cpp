#include "Common.h"
#include <assert.h>

#define SERVERPORT 9000
#define NAMESIZE   256  // 파일 이름 최대 길이 + 1

int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	FILE *fp;
	char name[NAMESIZE];
	int filesize;
	char *buf;

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP Server] Client Connected: IP Address=%s, Port Number=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// 클라이언트와 데이터 통신
		// (1) 파일 이름을 받는다.
		// -- 파일 이름은 문제와 상관 없이 항상 고정 길이로 받기로 한다.
		// -- 파일 데이터는 문제의 요구 사항에 따라 다른 방식으로 처리한다.
		retval = recv(client_sock, name, NAMESIZE, MSG_WAITALL);
		if (retval == 0 || retval == SOCKET_ERROR) {
			// 소켓 닫기
			closesocket(client_sock);
			printf("[TCP Server] Client Terminated: IP Address=%s, Port Number=%d\n",
				addr, ntohs(clientaddr.sin_port));
			continue;
		}
		printf("%s Receive Start!\n", name);

		// (2) 저장할 파일을 {쓰기 전용 + 이진 모드}로 연다.
		fp = fopen(name, "wb");
		assert(fp != NULL);

		do {
			// (3) 파일 크기(고정 길이)를 받는다.
			retval = recv(client_sock, (char *)&filesize, sizeof(int), MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// (4) 파일 데이터(가변 길이)를 받아서 저장한다.
			buf = (char *)malloc(filesize);
			retval = recv(client_sock, buf, filesize, MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			int nbytes = (int)fwrite(buf, 1, retval, fp);
			assert(nbytes == retval);
		} while (0);
		printf("%s Receive Completed! Total %d Bytes\n", name, filesize);

		// 파일과 소켓 닫기
		fclose(fp);
		closesocket(client_sock);
		printf("[TCP Server] Client Terminated: IP Address=%s, Port Number=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
