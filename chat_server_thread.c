#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <pthread.h>

#define BUF_SIZE	1024	//버퍼 사이즈
#define MAX_CLNT	30	//사용자수 상한
#define NAME_SIZE	20	//사용자명 사이즈
#define PORT_NUM	3500	//포트 번호

void *handle_clnt(void *arg);	//클라이언트 처리 메소드 - 멀티쓰레드처리
void send_msg(char *msg, int len, int sender);	//문자열 전송 메소드
void print_online_user(int clnt_sock);		//접속 중인 사용자 출력
void print_online_message(int index);		//다른 사용자들에게 접속 메시지 출력
void print_offline_message(int index);		//다른 사용자들에게 접속종료 메시지 출력
void make_offline(int index);		//사용자 리스트 초기화
void whisper(int index);		//귓속말 메소드
int find_index(int socket);		//소켓주소로 사용자index를 찾는 메소드

//전역변수 - mutex사용
int clnt_cnt = 0;	//사용자 카운트
pthread_mutex_t mtx;	//mutex변수

//사용자 구조체
struct member
{
	int sock_fd;	//클라이언트 소켓 파일 디스크립터
	char name[NAME_SIZE];	//사용자명
};
struct member mem[MAX_CLNT];
char msg[BUF_SIZE];		//메시지 버퍼

int main(int argc, char ** argv)
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_addr, clnt_addr;
	socklen_t clnt_addr_size;
	pthread_t tid;
	
	//사용법
	if(argc != 2)
	{
		printf("usage : %s\n", argv[0]);
		return -1;
	}

	pthread_mutex_init(&mtx, NULL);

	//소켓생성 및 구조체 초기화
	serv_sock = socket(AF_INET, SOCK_STREAM, 0);
	//소켓생성 오류
	if(serv_sock < 0)
	{
		perror("socket");
		return -1;
	}
	//서버 소켓 할당
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));
	//serv_addr.sin_port = htons(PORT_NUM);

	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("bind");
		return -1;
	}
	//연결 대기
	if(listen(serv_sock, 5) < 0)
	{
		perror("listen");
		return -1;
	}

	while(true)
	{
		//연결
		clnt_addr_size = sizeof(clnt_addr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
		if(clnt_sock < 0)
		{
			perror("accept");
			return -1;
		}

		//사용자명 read 및 member구조체 초기화
		pthread_mutex_lock(&mtx);
		read(clnt_sock, mem[clnt_cnt].name, NAME_SIZE);
		mem[clnt_cnt++].sock_fd = clnt_sock;
		pthread_mutex_unlock(&mtx);

		//환영 메시지 출력
		sprintf(msg, "%s님 채팅방에 오신 것을 환영합니다!\n", mem[clnt_cnt - 1].name);
		write(clnt_sock, msg, strlen(msg));
		print_online_user(clnt_sock);

		//쓰레드 detach
		pthread_create(&tid, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(tid);
		printf("클라이언트 IP : %s 연결\n", inet_ntoa(clnt_addr.sin_addr));
	}
	close(serv_sock);
	return 0;
}

void *handle_clnt(void *arg)
{
	int clnt_sock = *((int*)arg);
	int index;	//현재 연결된 클라이언트 소켓 인덱스

	//사용자index 찾는 메소드
	index = find_index(clnt_sock);

	//다른 사용자들에게 접속 메시지 출력
	print_online_message(index);

	//클라이언트에서 메시지를 받아 처리하는 반복문
	while(read(clnt_sock, msg, sizeof(msg)))
	{
		//index변수값 초기화
		index = find_index(clnt_sock);

		//@exit
		if( !strcmp(msg, "@exit\n"))
		{
			print_offline_message(index);	//다른 사용자들에게 종료 메시지 출력
			make_offline(index);	//사용자 구조체 초기화
			return NULL;
		}
		//@show
		else if( !strcmp(msg, "@show\n"))
			print_online_user(clnt_sock);	//접속 중인 사용자 출력
		//귓속말
		else if( strncmp(msg, "@", 1) == 0)
			whisper(index);
		//채팅
		else
		{
			send_msg(mem[index].name, strlen(mem[index].name), clnt_sock);
			send_msg(": ", 2, clnt_sock);
			send_msg(msg, strlen(msg), clnt_sock);
		}
		bzero(msg, BUF_SIZE);	//버퍼 초기화
	}
	return NULL;
}

//클라이언트로 버퍼 내용 출력
void send_msg(char *msg, int len, int sender)
{
	for(int i=0; i<clnt_cnt; i++)
	{
		//sender로 입력받은 소켓을 제외하고 출력. sender는 송신자
		if(sender == mem[i].sock_fd)
			continue;
		write(mem[i].sock_fd, msg, len);
	}
}

//접속중인 사용자 출력
void print_online_user(int clnt_sock)
{
	bzero(msg, BUF_SIZE);

	//접속중인 사용자가 혼자일때
	if(clnt_cnt == 1)
		strcat(msg, "접속중인 다른 사용자가 없습니다.\n");
	else
	{
		strcat(msg, "현재 서버에는 ");
		//본인을 제외하고 문자열 추가
		for(int i=0; i < clnt_cnt; i++)
		{
			if(mem[i].sock_fd == clnt_sock)
				continue;
			strcat(msg, mem[i].name);
			strcat(msg, "님 ");
		}
		strcat(msg, "이 접속중입니다.\n");
	}
	write(clnt_sock, msg, strlen(msg));
	bzero(msg, BUF_SIZE);
}

//사용자가 접속하면 다른 사용자들에게 알림
void print_online_message(int index)
{
	sprintf(msg, "%s님이 입장하셨습니다.\n", mem[index].name);
	for(int i=0; i < clnt_cnt; i++)
	{
		//접속한 당사자는 제외
		if(i == index)
			continue;
		write(mem[i].sock_fd, msg, strlen(msg));
	}
	bzero(msg, BUF_SIZE);
}

//사용자가 접속종료하면 다른 사용자들에게 알림
void print_offline_message(int index)
{
	sprintf(msg, "%s님이 퇴장하셨습니다.\n", mem[index].name);
	for(int i=0; i < clnt_cnt; i++)
	{
		//접속종료하는 당사자는 제외
		if(i == index)
			continue;
		write(mem[i].sock_fd, msg, strlen(msg));
	}
	bzero(msg, BUF_SIZE);
}

//접속종료
void make_offline(int index)
{
	//접속종료하는 클라이언트로 메시지 출력 후 소켓 종료
	if(index >= 0)
	{
		write(mem[index].sock_fd, "@exit\n", 6);
		close(mem[index].sock_fd);
	}
	pthread_mutex_lock(&mtx);
	//구조체 내용 및 전역변수 초기화
	for(int i = index + 1; i < clnt_cnt; i++)
	{
		mem[i-1].sock_fd = mem[i].sock_fd;
		strcpy(mem[i-1].name, mem[i].name);
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mtx);
	bzero(msg, BUF_SIZE);
}

void whisper(int index)
{
	//귓속말 보낸 사용자명 분리
	char buf[BUF_SIZE];
	strcpy(buf, msg);
	char* ptr = strtok(buf, " ");
	int len = strlen(ptr);
	strncpy(ptr, ptr + 1, len - 1);
	ptr[len - 1] = '\0';

	//수신자 찾아서 내용 보내기
	for(int i=0; i < clnt_cnt; i++)
	{
		//수신자와 동일한 이름의 사용자 찾은 경우
		if( !strcmp(ptr, mem[i].name))
		{
			bzero(buf, sizeof(buf));
			sprintf(buf, "귓속말 %s: ",mem[index].name);
			ptr = strtok(msg, " ");
			ptr = strtok(NULL, " ");
			while(ptr != NULL)
			{
				strcat(buf, ptr);
				ptr = strtok(NULL, " ");
				if(ptr != NULL)
					strcat(buf, " ");
			}
			write(mem[i].sock_fd, buf, strlen(buf));
			break;
		}
		//수신자와 동일한 이름의 사용자가 없는 경우
		else if( i == clnt_cnt - 1)
		{
			bzero(buf, sizeof(buf));
			strcat(buf, "상대방이 없습니다.\n");
			write(mem[index].sock_fd, buf, strlen(buf));
		}
	}
}

//소켓주소로 사용자index를 찾는 함수
int find_index(int socket)
{
	for(int i=0; i < clnt_cnt; i++)
	{
		if(mem[i].sock_fd == socket)
			return i;
	}
	return -1;
}
