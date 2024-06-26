#include "Common.h"

char *SERVERIP = (char *)"127.0.0.1";
#define IDSIZE     16               // 채팅 ID(앞뒤 <> 기호 포함): 최대 길이
#define SERVERPORT 9000
#define MSGSIZE    240              // 채팅 메시지: 최대 길이
#define BUFSIZE    (IDSIZE+MSGSIZE) // 버퍼 전체 길이 = 채팅 ID + 채팅 메시지

int main(int argc, char *argv[])
{
	int retval;

	// 클라이언트 ID 설정
	const char *chatid = "Choi";

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// 데이터 통신에 사용할 변수
	char msg[MSGSIZE + 1];
	char buf[BUFSIZE + 1];
	int len;

	// 서버와 데이터 통신
	while (1) {
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
		retval = send(sock, (char *)&len, sizeof(int), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}

		// 데이터 보내기(가변 길이)
		retval = send(sock, buf, len, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}

		// 데이터 받기(고정 길이)
		retval = recv(sock, (char *)&len, sizeof(int), MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// 메시지 받기(가변 길이)
		retval = recv(sock, buf, len, MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// 받은 메시지 출력
		buf[retval] = '\0';
		printf("%s", buf);
	}

	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
