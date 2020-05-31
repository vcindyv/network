#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#pragma comment (lib, "ws2_32")
//PROTOCAL
#define BUF_SIZE 1024

//MAP
#define MAP_X 30//��Xũ��
#define MAP_Y 30//��Yũ��


void ErrorHandling(char *message);//�������
void set_map();//�� ����
int probability(int num1, int num2, int num3);//Ȯ�����
void recv_msg(int num);//���ŵ����ͺ� num = ���Ź�ȣ
void send_msg(int opcode, int sendNum);//������ �޼��� sendNum = -1 sendAll
void send_all(int strLen);//Ŭ���̾�Ʈ ��ü���� �޼��� ����
int timer = 10;//�����ð�
int user_count = 0;
int check[4] = { 0 };//Ŭ���̾�Ʈ �˻�

 //GLOBAL
WSADATA wsaData;
SOCKET hServSock, hClntSock;
SOCKADDR_IN servAdr, clntAdr, clientaddr;
TIMEVAL timeout;
fd_set reads, cpyReads;
int adrSz, strLen, fdNum;
char recvData[BUF_SIZE];//������ ������
char sendData[BUF_SIZE];//������ ������

int ready_num = 0;//�����ο�
int ready_count = 0;//msg2���������� �ο�üũ
int map[MAP_X][MAP_Y] = { 0 };//MAP 0���� �ʱ�ȭ����
int step = 0;//0��⿭ 1���ӽ���

int main()
{
   int i;

   srand((unsigned)time(NULL));//����ǥ����
   set_map();//�� ����

   if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
      ErrorHandling("WSAStartup() error!");

   hServSock = socket(PF_INET, SOCK_STREAM, 0);
   memset(&servAdr, 0, sizeof(servAdr));
   servAdr.sin_family = AF_INET;
   servAdr.sin_addr.s_addr = inet_addr("127.0.0.1");
   servAdr.sin_port = htons(9000);

   if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
      ErrorHandling("bind() error");
   if (listen(hServSock, 5) == SOCKET_ERROR)
      ErrorHandling("listen() error");

   //���� ���� set�� ���
   FD_ZERO(&reads);
   FD_SET(hServSock, &reads);

   while (1)
   {
      if (step != 0 && ready_num == 1) {
         send_msg(14, -1);
      }
      //set ���纻 ����� (select ȣ�� �� reset �ǹǷ�)
      cpyReads = reads;

      //Ÿ�̸� ����.
      timeout.tv_sec = 600;
      timeout.tv_usec = 0;

      //select �Լ� ȣ�� (���� �̺�Ʈ ���)
      fdNum = select(0, &cpyReads, 0, 0, &timeout);
      if (fdNum == 0) continue; //timeout�϶�

                          //select�� ��ȯ�� �̺�Ʈ�� ó��...
      for (i = 0; i<reads.fd_count; i++)
      {
         if (FD_ISSET(reads.fd_array[i], &cpyReads))
         {
            if (reads.fd_array[i] == hServSock)
            {//connection request
               if (step == 0) {
                  adrSz = sizeof(clntAdr);
                  hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &adrSz);
                  FD_SET(hClntSock, &reads);
               }
            }
            else// read message
            {
               //data ���� (client send ó��)
               strLen = recv(reads.fd_array[i], recvData, BUF_SIZE - 1, 0);
               if (strLen < 0)    // ��밡 ������ ������ �� 
               {
                  if (step == 0) {//��⿭
                     ready_num--;//����ο� ����
                     FD_CLR(reads.fd_array[i], &reads);
                     send_msg(1, -1);
                     continue;
                  }
                  if (step == 1) {//������(���,�α׾ƿ�)
                     FD_CLR(reads.fd_array[i], &reads);
                     ready_num--;
                     send_msg(11, -1);
                     continue;
                  }
               }
               else
               {
                  recv_msg(i);//���ŵ����ͺ�
               }
            }
         }
      }
   }
   closesocket(hServSock);
   WSACleanup();
   return 0;
}

void ErrorHandling(char *message) {//�������
   fputs(message, stderr);
   fputc('\n', stderr);
   exit(1);
}

void recv_msg(int num) {//���ŵ����ͺ� num = ���Ź�ȣ
   int i;

   switch (recvData[0]) {
   case 1://��⿭ ���ӽ���
         /*
         recvData[0]
         */
      ready_num++;//��⿭ �ο� ����
      send_msg(1, -1);
      break;

   case 2://���ð� ����
         /*
         recvData[0]
         */
      ready_count++;
      if (ready_count == ready_num) send_msg(2, -1);
      break;

   case 3://move
         /*
         recvData[0]
         recvData[1] ĳ����
         recvData[2] dir
         recvData[3] X
         recvData[4] Y
         */
      send_msg(3, -1);
      break;

   case 4://�Ѿ� ����
         /*
         recvData[0]
         recvData[1] �Ѿ�X
         recvData[2] �Ѿ�Y
         */
      send_msg(4, -1);
      break;

   case 5://�Ѿ� �̵�
         /*
         recvData[0]
         recvData[1] dir
         recvData[2] �Ѿ�X
         recvData[3] �Ѿ�Y
         */
      send_msg(5, -1);
      break;

   case 6://�Ѿ� �ı�
         /*
         recvData[0]
         recvData[1] dir
         recvData[2] �Ѿ�X
         recvData[3] �Ѿ�Y
         */
      send_msg(6, -1);
      break;

   case 7://������ �ı�(�ٷξտ��� �Ѿ˹߻�)
         /*
         recvData[0]
         recvData[1] ����X
         recvData[2] ����Y
         */
      send_msg(7, -1);
      break;

   case 8://�����ı�(�Ѿ� �̵�)
         /*
         recvData[0]
         recvData[1] dir
         recvData[2] �Ѿ�X
         recvData[3] �Ѿ�Y
         */
      send_msg(8, -1);
      break;

   case 9://���ڸ� �Ѿ˹߻�
         /*
         recvData[0]
         recvData[1] ü�´޾ƾ��� �ɸ���
         recvData[2] X
         recvData[3] Y
         */
      send_msg(9, -1);
      break;

   case 10://�Ѿ� ĳ�����浹
         /*
         recvData[0]
         recvData[1] dir
         recvData[2] �Ѿ�X
         recvData[3] �Ѿ�Y
         */
      send_msg(10, -1);
      break;

   case 11://�����Ű�
         /*
         recvData[0]
         recvData[1] character
         */
      user_count++;
      check[user_count] = recvData[1];
      if (user_count == ready_num) {
         send_msg(12, -1);
         user_count = 0;
         for (i = 0; i < 4; i++) check[i] = 0;//0���� �� �ʱ�ȭ
         if (ready_num == 1) {
            send_msg(14, -1);
            break;   
         }
      }
      break;

   case 13://���
         /*
         recvData[0]
         recvData[1] character
         */
      send_msg(13, -1);
      break;
   }
}

void send_msg(int opcode, int sendNum) {//������ �޼��� sendNum = -1 sendAll

   int strLen, i, count = 1;
   sendData[0] = opcode;

   switch (opcode) {
   case 1://��⿭ ������
      sendData[0] = 1;
      sendData[1] = ready_num;//�����ο�
      strLen = 2;
      break;

   case 2://���� ����
      step = 1;
      set_map();//�� ����
      strLen = MAP_X * MAP_Y + 2;

      for (i = 0; i < reads.fd_count; i++) {
         if (reads.fd_array[i] != hServSock) {
            sendData[1] = count;
            send(reads.fd_array[i], sendData, strLen, 0);
            count++;
         }
      }
      return;
      break;

   case 3://move
      sendData[1] = recvData[1];//character
      sendData[2] = recvData[2];//dir
      sendData[3] = recvData[3];//������X
      sendData[4] = recvData[4];//������Y
      strLen = 5;
      break;

   case 4://�Ѿ� ����
      sendData[1] = recvData[1];//�Ѿ�X
      sendData[2] = recvData[2];//�Ѿ�Y
      strLen = 3;
      break;

   case 5://�Ѿ� �̵�
      sendData[1] = recvData[1];//dir
      sendData[2] = recvData[2];//�Ѿ�X
      sendData[3] = recvData[3];//�Ѿ�Y
      strLen = 4;
      break;

   case 6://�Ѿ� �ı�
      sendData[1] = recvData[1];//dir
      sendData[2] = recvData[2];//�Ѿ�X
      sendData[3] = recvData[3];//�Ѿ�Y
      strLen = 4;
      break;

   case 7://������ ����
      sendData[1] = probability(50, 25, 25);//������ 0 ��ĭ 1 �߰��Ѿ� 2 ü��
      sendData[2] = recvData[1];//����X
      sendData[3] = recvData[2];//����Y
      strLen = 4;
      break;

   case 8://�����ı�(�Ѿ� �̵�)
      sendData[1] = probability(50, 25, 25);//������ 0 ��ĭ 1 �߰��Ѿ� 2 ü��
      sendData[2] = recvData[2];//�Ѿ�X
      sendData[3] = recvData[3];//�Ѿ�Y
      sendData[4] = recvData[1];//dir
      strLen = 5;
      break;

   case 9://�Ѿ˻��� �ɸ����̵�
      sendData[1] = recvData[1];//ü�´޾ƾ��� �ɸ���
      sendData[2] = recvData[2];//X
      sendData[3] = recvData[3];//Y
      strLen = 4;
      break;

   case 10://�Ѿ� ĳ�����浹
      sendData[1] = recvData[1];//dir
      sendData[2] = recvData[2];//�Ѿ�X
      sendData[3] = recvData[3];//�Ѿ�Y
      strLen = 4;
      break;

   case 11://���� Ŭ���̾�Ʈ �˻���
      strLen = 1;
      break;

   case 12://������ Ŭ���̾�Ʈ
      sendData[1] = user_count;
      for (i = 0; i < 4; i++) {
         sendData[i + 2] = check[i];
      }
      strLen = 6;
      break;

   case 13://���
      sendData[1] = recvData[1];
      strLen = 2;
      break;

   case 14://�¸�
      strLen = 1;
      break;
   }
   if (sendNum == -1) send_all(strLen);//Ŭ���̾�Ʈ ��ü���� �޼��� ����
   else send(reads.fd_array[sendNum], sendData, strLen, 0);//�ش� Ŭ���̾�Ʈ���Ը� �޼��� ����
   return;
}

void send_all(int strLen) {//Ŭ���̾�Ʈ ��ü���� �޼��� ����
   int i;
   for (i = 0; i < reads.fd_count; i++) {
      if (reads.fd_array[i] != hServSock) {
         send(reads.fd_array[i], sendData, strLen, 0);
      }
   }
}

void set_map() {//�� ����

   int i, j;

   //map ������ġ
   for (i = 0; i < MAP_Y; i++) {
      for (j = 0; j < MAP_X; j++) {
         map[j][i] = 1;
         if (i > 0 && i < MAP_Y - 1 && j > 0 && j < MAP_X - 1) map[j][i] = probability(75, 5, 20);
      }
   }

   //������ġ ����� ����
   map[1][1] = 0;
   map[1][2] = 0;
   map[1][3] = 0;
   map[2][1] = 0;
   map[2][2] = 0;
   map[3][1] = 0;

   map[MAP_X - 2][1] = 0;
   map[MAP_X - 2][2] = 0;
   map[MAP_X - 2][3] = 0;
   map[MAP_X - 3][1] = 0;
   map[MAP_X - 3][2] = 0;
   map[MAP_X - 4][1] = 0;

   map[1][MAP_Y - 2] = 0;
   map[1][MAP_Y - 3] = 0;
   map[1][MAP_Y - 4] = 0;
   map[2][MAP_Y - 2] = 0;
   map[2][MAP_Y - 3] = 0;
   map[3][MAP_Y - 2] = 0;

   map[MAP_X - 2][MAP_Y - 2] = 0;
   map[MAP_X - 2][MAP_Y - 3] = 0;
   map[MAP_X - 2][MAP_Y - 4] = 0;
   map[MAP_X - 3][MAP_Y - 2] = 0;
   map[MAP_X - 3][MAP_Y - 3] = 0;
   map[MAP_X - 4][MAP_Y - 2] = 0;

   //�ο��� ���� ĳ���� ����
   if (ready_num >= 1) map[1][1] = 13;
   if (ready_num >= 2) map[MAP_X - 2][MAP_Y - 2] = 23;
   if (ready_num >= 3) map[MAP_X - 2][1] = 33;
   if (ready_num == 4) map[1][MAP_Y - 2] = 43;

   //sendData�� map�迭 ����
   for (i = 0; i < MAP_X; i++) {
      for (j = 0; j < MAP_Y; j++) {
         sendData[i * MAP_X + j + 2] = map[i][j];
      }
   }
}

int probability(int num1, int num2, int num3) {//Ȯ�����
   int pro;
   pro = rand() % 100;
   if (pro<num1)
      return 0;
   else if (pro<num1 + num2)
      return 1;
   else
      return 2;
}