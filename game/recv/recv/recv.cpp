#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <conio.h>

#pragma comment (lib, "ws2_32")

//PROTOCAL
#define    BUF_SIZE 1024

//COLOR
#define COL GetStdHandle(STD_OUTPUT_HANDLE)
#define GRAY SetConsoleTextAttribute(COL, C_GRAY)
#define GREEN SetConsoleTextAttribute(COL, C_GREEN)
#define C_GREEN 0x0002
#define C_GRAY 0x0008
#define C_RED 0x000c
#define C_YELLOW 0x000e
#define C_SKY_BLUE 0x000b

//KEY
#define LEFT 75
#define RIGHT 77
#define UP 72
#define DOWN 80
#define ESC 27
#define SPACEBAR 32
#define ENTER 13

//MAP
#define SPACE_X 3//여백X
#define SPACE_Y 2//여백Y
#define MAP_X 30//맵X크기
#define MAP_Y 30//맵Y크기

//Start
#define ready_time 10//대기시간
#define gun_max 5//총알 최대갯수
#define helth_max 5//체력 최대량

//FUNCTION
void ErrorHandling(char *message);//에러출력
void ClearCursor();//커서 삭제
void gotoxyDraw(int x, int y, char* s, int color);//x값을 2x로 변경, 좌표값에 바로 문자열을 입력할 수 있도록 printf함수 삽입
void gotoxyDrawInt(int x, int y, int d, int color);//x값을 2x로 변경, 좌표값에 바로 정수를 입력할 수 있도록 printf함수 삽입
void title();//시작화면 출력
void map_edge();//테두리 그리기
int multiplexing();//멀티플렉싱
void recv_msg();//수신데이터별
void send_msg(int opcode);//전송할 메세지
void draw_map();//맵 그리는 함수 
void draw_character();//캐릭터 그리는 함수
void input_key();//키입력
void move(int pos, int dir);//이동할 위치의 맵정보
void trans_draw(int num, int dir);//변화된곳 그리기 dir 0 총알생성
void create_gun(int pos, int dir);//총알 생성
void gun_move();//총알 이동
void gun_collision(int i, int pos);//총알 충돌
void draw_interface();//인터페이스 그리는 함수
void trans_draw_interface(int num);//인터페이스 수정 그리기
void helth_test();//체력검사
void user_delete(int num);//나간 클라이언트 삭제
void user_die(int num);//클라이언트 사망
void user_victory();//승리

//GLOBAL
WSADATA   wsaData;
SOCKET hSocket;
SOCKADDR_IN   servAdr;
char recvData[BUF_SIZE];//수신할 데이터
char sendData[BUF_SIZE];//전송할 데이터
TIMEVAL timeout;
fd_set reads, cpyReads;
int fdNum, strLen;
int ready = 1;//0 대기열 종료
int ready_num = 1;//대기열 인원
int ready_count = 0;//100ms당 1증가
int ready_timer = ready_time;//대기시간
int map[50][40] = { 0 };//MAP 0으로 초기화설정
int key;//입력받을 키 저장
int game = 1;//0 사망
int victory = 1;// 0승리
int user_check[4] = { 0 };//접속중인 유저

//CHARACTER STAT
int character = 0;//캐릭터
int X, Y;//캐릭터 좌표
int direction = 0;//케릭터 방향(초기총알 진행방향) 1 Up 2 RIGHT 3 DOWN 4 LEFT
int gun[gun_max][4] = { 0 };//총알 []번호, [] 0 = 사용가능0/불가1/생성대기2/이동대기3, 1 = 방향, 2 = x, 3 = y
int gun_count = 0;//발사한 총알개수
int gun_startN = 3;//초기 총알개수
int helth = 3;//현재체력
int helth_nowM = 3;//현재 최대체력
int timer_key = 0;//입력키 타이머
int timer_gun = 0;//총알 타이머

int main(void)
{
   int i;

   ClearCursor();//커서 삭제

   if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
      ErrorHandling("WSAStartup() error!");

   hSocket = socket(PF_INET, SOCK_STREAM, 0);//TCP
   if (hSocket == INVALID_SOCKET)
      ErrorHandling("socket() error");

   memset(&servAdr, 0, sizeof(servAdr));
   servAdr.sin_family = AF_INET;
   servAdr.sin_addr.s_addr = inet_addr("127.0.0.1");//IP
   servAdr.sin_port = htons(9000);//PORT

   if (connect(hSocket, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
      ErrorHandling("connect() error!");

   FD_ZERO(&reads);
   FD_SET(hSocket, &reads);

   //------------------------------------------------------Connect------------------------------------------------

   title();//시작화면 출력
   ready = 1;
   while (ready) {
      multiplexing();//멀티플렉싱
   }
   //게임시작
   draw_interface();

   while (game) {
      timer_key++;
      timer_gun++;
	   
      if(timer_key > 20) input_key();//키입력

      if (timer_gun > 20) {
         timer_gun = 0;
         gun_move();//총알이동
      }

      helth_test();
      Sleep(2);
      if (multiplexing()) break;//멀티플렉싱
   }
   
   getch();
   return 0;
}

void ErrorHandling(char *message)//에러출력
{
   fputs(message, stderr);
   fputc('\n', stderr);
   exit(1);
}

void ClearCursor() {//커서 삭제
   CONSOLE_CURSOR_INFO curinfo;
   curinfo.bVisible = 0;
   curinfo.dwSize = 1;
   SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curinfo);
}

void gotoxyDraw(int x, int y, char* s, int color) { //x값을 2x로 변경, 좌표값에 바로 문자열을 입력할 수 있도록 printf함수 삽입
   COORD pos = { 2 * x,y };
   SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
   SetConsoleTextAttribute(COL, color);
   printf("%s", s);
   GRAY;
}

void gotoxyDrawInt(int x, int y, int d, int color) { //x값을 2x로 변경, 좌표값에 바로 정수를 입력할 수 있도록 printf함수 삽입
   COORD pos = { 2 * x,y };
   SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
   SetConsoleTextAttribute(COL, color);
   printf("%d", d);
   GRAY;
}

void title() {//시작화면
   int i;

   system("cls");
   map_edge();//테두리 그리기

   //-----------------------------------Interface----------------------------------------------
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 6, SPACE_Y + MAP_Y / 2 - 4, "+--------------------------+", C_SKY_BLUE);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 6, SPACE_Y + MAP_Y / 2 - 3, "|         G A M E          |", C_SKY_BLUE);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 6, SPACE_Y + MAP_Y / 2 - 2, "+--------------------------+", C_SKY_BLUE);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 6, SPACE_Y + MAP_Y / 2, "       < PLEASE WAIT >      ", C_RED);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 2, "   ※ ←,→,↑,↓ : Move     ", C_GREEN);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 3, "   ※ SPACE BAR : Bomb       ", C_GREEN);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 4, "   - ⊙ : Character          ", C_SKY_BLUE);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 5, "   - ⊙ : Other Character    ", C_RED);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 6, "   - ■ : Block              ", C_GRAY);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 7, "   - △ : Tree               ", C_GREEN);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 8, "   - ▒ : Add Gun Item       ", C_YELLOW);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 9, "   - ♡ : Add Helth Item     ", C_YELLOW);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 10, "   - ¤ : Bomb               ", C_GRAY);
   
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + MAP_Y / 4, "R E A D Y", C_YELLOW);
   for (i = 0; i<4; i++) {
      if (i<ready_num)   gotoxyDraw(SPACE_X + MAP_X + 7 + i, SPACE_Y + MAP_Y / 4 + 2, "● ", C_YELLOW);//참여인원
      else gotoxyDraw(SPACE_X + MAP_X + 7 + i, SPACE_Y + MAP_Y / 4 + 2, "○ ", C_YELLOW);//입장가능한 인원
   }
   //ex)30:00출력
   gotoxyDrawInt(SPACE_X + MAP_X + 5, SPACE_Y + MAP_Y / 4 + 5, (ready_timer / 60) / 10, C_RED);
   gotoxyDrawInt(SPACE_X + MAP_X + 7, SPACE_Y + MAP_Y / 4 + 5, ready_timer / 60, C_RED);
   gotoxyDraw(SPACE_X + MAP_X + 9, SPACE_Y + MAP_Y / 4 + 5, ":", C_RED);
   gotoxyDrawInt(SPACE_X + MAP_X + 11, SPACE_Y + MAP_Y / 4 + 5, (ready_timer % 60) / 10, C_RED);
   gotoxyDrawInt(SPACE_X + MAP_X + 13, SPACE_Y + MAP_Y / 4 + 5, (ready_timer % 60) % 10, C_RED);
   //-----------------------------------Interface----------------------------------------------

   send_msg(1);

   while (ready) {
      Sleep(100);
      multiplexing();//멀티플렉싱
      if (ready_num == 1) {
         ready_timer = ready_time;
         gotoxyDrawInt(SPACE_X + MAP_X + 5, SPACE_Y + MAP_Y / 4 + 5, (ready_timer / 60) / 10, C_RED);
         gotoxyDrawInt(SPACE_X + MAP_X + 7, SPACE_Y + MAP_Y / 4 + 5, ready_timer / 60, C_RED);
         gotoxyDraw(SPACE_X + MAP_X + 9, SPACE_Y + MAP_Y / 4 + 5, ":", C_RED);
         gotoxyDrawInt(SPACE_X + MAP_X + 11, SPACE_Y + MAP_Y / 4 + 5, (ready_timer % 60) / 10, C_RED);
         gotoxyDrawInt(SPACE_X + MAP_X + 13, SPACE_Y + MAP_Y / 4 + 5, (ready_timer % 60) % 10, C_RED);
      }
      else{
         ready_count++;

         if (ready_count >= 10) {//1초지남
            ready_count = 0;
            ready_timer--;//1초 감소
                       //줄어든 시간 출력
            gotoxyDrawInt(SPACE_X + MAP_X + 5, SPACE_Y + MAP_Y / 4 + 5, (ready_timer / 60) / 10, C_RED);
            gotoxyDrawInt(SPACE_X + MAP_X + 7, SPACE_Y + MAP_Y / 4 + 5, ready_timer / 60, C_RED);
            gotoxyDraw(SPACE_X + MAP_X + 9, SPACE_Y + MAP_Y / 4 + 5, ":", C_RED);
            gotoxyDrawInt(SPACE_X + MAP_X + 11, SPACE_Y + MAP_Y / 4 + 5, (ready_timer % 60) / 10, C_RED);
            gotoxyDrawInt(SPACE_X + MAP_X + 13, SPACE_Y + MAP_Y / 4 + 5, (ready_timer % 60) % 10, C_RED);
         }

         if (ready_timer <= 0) {//게임시작
            send_msg(2);
            break;
         }
      }
   }
}

void map_edge() {//테두리 그리기
   int i, j;
   for (j = 0; j < MAP_Y; j++) {
      for (i = 0; i < MAP_X; i++) {
         if (j == 0 || j == MAP_Y - 1 || i == 0 || i == MAP_X - 1) {
            gotoxyDraw(i + SPACE_X + 1, j + SPACE_Y + 1, "■", C_GRAY);
         }
      }
   }
}

int multiplexing() {//멀티플렉싱
   while (1) {
      cpyReads = reads;//복사

      //타이머 설정.
      timeout.tv_sec = 0;
      timeout.tv_usec = 1;

      //select 함수 호출 (소켓 이벤트 대기)
      fdNum = select(1, &cpyReads, 0, 0, &timeout);
      if (fdNum == 0)   return 0; //timeout일때

      //select가 반환한 이벤트들 처리...
      //server가 메세지 보냈는지 확인
      if (FD_ISSET(hSocket, &cpyReads)) {
         strLen = recv(reads.fd_array[0], recvData, BUF_SIZE - 1, 0);
         if (strLen > 0) {
            recv_msg();//수신데이터별
            if (recvData[0] == 14) return 1;
            continue;
         }
         else {
            break;
         }
      }
      
   }
   return 0;
}

void recv_msg() {//수신데이터별
   int i, j, x, y;

   switch (recvData[0]) {
   case 1://대기열 접속중
         /*
         recvData[0]
         recvData[1] 참여인원
         */
      if (ready_num != recvData[1]) {//인원변경
         ready_num = recvData[1];//대기열 인원 변경

         for (i = 0; i<4; i++) {
            if (i<ready_num)   gotoxyDraw(SPACE_X + MAP_X + 7 + i, SPACE_Y + MAP_Y / 4 + 2, "● ", C_YELLOW);//참여인원
            else gotoxyDraw(SPACE_X + MAP_X + 7 + i, SPACE_Y + MAP_Y / 4 + 2, "○ ", C_YELLOW);//입장가능한 인원
         }

         //대기시간 다시 초기화 
		 //새로운 대기자가 등장했을 때 대기시간 초기화
         ready_timer = ready_time;
         ready_count = 0;
      }
      break;

   case 2://게임시작
         /*
         recvData[0]
         recvData[1] 캐릭터조회번호(1,2,3,4)
         recvData[2~Map크기+2] 맵정보
         */
      character = recvData[1];

	  //맵정보를 받아온다.
      for (i = 0; i < MAP_X; i++) {
         for (j = 0; j < MAP_Y; j++) {
            map[i][j] = recvData[i * MAP_X + j + 2];
         }
      }
      draw_map();//맵 그리는 함수 
      draw_character();//캐릭터 그리는 함수
      ready = 0;
      break;
   case 3://move
         /*
         recvData[0]
         recvData[1] 캐릭터
         recvData[2] dir
         recvData[3] 삭제할X
         recvData[4] 삭제할Y
         */
      trans_draw(1, recvData[2]);//변화된곳 그리기
      break;

   case 4://총알 생성
         /*
         recvData[0] ==4
         recvData[1] 총알X
         recvData[2] 총알Y
         */
      trans_draw(2, 0);
      break;

   case 5://총알 이동
         /*
         recvData[0]
         recvData[1] dir
         recvData[2] 총알X
         recvData[3] 총알Y
         */
      trans_draw(2, recvData[1]);
      break;

   case 6://총알 파괴
         /*
         recvData[0]
         recvData[1] dir
         recvData[2] 총알X
         recvData[3] 총알Y
         */
      trans_draw(2, recvData[1]);
      break;

   case 7://나무만 파괴 
         /*
         recvData[0]
         recvData[1] 아이템 0 빈칸 1 추가총알 2 체력
         recvData[2] 나무X
         recvData[3] 나무Y

         */
      if (recvData[1] == 0) map[recvData[2]][recvData[3]] = 0;
      else map[recvData[2]][recvData[3]] = recvData[1] + 4; 
      trans_draw(recvData[1] + 3, 5); //5->아이템.
      break;

   case 8://나무파괴(총알 이동)
         /*
         recvData[0]
         recvData[1] 아이템 0 빈칸 1 추가총알 2 체력
         recvData[2] 총알X
         recvData[3] 총알Y
         recvData[4] dir
         */
      trans_draw(recvData[1] + 3, recvData[4]);
      break;

   case 9://총알삭제 케릭터이동
         /*
         recvData[0]
         recvData[1] 체력달아야할 케릭터
         recvData[2] X
         recvData[3] Y
         */
      //해당 클라이언트 체력감소
      if (character == recvData[1] / 10) {
         helth--;
         trans_draw_interface(3);
      }
      break;

   case 10://총알 캐릭터충돌
         /*
         recvData[0]
         recvData[1] dir
         recvData[2] 총알X
         recvData[3] 총알Y
         */
      x = recvData[2];
      y = recvData[3];
      trans_draw(2, recvData[1]);

      switch (recvData[1]) {
      case 1://UP
         y--;
         break;

      case 2://RIGHT
         x++;
         break;

      case 3://DOWN
         y++;
         break;

      case 4://LEFT
         x--;
         break;
      }
      if (X == x && Y == y) {//해당클라이언트 체력감소
         helth--;
         trans_draw_interface(3);
      }
      break;


	
   case 11://나간 클라이언트 검색용
         /*
         recvData[0]
         */
      send_msg(11);
      break;

   case 12://생존한 클라이언트
         /*
         recvData[0]
         recvData[1] user_count
         recvData[2] character
         recvData[3] character
         recvData[4] character
         */
      for (i = 0; i < 4; i++) {
         switch (recvData[i + 2]) {
         case 1:
            user_check[0] = 1;
            break;
            
         case 2:
            user_check[1] = 1;
            break;

         case 3:
            user_check[2] = 1;
            break;
            
         case 4:
            user_check[3] = 1;
            break;
         }
      }
      for (i = 0; i < 4; i++) {
         if (user_check[i] == 0) break;
      }
      user_delete(i);
      //user_check초기화
      for (i = 0; i < 4; i++) user_check[i] = 0;
      break;

   case 13://사망
         /*
         recvData[0]
         recvData[1] character
         */
      user_die(recvData[1]);
      break;

   case 14://승리
         /*
         recvData[0]
         */
      user_victory();
      break;
   }
}

void send_msg(int opcode) {//전송할 메세지
   int strLen, i;
   int x = X;
   int y = Y;

   sendData[0] = opcode;

   switch (opcode) {
   case 1://대기열 접속 시작
      strLen = 1;
      break;

   case 2://대기시간 종료
      system("cls");
      strLen = 1;
      break;

   case 3://move
      sendData[1] = map[X][Y];//캐릭터
      sendData[2] = direction;//dir
      sendData[3] = X;//X
      sendData[4] = Y;//Y
      strLen = 5;
      break;

   case 4://총알생성
         //대기총알 검색
      for (i = 0; i < gun_max; i++) {
         if (gun[i][0] == 2) //(i,0)==2이면 생성대기임.
            break;
      }
      gun[i][0] = 1; //(i,0)==1이면 불가임
      sendData[1] = gun[i][2];//총알X
      sendData[2] = gun[i][3];//총알Y
      strLen = 3;
      break;

   case 5://총알이동
         //대기총알 검색
      for (i = 0; i < gun_max; i++) {
         if (gun[i][0] == 3) {
            gun[i][0] = 1; 
            sendData[1] = gun[i][1];//dir
            sendData[2] = gun[i][2];//총알X
            sendData[3] = gun[i][3];//총알Y
            strLen = 4;
            send(hSocket, sendData, strLen, 0);
            break;
         }
      }
      return;
      break;

   case 6://총알파괴
	   // sendData[0] = opcode;
      for (i = 0; i < gun_max; i++) {
         if (gun[i][0] == 3) {
            gun[i][0] = 0;
            gun_count--;
            sendData[1] = gun[i][1];//dir
            sendData[2] = gun[i][2];//총알X
            sendData[3] = gun[i][3];//총알Y
            strLen = 4;
            send(hSocket, sendData, strLen, 0);
            break;
         }
      }
      return;
      break;

   case 7://나무만 파괴(바로앞에서 총알발사)
      sendData[1] = X;//X
      sendData[2] = Y;//Y

      switch (direction) {
      case 1://UP
         sendData[2] = Y - 1;//Y
         break;

      case 2://RIGHT
         sendData[1] = X + 1;//X
         break;

      case 3://DOWN
         sendData[2] = Y + 1;//Y
         break;

      case 4://LEFT
         sendData[1] = X - 1;//X
         break;
      }
      strLen = 3;
      break;

   case 8://나무파괴(총알 이동)
      for (i = 0; i < gun_max; i++) {
         if (gun[i][0] == 3) {
            gun[i][0] = 0;
            gun_count--;
            sendData[1] = gun[i][1];//dir
            sendData[2] = gun[i][2];//총알X
            sendData[3] = gun[i][3];//총알Y
            strLen = 4;
            send(hSocket, sendData, strLen, 0);
            break;
         }
      }
      return;
      break;

   case 9://제자리 총알발사
      switch (direction) {
      case 1://UP
         y--;
         break;

      case 2://RIGHT
         x++;
         break;

      case 3://DOWN
         y++;
         break;

      case 4://LEFT
         x--;
         break;
      }
      sendData[1] = map[x][y];//체력달아야할 케릭터
      sendData[2] = x;//X
      sendData[3] = x;//Y
      strLen = 3;
      break;

   case 10://총알 캐릭터충돌
      for (i = 0; i < gun_max; i++) {
         if (gun[i][0] == 3) {
            gun[i][0] = 0;
            gun_count--;
            sendData[1] = gun[i][1];//dir
            sendData[2] = gun[i][2];//총알X
            sendData[3] = gun[i][3];//총알Y
            strLen = 4;
            send(hSocket, sendData, strLen, 0);
            break;
         }
      }
      return;
      break;
      
   case 11://생존신고
      sendData[1] = character;
      strLen = 2;
      break;

   case 13://사망
      sendData[1] = character;
      strLen = 2;
      break;
   }

   send(hSocket, sendData, strLen, 0);
   return;
}

void draw_map() {//맵 그리는 함수 
   int i, j;
   //맵!!!
   map_edge();//테두리 그리기

   for (j = 0; j < MAP_Y; j++) {
      for (i = 0; i < MAP_X; i++) {
         switch (map[i][j])
         {
         case 0: gotoxyDraw(i + SPACE_X + 1, j + SPACE_Y + 1, "  ", C_GRAY);//빈공간
            break;
         case 1: gotoxyDraw(i + SPACE_X + 1, j + SPACE_Y + 1, "■", C_GRAY);//벽
            break;
         case 2: gotoxyDraw(i + SPACE_X + 1, j + SPACE_Y + 1, "△", C_GREEN);//나무
            break;
         case 3: gotoxyDraw(i + SPACE_X + 1, j + SPACE_Y + 1, "⊙", C_SKY_BLUE);//캐릭터
            break;
         case 4: gotoxyDraw(i + SPACE_X + 1, j + SPACE_Y + 1, "¤", C_RED);//총알
            break;
         case 5: gotoxyDraw(i + SPACE_X + 1, j + SPACE_Y + 1, "▒", C_YELLOW);//추가총알
            break;
         case 6: gotoxyDraw(i + SPACE_X + 1, j + SPACE_Y + 1, "♡", C_YELLOW);//체력
            break;
         }
      }
   }
   gotoxyDraw(0, SPACE_Y, "", C_GRAY);
   gotoxyDraw(0, SPACE_Y + MAP_Y, "", C_GRAY);
}

void draw_character() {//캐릭터 그리는 함수

   //적 그리기
   if (map[1][1] % 10 == 3) gotoxyDraw(SPACE_X + 2, SPACE_Y + 2, "⊙", C_RED);
   if (map[MAP_X - 2][1] % 10 == 3) gotoxyDraw(SPACE_X + MAP_X - 1, SPACE_Y + 2, "⊙", C_RED);
   if (map[1][MAP_Y - 2] % 10 == 3) gotoxyDraw(SPACE_X + 2, SPACE_Y + MAP_Y - 1, "⊙", C_RED);
   if (map[MAP_X - 2][MAP_Y - 2] % 10 == 3) gotoxyDraw(SPACE_X + MAP_X - 1, SPACE_Y + MAP_Y - 1, "⊙", C_RED);

   //본인 그리기
   switch (character) {
   case 1:
      X = 1; Y = 1;
      direction = 2;//RIGHT
      gotoxyDraw(SPACE_X + X + 1, SPACE_Y + Y + 1, "⊙", C_SKY_BLUE);
      break;

   case 2:
      X = MAP_X - 2; Y = MAP_Y - 2;
      direction = 4;//LEFT
      gotoxyDraw(SPACE_X + X + 1, SPACE_Y + Y + 1, "⊙", C_SKY_BLUE);
      break;

   case 3:
      X = MAP_X - 2; Y = 1; 
      direction = 4;//LEFT
      gotoxyDraw(SPACE_X + X + 1, SPACE_Y + Y + 1, "⊙", C_SKY_BLUE);
      break;

   case 4:
      X = 1; Y = MAP_Y - 2;
      direction = 2;//RIGHT
      gotoxyDraw(SPACE_X + X + 1, SPACE_Y + Y + 1, "⊙", C_SKY_BLUE);
      break;

   }
   gotoxyDraw(0, SPACE_Y, "", C_GRAY);
   gotoxyDraw(0, SPACE_Y + MAP_Y, "", C_GRAY);
}

void input_key() {//키입력
   int i, count;
   
   fflush(stdin);

   if (multiplexing()) return;//멀티플렉싱
   
   if (kbhit()) { //키입력받음 
      key = getch();

      switch (key) {
      case UP://1
         direction = 1;
         move(map[X][Y - 1], direction);
         break;

      case RIGHT://2
         direction = 2;
         move(map[X + 1][Y], direction);
         break;

      case DOWN://3
         direction = 3;
         move(map[X][Y + 1], direction);
         break;

      case LEFT://4
         direction = 4;
         move(map[X - 1][Y], direction);
         break;

      case SPACEBAR://gun
         switch (direction) {
         case 1://UP
            create_gun(map[X][Y - 1], direction);//총알 생성
            break;

         case 2://RIGHT
            create_gun(map[X + 1][Y], direction);//총알 생성
            break;

         case 3://DOWN
            create_gun(map[X][Y + 1], direction);//총알 생성
            break;

         case 4://LEFT
            create_gun(map[X - 1][Y], direction);//총알 생성
            break;
         }
         break;
      }
      timer_key = 0;
   }
}

void move(int pos, int dir) {//이동할 위치의 맵정보
	
   switch (pos) {
   case 0://빈공간
      send_msg(3);


      switch (direction) {
      case 1://UP
         Y--;
         break;

      case 2://RIGHT
         X++;
         break;

      case 3://DOWN
         Y++;
         break;

      case 4://LEFT
         X--;
         break;
      }
      break;

   case 5://추가총알
      send_msg(3);

      switch (direction) {
      case 1://UP
         Y--;
         break;

      case 2://RIGHT
         X++;
         break;

      case 3://DOWN
         Y++;
         break;

      case 4://LEFT
         X--;
         break;
      }
      if (gun_startN < gun_max) gun_startN++;

      trans_draw_interface(1);
      break;

   case 6://체력
      send_msg(3);
      if (helth_nowM == helth && helth_nowM < helth_max) {
         helth++;
         helth_nowM++;
      }
      else if (helth < helth_nowM) {
         helth++;
      }

      trans_draw_interface(3);

      switch (direction) {
      case 1://UP
         Y--;
         map[X][Y] = map[X][Y + 1];
         map[X][Y + 1] = 0;
         break;

      case 2://RIGHT
         X++;
         map[X][Y] = map[X - 1][Y];
         map[X - 1][1] = 0;
         break;

      case 3://DOWN
         Y++;
         map[X][Y] = map[X][Y - 1];
         map[X][Y - 1] = 0;
         break;

      case 4://LEFT
         X--;
         map[X][Y] = map[X + 1][Y];
         map[X + 1][1] = 0;
         break;
      }
      break;
   }
}

void trans_draw(int num, int dir) {//변화된곳 그리기
   char *paint;

   switch (num) {
   case 1://캐릭터
      paint = "⊙";
      break;

   case 2://총알
      paint = "¤";
      break;

   case 3://빈공간
      paint = "  ";
      break;

   case 4://추가총알
      paint = "▒";
      break;

   case 5://체력
      paint = "♡";
      break;
   }
  
   switch (dir) {
   case 0://총알
      gotoxyDraw(SPACE_X + recvData[1] + 1, SPACE_Y + recvData[2] + 1, paint, C_GRAY);
      break;

   case 1://UP


      if (recvData[1] > 12) { 
         map[recvData[3]][recvData[4]] = 0; 
         map[recvData[3]][recvData[4] - 1] = recvData[1]; 
         gotoxyDraw(SPACE_X + recvData[3] + 1, SPACE_Y + recvData[4] + 1, "  ", C_GRAY);
         if (recvData[1] / 10 == character) 
            gotoxyDraw(SPACE_X + recvData[3] + 1, SPACE_Y + recvData[4], paint, C_SKY_BLUE);
         else
            gotoxyDraw(SPACE_X + recvData[3] + 1, SPACE_Y + recvData[4], paint, C_RED);
      }

      else { 
         gotoxyDraw(SPACE_X + recvData[2] + 1, SPACE_Y + recvData[3] + 1, "  ", C_GRAY);
         if (recvData[0] == 5) 
            gotoxyDraw(SPACE_X + recvData[2] + 1, SPACE_Y + recvData[3], paint, C_GRAY); 
		 /*
         recvData[0]
         recvData[1] dir
         recvData[2] 총알X
         recvData[3] 총알Y
         */
		 // case 8: 나무파괴(총알 이동)  
         if (recvData[0] == 8) {
            if (recvData[1] == 0) map[recvData[2]][recvData[3] - 1] = 0; 
            else map[recvData[2]][recvData[3] - 1] = recvData[1] + 4; 

            gotoxyDraw(SPACE_X + recvData[2] + 1, SPACE_Y + recvData[3], paint, C_YELLOW);
         }
      }
      break;

   case 2://RIGHT
      if (recvData[1] > 12) { 
         map[recvData[3]][recvData[4]] = 0; //map에 x,y초기화
         map[recvData[3] + 1][recvData[4]] = recvData[1];
         gotoxyDraw(SPACE_X + recvData[3] + 1, 
			 SPACE_Y + recvData[4] + 1, 
			 "  ", C_GRAY); 
         if (recvData[1] / 10 == character) 
            gotoxyDraw(SPACE_X + recvData[3] + 2, SPACE_Y + recvData[4] + 1, paint, C_SKY_BLUE);
         else 
            gotoxyDraw(SPACE_X + recvData[3] + 2, SPACE_Y + recvData[4] + 1, paint, C_RED);
      }
      else { //총 아이템 
         gotoxyDraw(SPACE_X + recvData[2] + 1, 
			 SPACE_Y + recvData[3] + 1, "  ", C_GRAY);  
         if (recvData[0] == 5) 
            gotoxyDraw(SPACE_X + recvData[2] + 2, SPACE_Y + recvData[3] + 1, paint, C_GRAY);
         if (recvData[0] == 8) {
            if (recvData[1] == 0) map[recvData[2] + 1][recvData[3]] = 0; 
            else map[recvData[2] + 1][recvData[3]] = recvData[1] + 4;

            gotoxyDraw(SPACE_X + recvData[2] + 2, SPACE_Y + recvData[3] + 1, paint, C_YELLOW);
         }
      }
      break;

   case 3://DOWN
      if (recvData[1] > 12) {
         map[recvData[3]][recvData[4]] = 0;//원래 자리 빈 공간으로 변경
         map[recvData[3]][recvData[4] + 1] = recvData[1];//이동할 자리 캐릭터변경
         gotoxyDraw(SPACE_X + recvData[3] + 1, SPACE_Y + recvData[4] + 1, "  ", C_GRAY);
         if (recvData[1] / 10 == character)
            gotoxyDraw(SPACE_X + recvData[3] + 1, SPACE_Y + recvData[4] + 2, paint, C_SKY_BLUE);
         else
            gotoxyDraw(SPACE_X + recvData[3] + 1, SPACE_Y + recvData[4] + 2, paint, C_RED);
      }
      else {
         gotoxyDraw(SPACE_X + recvData[2] + 1, SPACE_Y + recvData[3] + 1, "  ", C_GRAY);
         if (recvData[0] == 5)
            gotoxyDraw(SPACE_X + recvData[2] + 1, SPACE_Y + recvData[3] + 2, paint, C_GRAY);
         if (recvData[0] == 8) {
            if (recvData[1] == 0) map[recvData[2]][recvData[3] + 1] = 0;
            else map[recvData[2]][recvData[3] + 1] = recvData[1] + 4;
            gotoxyDraw(SPACE_X + recvData[2] + 1, SPACE_Y + recvData[3] + 2, paint, C_YELLOW);
         }
      }
      break;

   case 4://LEFT
      if (recvData[1] > 12) {
         map[recvData[3]][recvData[4]] = 0;//원래자리 빈공간변경
         map[recvData[3] - 1][recvData[4]] = recvData[1];//이동할자리 캐릭터변경
         gotoxyDraw(SPACE_X + recvData[3] + 1, SPACE_Y + recvData[4] + 1, "  ", C_GRAY);
         if (recvData[1] / 10 == character)
            gotoxyDraw(SPACE_X + recvData[3], SPACE_Y + recvData[4] + 1, paint, C_SKY_BLUE);
         else
            gotoxyDraw(SPACE_X + recvData[3], SPACE_Y + recvData[4] + 1, paint, C_RED);
      }
      else {
         gotoxyDraw(SPACE_X + recvData[2] + 1, SPACE_Y + recvData[3] + 1, "  ", C_GRAY);
         if (recvData[0] == 5)
            gotoxyDraw(SPACE_X + recvData[2], SPACE_Y + recvData[3] + 1, paint, C_GRAY);
         if (recvData[0] == 8) {
            if (recvData[1] == 0) map[recvData[2] - 1][recvData[3]] = 0;
            else map[recvData[2] - 1][recvData[3]] = recvData[1] + 4;
            gotoxyDraw(SPACE_X + recvData[2], SPACE_Y + recvData[3] + 1, paint, C_YELLOW);
         }
      }
      break;

   case 5://아이템
      gotoxyDraw(SPACE_X + recvData[2] + 1, SPACE_Y + recvData[3] + 1, paint, C_YELLOW);
      break;
   }
}

void create_gun(int pos, int dir) {//총알 생성
   int i;

   //빈총알 검색
   for (i = 0; i < gun_max; i++) {
      if (gun[i][0] == 0 && gun_startN > gun_count && pos != 1) {
         switch (pos % 10) {
         case 0://빈공간
            gun[i][0] = 2;
            gun[i][1] = direction;
            gun[i][2] = X;
            gun[i][3] = Y;
            gun_count++;
            switch (dir) {
            case 1://UP
               gun[i][3]--;
               break;

            case 2://RIGHT
               gun[i][2]++;
               break;

            case 3://DOWN
               gun[i][3]++;
               break;

            case 4://LEFT
               gun[i][2]--;
               break;
            }
            send_msg(4);
            break;

         case 2://나무
               //부서짐(아이템생성)
            send_msg(7);
            break;

         case 3://캐릭터
            send_msg(9);
            break;
         }
         break;
      }
   }
}

void gun_move() {//총알 이동
   int i;
   trans_draw_interface(2);
   for (i = 0; i < gun_max; i++) {
      if (gun[i][0] == 1) {
         multiplexing();//멀티플렉싱
         switch (gun[i][1]) {
         case 1://UP
            gun_collision(i, map[gun[i][2]][gun[i][3] - 1]);
            break;

         case 2://RIGHT
            gun_collision(i, map[gun[i][2] + 1][gun[i][3]]);
            break;

         case 3://DOWN
            gun_collision(i, map[gun[i][2]][gun[i][3] + 1]);
            break;

         case 4://LEFT
            gun_collision(i, map[gun[i][2] - 1][gun[i][3]]);
            break;
         }
      }
   }
}

void gun_collision(int i, int pos) {//총알 충돌
   switch (pos%10) {
   case 0://빈공간
      gun[i][0] = 3;

      send_msg(5); //case 5 -> 총알이동
      map[gun[i][2]][gun[i][3]] = 0;
      switch (gun[i][1]) {
		  case 1://UP
			 gun[i][3]--;
			 map[gun[i][2]][gun[i][3]] = 4;
			 break;

		  case 2://RIGHT
			 gun[i][2]++;
			 map[gun[i][2]][gun[i][3]] = 4;
			 break;

		  case 3://DOWN
			 gun[i][3]++;
			 map[gun[i][2]][gun[i][3]] = 4;
			 break;

		  case 4://LEFT
			 gun[i][2]--;
			 map[gun[i][2]][gun[i][3]] = 4;
			 break;
      }
      break;
   case 1://벽
      gun[i][0] = 3;
      send_msg(6);
      map[gun[i][2]][gun[i][3]] = 0;
      break;

   case 2://나무파괴
      gun[i][0] = 3;
      send_msg(8);
      map[gun[i][2]][gun[i][3]] = 0;
      break;
	   
   case 3://캐릭터
      gun[i][0] = 3;
      map[gun[i][2]][gun[i][3]] = 0;
      send_msg(10);
      break;

   case 5://아이템
   case 6://아이템
      gun[i][0] = 3;
      send_msg(6);
      map[gun[i][2]][gun[i][3]] = 0;
      break;
   }
}

void draw_interface() {//인터페이스 그리는 함수
   int i;
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 10, "보유한 총알", C_YELLOW);
   gotoxyDrawInt(SPACE_X + MAP_X + 14, SPACE_Y + 10, gun_startN, C_YELLOW);//보유한 총알

   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 12, "남은 총알", C_YELLOW);
   gotoxyDrawInt(SPACE_X + MAP_X + 14, SPACE_Y + 12, gun_startN - gun_count, C_YELLOW);//남은총알

   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 14, "체력 상황", C_YELLOW);
   for (i = 0; i<helth_nowM; i++) {
      gotoxyDraw(SPACE_X + MAP_X + 14 + i, SPACE_Y + 14, "♡", C_RED);
   }
   for (i = 0; i<helth; i++) {
      gotoxyDraw(SPACE_X + MAP_X + 14 + i, SPACE_Y + 14, "♥", C_RED);
   }
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 18, "※ ←,→,↑,↓ : Move     ", C_GREEN);
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 19, "※ SPACE BAR : Bomb       ", C_GREEN);
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 20, "- ⊙ : Character          ", C_SKY_BLUE);
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 21, "- ⊙ : Other Character    ", C_RED);
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 22, "- ■ : Block              ", C_GRAY);
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 23, "- △ : Tree               ", C_GREEN);
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 24, "- ▒ : Add Gun Item       ", C_YELLOW);
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 25, "- ♡ : Add Helth Item     ", C_YELLOW);
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 26, "- ¤ : Bomb               ", C_GRAY);
}

void trans_draw_interface(int num) {//인터페이스 수정 그리기
   int i;

   switch (num) {
   case 1://최대총알
      gotoxyDrawInt(SPACE_X + MAP_X + 14, SPACE_Y + 10, gun_startN, C_YELLOW);//보유한 총알
      break;

   case 2://현재총알
      gotoxyDrawInt(SPACE_X + MAP_X + 14, SPACE_Y + 12, gun_startN - gun_count, C_YELLOW);//남은총알
      break;

   case 3://체력
      for (i = 0; i<helth_nowM; i++) {
         gotoxyDraw(SPACE_X + MAP_X + 14 + i, SPACE_Y + 14, "♡", C_RED);
      }
      for (i = 0; i<helth; i++) {
         gotoxyDraw(SPACE_X + MAP_X + 14 + i, SPACE_Y + 14, "♥", C_RED);
      }
      break;
   }
}

void helth_test() {//체력검사
   if (helth <= 0) {
      gotoxyDraw(SPACE_X + (MAP_X / 2) - 6, SPACE_Y + MAP_Y / 2 - 2, "+--------------------------+", C_SKY_BLUE);
      gotoxyDraw(SPACE_X + (MAP_X / 2) - 6, SPACE_Y + MAP_Y / 2 - 1, "|         GAME OVER        |", C_SKY_BLUE);
      gotoxyDraw(SPACE_X + (MAP_X / 2) - 6, SPACE_Y + MAP_Y / 2, "+--------------------------+", C_SKY_BLUE);
      send_msg(13);
      closesocket(hSocket);
      WSACleanup();
      game = 0;
   }
}

void user_delete(int num) {//나간 클라이언트 삭제
   int i, j;
   
   for (j = 0; j < MAP_Y; j++) {
      for (i = 0; i < MAP_X; i++) {
         if (map[i][j] == (num + 1) * 10 + 3) {
            map[i][j] = 0;
            gotoxyDraw(SPACE_X + 1 + i, SPACE_Y + 1 + j, "  ", C_GRAY);
         }
      }
   }
}

void user_die(int num) {
   int i, j;

   for (j = 0; j < MAP_Y; j++) {
      for (i = 0; i < MAP_X; i++) {
         if (map[i][j] == num * 10 + 3) {
            map[i][j] = 0;
            gotoxyDraw(SPACE_X + 1 + i, SPACE_Y + 1 + j, "  ", C_GRAY);
         }
      }
   }
}

void user_victory() {//승리
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 6, SPACE_Y + MAP_Y / 2 - 2, "+--------------------------+", C_SKY_BLUE);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 6, SPACE_Y + MAP_Y / 2 - 1, "|          VICTORY         |", C_SKY_BLUE);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 6, SPACE_Y + MAP_Y / 2, "+--------------------------+", C_SKY_BLUE);
   game = 0;
   victory = 0;
}