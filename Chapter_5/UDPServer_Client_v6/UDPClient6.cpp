#include "Common.h"

char *SERVERIP = (char *)"::1";
#define SERVERPORT 9000
#define BUFSIZE    512

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
	SOCKET sock = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// 소켓 주소 구조체 초기화
	struct sockaddr_in6 serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin6_family = AF_INET6;
	inet_pton(AF_INET6, SERVERIP, &serveraddr.sin6_addr);
	serveraddr.sin6_port = htons(SERVERPORT);

	// 데이터 통신에 사용할 변수
	struct sockaddr_in6 peeraddr;
	int addrlen;
	char buf[BUFSIZE + 1];
	int len;

	// 서버와 데이터 통신
	while (1) {
		// 데이터 입력
		printf("\n[Sending Data] ");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
			break;

		// '\n' 문자 제거
		len = (int)strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		if (strlen(buf) == 0)
			break;

		// 데이터 보내기
		retval = sendto(sock, buf, (int)strlen(buf), 0,
			(struct sockaddr *)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) {
			err_display("sendto()");
			break;
		}
		printf("[UDP Client] %d Bytes Send\n", retval);

		// 데이터 받기
		addrlen = sizeof(peeraddr);
		retval = recvfrom(sock, buf, BUFSIZE, 0,
			(struct sockaddr *)&peeraddr, &addrlen);
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			break;
		}

		// 송신자의 주소 체크
		if (memcmp(&peeraddr, &serveraddr, sizeof(peeraddr))) {
			printf("[Error] Error Data! \n");
			break;
		}

		// 받은 데이터 출력
		buf[retval] = '\0';
		printf("[UDP Client] %d Bytes Receive\n", retval);
		printf("[Received Data] %s\n", buf);
	}

	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
