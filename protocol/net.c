#include <stdio.h>
#include <winsock2.h>
#include <Windows.h>
#include <process.h>
#include <stdlib.h>


#define BUF_SIZE 256

HANDLE hMutex;
char recvData[BUF_SIZE] = { 0 };//수신할 데이터
char sendData[BUF_SIZE] = { 0 };//전송할 데이터

int leftPort, rightPort, nleftPort, nrightPort;
int start = 0;//0 = ready 1 = start
int print = 0;
unsigned WINAPI ThreadSendFunc(void *arg);
unsigned WINAPI ThreadRecvFunc(void *arg);
void change_itoa(int num, int start);
int change_atoi(int start);
void input_Port();
void input_Command();

int main() {

	input_Port();
	input_Command();
	return 0;
}

unsigned WINAPI ThreadSendFunc(void *arg) {
	Sleep(30000);
	WSADATA wsaData;
	SOCKET Socket;
	int strLen;
	int AdrSz, clntAdrSz;
	SOCKADDR_IN Adr, clntAdr;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	Socket = socket(PF_INET, SOCK_DGRAM, 0);//UDP

	memset(&Adr, 0, sizeof(Adr));
	Adr.sin_family = AF_INET;
	Adr.sin_addr.s_addr = inet_addr("127.0.0.1");//IP
	Adr.sin_port = htons(*((int*)arg));//PORT

	bind(Socket, (SOCKADDR *)&Adr, sizeof(Adr));
	if (start == 1) {
		change_itoa(change_atoi(0), 0);
		change_itoa(leftPort, 5);
		change_itoa(rightPort, 10);
		change_itoa(change_atoi(15) + 1, 15);
		change_itoa(change_atoi(20), 20);
		sendto(Socket, sendData, 25, 0, (SOCKADDR*)&Adr, sizeof(Adr));
	}
	return 0;
}

unsigned WINAPI ThreadRecvFunc(void *arg) {
	WSADATA wsaData, wsaData1;
	SOCKET Socket, Socket1;
	int strLen;
	int AdrSz, clntAdrSz;
	SOCKADDR_IN Adr, Adr1, clntAdr;
	HANDLE hThread1, hThread2;
	unsigned threadID1, threadID2;
	char temp[25];

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	Socket = socket(PF_INET, SOCK_DGRAM, 0);//UDP

	memset(&Adr, 0, sizeof(Adr));
	Adr.sin_family = AF_INET;
	Adr.sin_addr.s_addr = inet_addr("127.0.0.1");//IP
	Adr.sin_port = htons(*((int*)arg));//PORT

	bind(Socket, (SOCKADDR *)&Adr, sizeof(Adr));
	while (1) {
		clntAdrSz = sizeof(clntAdr);
		strLen = recvfrom(Socket, recvData, 25, 0,
			(SOCKADDR*)&clntAdr, &clntAdrSz);
		if (strLen > 0) {
			if (change_atoi(0) == leftPort || change_atoi(0) == rightPort) {
				if (print == 1) {
					if (change_atoi(20) == 0) {
						printf("%d\t  left\t            %d\n", change_atoi(5), change_atoi(15));
						printf("%d\t  left\t            %d\n", change_atoi(10), change_atoi(15));
					}
					if (change_atoi(20) == 1) {
						printf("%d\t  right\t            %d\n", change_atoi(5), change_atoi(15));
						printf("%d\t  right\t            %d\n", change_atoi(10), change_atoi(15));
					}
				}
			}
			else if (start == 1) {
				int port = change_atoi(0);
				if (change_atoi(20) == 0) {
					hThread1 = (HANDLE)_beginthreadex(NULL, 0, ThreadSendFunc, (void *)&nleftPort, 0, &threadID1);
					Sleep(90);
					hThread2 = (HANDLE)_beginthreadex(NULL, 0, ThreadSendFunc, (void *)&port, 0, &threadID2);
				}
				else if (change_atoi(20) == 1) {
					hThread1 = (HANDLE)_beginthreadex(NULL, 0, ThreadSendFunc, (void *)&nrightPort, 0, &threadID1);
					Sleep(100);
					hThread2 = (HANDLE)_beginthreadex(NULL, 0, ThreadSendFunc, (void *)&port, 0, &threadID2);
				}
			}
		}
	}

	close(Socket);
	WSACleanup();
	return 0;
}

void change_itoa(int num, int start) {
	int i = 0;
	for (i = 0; i < 4; i++) sendData[start + i] = '0';
	sendData[start + 4] = '\0';
	i = 0;
	if (num == 0) return;
	while (num > 0) {
		sendData[start + 3 - i] = num % 10 + '0';
		num /= 10;
		i++;
	}
}

int change_atoi(int start) {
	char temp[5];
	for (int i = 0; i < 5; i++) {
		temp[i] = recvData[start + i];
	}
	return atoi(temp);
}



void input_Port() {
	HANDLE hThread1, hThread2;
	unsigned threadID1, threadID2;


	printf(">My Node Configuration\n");
	printf("-Input Left Port : ");
	scanf("%d", &leftPort);
	printf("-Input Right Port : ");
	scanf("%d", &rightPort);

	printf("\n>Neighbor Node Configuration\n");
	printf("-Input Left Neighbor Port : ");
	scanf("%d", &nleftPort);
	printf("-Input Right Neighbor Port : ");
	scanf("%d", &nrightPort);

	hThread1 = (HANDLE)_beginthreadex(NULL, 0, ThreadRecvFunc, (void *)&leftPort, 0, &threadID1);
	hThread2 = (HANDLE)_beginthreadex(NULL, 0, ThreadRecvFunc, (void *)&rightPort, 0, &threadID2);

	return;
}

void input_Command() {
	WSADATA wsaData, wsaData1;
	SOCKET Socket, Socket1;
	SOCKADDR_IN Adr, Adr1;
	char c;
	while (1) {
		fgetc(stdin);
		printf("\n>Input Command(Start(S), Print(P)) : ");
		scanf("%c", &c);
		switch (c) {
		case 'S'://Start
			hMutex = CreateMutex(NULL, FALSE, NULL);
			start = 1;
			break;
		case 'P'://Print
			print = 1;
			CloseHandle(hMutex);
			WSAStartup(MAKEWORD(2, 2), &wsaData);
			WSAStartup(MAKEWORD(2, 2), &wsaData1);
			Socket = socket(PF_INET, SOCK_DGRAM, 0);//UDP
			Socket1 = socket(PF_INET, SOCK_DGRAM, 0);//UDP

			memset(&Adr, 0, sizeof(Adr));
			Adr.sin_family = AF_INET;
			Adr.sin_addr.s_addr = inet_addr("127.0.0.1");//IP
			Adr.sin_port = htons(nleftPort);//PORT
			memset(&Adr1, 0, sizeof(Adr1));
			Adr1.sin_family = AF_INET;
			Adr1.sin_addr.s_addr = inet_addr("127.0.0.1");//IP
			Adr1.sin_port = htons(nrightPort);//PORT

			connect(Socket, (SOCKADDR *)&Adr, sizeof(Adr));
			connect(Socket1, (SOCKADDR *)&Adr1, sizeof(Adr1));

			change_itoa(0, 5);
			change_itoa(0, 10);
			change_itoa(0, 15);
			change_itoa(leftPort, 0);
			change_itoa(0, 20);//0
			sendto(Socket, sendData, 25, 0, (SOCKADDR*)&Adr, sizeof(Adr));
			change_itoa(rightPort, 0);
			change_itoa(1, 20);//1
			Sleep(100);
			sendto(Socket1, sendData, 25, 0, (SOCKADDR*)&Adr1, sizeof(Adr1));

			printf("\n-Print Routing Table\n");
			printf("Port\tDirection\tDistance\n");
			printf("%d\t    - \t            0\n", leftPort);
			printf("%d\t    - \t            0\n", rightPort);
			while (1) {
				Sleep(1000);
			}

			break;
		}
	}
}