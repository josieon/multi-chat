#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <pthread.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE	1024	//버퍼 사이즈
#define NAME_SIZE	20	//사용자명 사이즈
#define PORT_NUM	3500	//포트번호

void *send_msg(void *sock_num);	//메시지를 받는 메소드
void *recv_msg(void *sock_num);	//메시지를 보내는 메소드

//전역변수
char name[NAME_SIZE] = "[DEFAULT]";	//사용자명
char msg[BUF_SIZE];			//메시지 버퍼

int main(int argc, char **argv)
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;	//멀티쓰레드 선언

	//사용법
	if(argc != 4)
	{
		printf("usage : %s <IP> <NAME>\n", argv[0]);
		return -1;
	}

	sprintf(name, "%s", argv[2]);	//이름 변수 초기화

	//소켓 할당 및 구조체 초기화
	sock = socket(AF_INET, SOCK_STREAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[3]));
	//serv_addr.sin_port = htons(PORT_NUM);

	if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("connect");
		return -1;
	}

	//사용자명 보내고, 환영 메시지 출력
	write(sock, name, strlen(name));
	read(sock, msg, BUF_SIZE);
	write(1, msg, strlen(msg));

	//쓰레드 할당
	pthread_create(&snd_thread, NULL, send_msg, (void *)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void *)&sock);
	pthread_join(snd_thread, NULL);
	pthread_join(rcv_thread, NULL);
	close(sock);
	return 0;
}

//메시지를 보내는 쓰레드
void *send_msg(void *sock_num)
{
	int sock = *((int *)sock_num);
	while(true)
	{
		bzero(msg, BUF_SIZE);	//버퍼 초기화
		fgets(msg, BUF_SIZE, stdin);	//stdin으로부터 입력받아 버퍼에 저장
		write(sock, msg, strlen(msg));	//소켓으로 버퍼 내용 출력
	}
	return NULL;
}

//메시지를 받는 쓰레드
void *recv_msg(void *sock_num)
{
	int sock = *((int *)sock_num);
	int str_len;
	while(true)
	{
		bzero(msg, BUF_SIZE);	//버퍼 초기화

		//버퍼크기보다 큰 문자열의 경우에 대비해 버퍼사이즈-1 까지만 read하고 NULL값 추가
		str_len = read(sock, msg, sizeof(msg) - 1);
		if(str_len < 0)
			return NULL;
		msg[str_len] = 0;

		//@exit입력하여 서버로 보내면 @exit\n 를 서버로부터 받게 되고, 소켓을 닫고 종료.
		if(!strcmp(msg, "@exit\n"))
		{
			close(sock);
			exit(0);
		}
		//stdout으로 버퍼 내용 출력
		else
		{
			fputs(msg, stdout);
		}
	}
	return NULL;
}
