#pragma comment(lib,"ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h> 
#include <winsock2.h> 
#include <time.h>
#include <stdlib.h>
#include <string.h>


#define MAX_LINE 1024 
#define BACKLOG 5
#define PORT 5001
char full[5] = "full";
char empty[6] = "empty";
int price;
char price2[10];

void cal_price(int time) {
	if (time <= 5) {
		price = 1000; return;
	}
	else {
		price = 1000;
		price += (time - 5) * 100;
		return;
	}
}

int main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET listen_fd, accept_fd, max_fd = 0, sock_fd;
	struct sockaddr_in listen_addr, accept_addr;
	char buf[MAX_LINE];
	
	clock_t start[200], end;		// car 주차된 시간 저장용( id: socket fd)
	int parking_time;
	int cnt = 2;
	

	int readn, addr_len;
	unsigned int i, fd_num = 0;
	
	//file 입출력을 위한 변수
	FILE *f1;	FILE *f2;	char out[100];
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	f1 = fopen("log_space1.txt", "w");
	f2 = fopen("log_space2.txt", "w");

	fd_set old_fds, new_fds;
	/*
	if (argc != 2)
	{
		printf("Usage : %s [PORT]\n", argv[1]);
		return 1;
	}
	*/
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 1;

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd == INVALID_SOCKET)
		return 1;

	memset((void *)&listen_addr, 0x00, sizeof(listen_addr));

	listen_addr.sin_family = AF_INET;
	listen_addr.sin_port = htons(PORT); //atoi(argv[1])
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listen_fd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) == SOCKET_ERROR)
		return 1;

	if (listen(listen_fd, BACKLOG) == SOCKET_ERROR)
		return 1;

	FD_ZERO(&new_fds);
	FD_SET(listen_fd, &new_fds);

	while (1)
	{
		int testi = 0;
		old_fds = new_fds;

		printf("accept wait %d\n", new_fds.fd_count);
		for (i = 0; i <= 10; i++)
		{
			printf("[%d] = %d \n", i, new_fds.fd_array[i]);
		}

		fd_num = select(0, &old_fds, NULL, NULL, NULL);
		if (FD_ISSET(listen_fd, &old_fds))
		{
			addr_len = sizeof(struct sockaddr_in);
			accept_fd = accept(listen_fd, (struct sockaddr *)&accept_addr, &addr_len);
			if (accept_fd == INVALID_SOCKET)
			{
				continue;
			}
			FD_SET(accept_fd, &new_fds);
		}
		int in_id=0;
		int out_id=0;
		for (i = 1; i <= new_fds.fd_count; i++)
		{
			sock_fd = new_fds.fd_array[i];
			if (FD_ISSET(sock_fd, &old_fds))
			{
				memset(buf, 0x00, MAX_LINE);
				readn = recv(sock_fd, buf, MAX_LINE, 0);
				if (readn <= 0)
				{
					printf("socket close\n");
					closesocket(sock_fd);
					FD_CLR(sock_fd, &new_fds);
				}
				else
				{
					printf("%s %d\n", buf, cnt);
					if (buf[0] == 'I') { 
					//if(readn==2){
						cnt--;
						printf("cnt is    %d and sock fd is    %d\n", cnt, sock_fd);
						start[sock_fd] = clock();
						printf("in time is %.3lf 초 \n", start[sock_fd] / (double)1000);
						/*
						if(cnt == 0) {
							for (i = 1; i <= new_fds.fd_count; i++) {
								send(new_fds.fd_array[i], full, 5, 0);
							}
						}
						*/
						/*	if (cnt == 0) {
							send(sock_fd, "full", 4, 0);
						} */
						char send_in[5];
						if (sock_fd == 140) in_id = 1;
						else in_id = 2;
						
						sprintf(send_in, "In %d", in_id);


						for (i = 1; i <= new_fds.fd_count; i++) {
							char in[3] = "In";
							//send(new_fds.fd_array[i], in, 3, 0);
							send(new_fds.fd_array[i], send_in, 5, 0);
						}
					}
					else if(buf[0] == 'O') {
						cnt++;
						printf("cnt is    %d\n", cnt);
						end = clock();
						parking_time = (int)((end - start[sock_fd]) / (double)1000);
						cal_price(parking_time);
						printf("%d 초 %d 원\n", parking_time, price);
						char out[4] = "Out";
						char send_out[5];
						if (sock_fd == 140) out_id = 1;
						else out_id = 2;
						
						//sprintf(send_out, "")
						for (i = 1; i <= new_fds.fd_count; i++) {
							//send(new_fds.fd_array[i], empty, 6, 0);
							//sprintf(price2, "%d", price);
							sprintf(price2, "%d %d", out_id, price);
							send(new_fds.fd_array[i], price2, 8, 0);
						}


						//send(132, price2, 8, 0);
							
					}
					else if (buf[0] == 'P') {
						t = time(NULL);
						tm = *localtime(&t);
						sprintf(out, "%d-%d-%d %d:%d:%d  %d원\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, price);
						printf("out id!!!!!!!!! %d\n", out_id);
						if (buf[5] == '1') { fprintf(f1, out); printf("write in f1\n"); }
						else { fprintf(f2, out); printf("wirte in f2\n"); }
						//if (out_id == 2) { fprintf(f2, out); printf("wirte in f2\n"); }
						//fprintf(f1, out);
					}
					
				}
				if (--fd_num <= 0) break;
			}
		}
	}
	fclose(f1);	fclose(f2);
	closesocket(listen_fd);
	WSACleanup();
	return 0;
}