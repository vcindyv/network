#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#pragma comment (lib, "ws2_32")
//PROTOCAL
#define BUF_SIZE 1024

//MAP
#define MAP_X 30//맵X크기
#define MAP_Y 30//맵Y크기


void ErrorHandling(char *message);//에러출력
void set_map();//맵 설정
int probability(int num1, int num2, int num3);//확률계산
void recv_msg(int num);//수신데이터별 num = 수신번호
void send_msg(int opcode, int sendNum);//전송할 메세지 sendNum = -1 sendAll
void send_all(int strLen);//클라이언트 전체에게 메세지 전달
int timer = 10;//남은시간
int user_count = 0;
int check[4] = { 0 };//클라이언트 검색

 //GLOBAL
WSADATA wsaData;
SOCKET hServSock, hClntSock;
SOCKADDR_IN servAdr, clntAdr, clientaddr;
TIMEVAL timeout;
fd_set reads, cpyReads;
int adrSz, strLen, fdNum;
char recvData[BUF_SIZE];//수신할 데이터
char sendData[BUF_SIZE];//전송할 데이터

int ready_num = 0;//참여인원
int ready_count = 0;//msg2보내기위한 인원체크
int map[MAP_X][MAP_Y] = { 0 };//MAP 0으로 초기화설정
int step = 0;//0대기열 1게임시작

int main()
{
   int i;

   srand((unsigned)time(NULL));//난수표생성
   set_map();//맵 설정

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

   //서버 소켓 set에 등록
   FD_ZERO(&reads);
   FD_SET(hServSock, &reads);

   while (1)
   {
      if (step != 0 && ready_num == 1) {
         send_msg(14, -1);
      }
      //set 복사본 만들기 (select 호출 후 reset 되므로)
      cpyReads = reads;

      //타이머 설정.
      timeout.tv_sec = 600;
      timeout.tv_usec = 0;

      //select 함수 호출 (소켓 이벤트 대기)
      fdNum = select(0, &cpyReads, 0, 0, &timeout);
      if (fdNum == 0) continue; //timeout일때

                          //select가 반환한 이벤트들 처리...
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
               //data 수신 (client send 처리)
               strLen = recv(reads.fd_array[i], recvData, BUF_SIZE - 1, 0);
               if (strLen < 0)    // 상대가 연결을 끊었을 때 
               {
                  if (step == 0) {//대기열
                     ready_num--;//대기인원 감소
                     FD_CLR(reads.fd_array[i], &reads);
                     send_msg(1, -1);
                     continue;
                  }
                  if (step == 1) {//게임중(사망,로그아웃)
                     FD_CLR(reads.fd_array[i], &reads);
                     ready_num--;
                     send_msg(11, -1);
                     continue;
                  }
               }
               else
               {
                  recv_msg(i);//수신데이터별
               }
            }
         }
      }
   }
   closesocket(hServSock);
   WSACleanup();
   return 0;
}

void ErrorHandling(char *message) {//에러출력
   fputs(message, stderr);
   fputc('\n', stderr);
   exit(1);
}

void recv_msg(int num) {//수신데이터별 num = 수신번호
   int i;

   switch (recvData[0]) {
   case 1://대기열 접속시작
         /*
         recvData[0]
         */
      ready_num++;//대기열 인원 증가
      send_msg(1, -1);
      break;

   case 2://대기시간 종료
         /*
         recvData[0]
         */
      ready_count++;
      if (ready_count == ready_num) send_msg(2, -1);
      break;

   case 3://move
         /*
         recvData[0]
         recvData[1] 캐릭터
         recvData[2] dir
         recvData[3] X
         recvData[4] Y
         */
      send_msg(3, -1);
      break;

   case 4://총알 생성
         /*
         recvData[0]
         recvData[1] 총알X
         recvData[2] 총알Y
         */
      send_msg(4, -1);
      break;

   case 5://총알 이동
         /*
         recvData[0]
         recvData[1] dir
         recvData[2] 총알X
         recvData[3] 총알Y
         */
      send_msg(5, -1);
      break;

   case 6://총알 파괴
         /*
         recvData[0]
         recvData[1] dir
         recvData[2] 총알X
         recvData[3] 총알Y
         */
      send_msg(6, -1);
      break;

   case 7://나무만 파괴(바로앞에서 총알발사)
         /*
         recvData[0]
         recvData[1] 나무X
         recvData[2] 나무Y
         */
      send_msg(7, -1);
      break;

   case 8://나무파괴(총알 이동)
         /*
         recvData[0]
         recvData[1] dir
         recvData[2] 총알X
         recvData[3] 총알Y
         */
      send_msg(8, -1);
      break;

   case 9://제자리 총알발사
         /*
         recvData[0]
         recvData[1] 체력달아야할 케릭터
         recvData[2] X
         recvData[3] Y
         */
      send_msg(9, -1);
      break;

   case 10://총알 캐릭터충돌
         /*
         recvData[0]
         recvData[1] dir
         recvData[2] 총알X
         recvData[3] 총알Y
         */
      send_msg(10, -1);
      break;

   case 11://생존신고
         /*
         recvData[0]
         recvData[1] character
         */
      user_count++;
      check[user_count] = recvData[1];
      if (user_count == ready_num) {
         send_msg(12, -1);
         user_count = 0;
         for (i = 0; i < 4; i++) check[i] = 0;//0으로 다 초기화
         if (ready_num == 1) {
            send_msg(14, -1);
            break;   
         }
      }
      break;

   case 13://사망
         /*
         recvData[0]
         recvData[1] character
         */
      send_msg(13, -1);
      break;
   }
}

void send_msg(int opcode, int sendNum) {//전송할 메세지 sendNum = -1 sendAll

   int strLen, i, count = 1;
   sendData[0] = opcode;

   switch (opcode) {
   case 1://대기열 접속중
      sendData[0] = 1;
      sendData[1] = ready_num;//참여인원
      strLen = 2;
      break;

   case 2://게임 시작
      step = 1;
      set_map();//맵 설정
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
      sendData[3] = recvData[3];//삭제할X
      sendData[4] = recvData[4];//삭제할Y
      strLen = 5;
      break;

   case 4://총알 생성
      sendData[1] = recvData[1];//총알X
      sendData[2] = recvData[2];//총알Y
      strLen = 3;
      break;

   case 5://총알 이동
      sendData[1] = recvData[1];//dir
      sendData[2] = recvData[2];//총알X
      sendData[3] = recvData[3];//총알Y
      strLen = 4;
      break;

   case 6://총알 파괴
      sendData[1] = recvData[1];//dir
      sendData[2] = recvData[2];//총알X
      sendData[3] = recvData[3];//총알Y
      strLen = 4;
      break;

   case 7://아이템 전달
      sendData[1] = probability(50, 25, 25);//아이템 0 빈칸 1 추가총알 2 체력
      sendData[2] = recvData[1];//나무X
      sendData[3] = recvData[2];//나무Y
      strLen = 4;
      break;

   case 8://나무파괴(총알 이동)
      sendData[1] = probability(50, 25, 25);//아이템 0 빈칸 1 추가총알 2 체력
      sendData[2] = recvData[2];//총알X
      sendData[3] = recvData[3];//총알Y
      sendData[4] = recvData[1];//dir
      strLen = 5;
      break;

   case 9://총알삭제 케릭터이동
      sendData[1] = recvData[1];//체력달아야할 케릭터
      sendData[2] = recvData[2];//X
      sendData[3] = recvData[3];//Y
      strLen = 4;
      break;

   case 10://총알 캐릭터충돌
      sendData[1] = recvData[1];//dir
      sendData[2] = recvData[2];//총알X
      sendData[3] = recvData[3];//총알Y
      strLen = 4;
      break;

   case 11://나간 클라이언트 검색용
      strLen = 1;
      break;

   case 12://생존한 클라이언트
      sendData[1] = user_count;
      for (i = 0; i < 4; i++) {
         sendData[i + 2] = check[i];
      }
      strLen = 6;
      break;

   case 13://사망
      sendData[1] = recvData[1];
      strLen = 2;
      break;

   case 14://승리
      strLen = 1;
      break;
   }
   if (sendNum == -1) send_all(strLen);//클라이언트 전체에게 메세지 전달
   else send(reads.fd_array[sendNum], sendData, strLen, 0);//해당 클라이언트에게만 메세지 전달
   return;
}

void send_all(int strLen) {//클라이언트 전체에게 메세지 전달
   int i;
   for (i = 0; i < reads.fd_count; i++) {
      if (reads.fd_array[i] != hServSock) {
         send(reads.fd_array[i], sendData, strLen, 0);
      }
   }
}

void set_map() {//맵 설정

   int i, j;

   //map 랜덤배치
   for (i = 0; i < MAP_Y; i++) {
      for (j = 0; j < MAP_X; j++) {
         map[j][i] = 1;
         if (i > 0 && i < MAP_Y - 1 && j > 0 && j < MAP_X - 1) map[j][i] = probability(75, 5, 20);
      }
   }

   //시작위치 빈공간 설정
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

   //인원에 따라 캐릭터 설정
   if (ready_num >= 1) map[1][1] = 13;
   if (ready_num >= 2) map[MAP_X - 2][MAP_Y - 2] = 23;
   if (ready_num >= 3) map[MAP_X - 2][1] = 33;
   if (ready_num == 4) map[1][MAP_Y - 2] = 43;

   //sendData에 map배열 복사
   for (i = 0; i < MAP_X; i++) {
      for (j = 0; j < MAP_Y; j++) {
         sendData[i * MAP_X + j + 2] = map[i][j];
      }
   }
}

int probability(int num1, int num2, int num3) {//확률계산
   int pro;
   pro = rand() % 100;
   if (pro<num1)
      return 0;
   else if (pro<num1 + num2)
      return 1;
   else
      return 2;
}