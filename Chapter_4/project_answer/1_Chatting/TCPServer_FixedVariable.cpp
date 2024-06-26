#include "Common.h"

#define SERVERPORT 9000
#define IDSIZE     16               // 채팅 ID(앞뒤 <> 기호 포함): 최대 길이
#define MSGSIZE    496              // 채팅 메시지: 최대 길이
#define BUFSIZE    (IDSIZE+MSGSIZE) // 버퍼 전체 길이 = 채팅 ID

int main(int argc, char *argv[])
{
	int retval;

	// 서버 ID 설정
	const char *chatid = "Server";

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
	int len; // 고정 길이 데이터
	char msg[MSGSIZE + 1]; // 입력 메시지 저장
	char buf[BUFSIZE + 1]; // 가변 길이 데이터

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
		while (1) {
			// 데이터 받기(고정 길이)
			retval = recv(client_sock, (char *)&len, sizeof(int), MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// 메시지 받기(가변 길이)
			retval = recv(client_sock, buf, len, MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// 받은 메시지 출력
			buf[retval] = '\0';
			printf("%s", buf);

			// 메시지 입력
			printf("<%s> ", chatid);
			if (fgets(msg, MSGSIZE + 1, stdin) == NULL)
				break;

			// '\n' 문자 제거
			len = (int)strlen(msg);
			if (msg[len - 1] == '\n')
				msg[len - 1] = '\0';
			if (strlen(msg) == 0)
				break;

			// 메시지 준비하기
			sprintf(buf, "<%s> %s\n", chatid, msg);
			len = (int)strlen(buf);

			// 데이터 보내기(고정 길이)
			retval = send(client_sock, (char *)&len, sizeof(int), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}

			// 데이터 보내기(가변 길이)
			retval = send(client_sock, buf, len, 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
		}

		// 소켓 닫기
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
