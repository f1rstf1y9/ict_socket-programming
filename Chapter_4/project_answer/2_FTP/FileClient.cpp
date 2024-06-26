#include "Common.h"
#include <assert.h>

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define NAMESIZE   256  // 파일 이름 최대 길이 + 1

int main(int argc, char *argv[])
{
	int retval;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <IP> <FileName>\n", argv[0]);
		exit(1);
	}

	// 명령행 인수가 있으면 IP 주소로 사용
	if (argc > 1) SERVERIP = argv[1];

	// 명령행 인수로 전달된 파일 이름을 저장하되 길이를 점검한다.
	const char *filename = argv[2];
	if (strlen(filename) > (NAMESIZE - 1)) {
		fprintf(stderr, "<FileName>의 최대 길이는 %d입니다.", (NAMESIZE - 1));
		exit(1);
	}

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
	FILE *fp;
	char name[NAMESIZE];
	int filesize;
	char *buf;

	// (1) 전송할 파일을 {읽기 전용 + 이진 모드}로 연다.
	fp = fopen(filename, "rb");
	assert(fp != NULL);

	// 서버와 데이터 통신
	// (2) 파일 이름을 보낸다.
	// -- 파일 이름은 문제와 상관 없이 항상 고정 길이로 보내기로 한다.
	// -- 파일 데이터는 문제의 요구 사항에 따라 다른 방식으로 처리한다.
	sprintf(name, "%s", filename);
	retval = send(sock, name, NAMESIZE, 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		exit(1);
	}
	printf("%s 전송 시작!\n", name);

	do {
		// (3) 파일 크기를 얻는다.
		fseek(fp, 0, SEEK_END);
		filesize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		// (4) 파일 데이터를 한꺼번에 읽는다.
		buf = (char *)malloc(filesize);
		int nbytes = (int)fread(buf, 1, filesize, fp);
		assert(nbytes == filesize);

		// (5) {파일 크기 + 파일 데이터}를 보낸다.
		// -- 파일 크기는 고정 길이, 파일 데이터는 가변 길이다.
		retval = send(sock, (const char *)&filesize, sizeof(int), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		retval = send(sock, buf, filesize, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	} while (0);
	printf("%s 전송 완료! 총 %d바이트\n", name, filesize);

	// 파일과 소켓 닫기
	fclose(fp);
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
