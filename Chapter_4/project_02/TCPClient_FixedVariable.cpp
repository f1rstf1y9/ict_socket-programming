#include "Common.h"

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    50

int main(int argc, char *argv[])
{
	int retval;

	// 명령행 인수가 있으면 IP 주소로 사용
	if (argc > 1) SERVERIP = argv[1];

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
  char fileName[BUFSIZE];
	char buf[BUFSIZE];
	int len;

	// 서버와 데이터 통신
	for (int i = 0; i < 4; i++) {
		
    // 데이터 입력(시뮬레이션)
		printf("\n[Input a file name] ");
		if (fgets(fileName, BUFSIZE + 1, stdin) == NULL)
			break;

    // '\n' 문자 제거
    len = (int)strlen(fileName);
		if (fileName[len - 1] == '\n')
			fileName[len - 1] = '\0';
		if (strlen(fileName) == 0)
			break;

    // 파일 열기
    FILE *fp = fopen(fileName, "rb");
    if (fp == NULL) {
      printf("[TCP Client] File not found.\n");
      break;
    }

    len = fread(buf, 1, BUFSIZE, fp);
    fclose(fp);

    if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		if (strlen(buf) == 0)
			break;

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
		printf("[TCP Client] Send %d+%d bytes.\n", (int)sizeof(int), retval);
	}

	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}