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
#define SPACE_X 3//����X
#define SPACE_Y 2//����Y
#define MAP_X 30//��Xũ��
#define MAP_Y 30//��Yũ��

//Start
#define ready_time 10//���ð�
#define gun_max 5//�Ѿ� �ִ밹��
#define helth_max 5//ü�� �ִ뷮

//FUNCTION
void ErrorHandling(char *message);//�������
void ClearCursor();//Ŀ�� ����
void gotoxyDraw(int x, int y, char* s, int color);//x���� 2x�� ����, ��ǥ���� �ٷ� ���ڿ��� �Է��� �� �ֵ��� printf�Լ� ����
void gotoxyDrawInt(int x, int y, int d, int color);//x���� 2x�� ����, ��ǥ���� �ٷ� ������ �Է��� �� �ֵ��� printf�Լ� ����
void title();//����ȭ�� ���
void map_edge();//�׵θ� �׸���
int multiplexing();//��Ƽ�÷���
void recv_msg();//���ŵ����ͺ�
void send_msg(int opcode);//������ �޼���
void draw_map();//�� �׸��� �Լ� 
void draw_character();//ĳ���� �׸��� �Լ�
void input_key();//Ű�Է�
void move(int pos, int dir);//�̵��� ��ġ�� ������
void trans_draw(int num, int dir);//��ȭ�Ȱ� �׸��� dir 0 �Ѿ˻���
void create_gun(int pos, int dir);//�Ѿ� ����
void gun_move();//�Ѿ� �̵�
void gun_collision(int i, int pos);//�Ѿ� �浹
void draw_interface();//�������̽� �׸��� �Լ�
void trans_draw_interface(int num);//�������̽� ���� �׸���
void helth_test();//ü�°˻�
void user_delete(int num);//���� Ŭ���̾�Ʈ ����
void user_die(int num);//Ŭ���̾�Ʈ ���
void user_victory();//�¸�

//GLOBAL
WSADATA   wsaData;
SOCKET hSocket;
SOCKADDR_IN   servAdr;
char recvData[BUF_SIZE];//������ ������
char sendData[BUF_SIZE];//������ ������
TIMEVAL timeout;
fd_set reads, cpyReads;
int fdNum, strLen;
int ready = 1;//0 ��⿭ ����
int ready_num = 1;//��⿭ �ο�
int ready_count = 0;//100ms�� 1����
int ready_timer = ready_time;//���ð�
int map[50][40] = { 0 };//MAP 0���� �ʱ�ȭ����
int key;//�Է¹��� Ű ����
int game = 1;//0 ���
int victory = 1;// 0�¸�
int user_check[4] = { 0 };//�������� ����

//CHARACTER STAT
int character = 0;//ĳ����
int X, Y;//ĳ���� ��ǥ
int direction = 0;//�ɸ��� ����(�ʱ��Ѿ� �������) 1 Up 2 RIGHT 3 DOWN 4 LEFT
int gun[gun_max][4] = { 0 };//�Ѿ� []��ȣ, [] 0 = ��밡��0/�Ұ�1/�������2/�̵����3, 1 = ����, 2 = x, 3 = y
int gun_count = 0;//�߻��� �Ѿ˰���
int gun_startN = 3;//�ʱ� �Ѿ˰���
int helth = 3;//����ü��
int helth_nowM = 3;//���� �ִ�ü��
int timer_key = 0;//�Է�Ű Ÿ�̸�
int timer_gun = 0;//�Ѿ� Ÿ�̸�

int main(void)
{
   int i;

   ClearCursor();//Ŀ�� ����

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

   title();//����ȭ�� ���
   ready = 1;
   while (ready) {
      multiplexing();//��Ƽ�÷���
   }
   //���ӽ���
   draw_interface();

   while (game) {
      timer_key++;
      timer_gun++;
	   
      if(timer_key > 20) input_key();//Ű�Է�

      if (timer_gun > 20) {
         timer_gun = 0;
         gun_move();//�Ѿ��̵�
      }

      helth_test();
      Sleep(2);
      if (multiplexing()) break;//��Ƽ�÷���
   }
   
   getch();
   return 0;
}

void ErrorHandling(char *message)//�������
{
   fputs(message, stderr);
   fputc('\n', stderr);
   exit(1);
}

void ClearCursor() {//Ŀ�� ����
   CONSOLE_CURSOR_INFO curinfo;
   curinfo.bVisible = 0;
   curinfo.dwSize = 1;
   SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curinfo);
}

void gotoxyDraw(int x, int y, char* s, int color) { //x���� 2x�� ����, ��ǥ���� �ٷ� ���ڿ��� �Է��� �� �ֵ��� printf�Լ� ����
   COORD pos = { 2 * x,y };
   SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
   SetConsoleTextAttribute(COL, color);
   printf("%s", s);
   GRAY;
}

void gotoxyDrawInt(int x, int y, int d, int color) { //x���� 2x�� ����, ��ǥ���� �ٷ� ������ �Է��� �� �ֵ��� printf�Լ� ����
   COORD pos = { 2 * x,y };
   SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
   SetConsoleTextAttribute(COL, color);
   printf("%d", d);
   GRAY;
}

void title() {//����ȭ��
   int i;

   system("cls");
   map_edge();//�׵θ� �׸���

   //-----------------------------------Interface----------------------------------------------
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 6, SPACE_Y + MAP_Y / 2 - 4, "+--------------------------+", C_SKY_BLUE);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 6, SPACE_Y + MAP_Y / 2 - 3, "|         G A M E          |", C_SKY_BLUE);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 6, SPACE_Y + MAP_Y / 2 - 2, "+--------------------------+", C_SKY_BLUE);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 6, SPACE_Y + MAP_Y / 2, "       < PLEASE WAIT >      ", C_RED);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 2, "   �� ��,��,��,�� : Move     ", C_GREEN);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 3, "   �� SPACE BAR : Bomb       ", C_GREEN);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 4, "   - �� : Character          ", C_SKY_BLUE);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 5, "   - �� : Other Character    ", C_RED);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 6, "   - �� : Block              ", C_GRAY);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 7, "   - �� : Tree               ", C_GREEN);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 8, "   - �� : Add Gun Item       ", C_YELLOW);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 9, "   - �� : Add Helth Item     ", C_YELLOW);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 7, SPACE_Y + MAP_Y / 2 + 10, "   - �� : Bomb               ", C_GRAY);
   
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + MAP_Y / 4, "R E A D Y", C_YELLOW);
   for (i = 0; i<4; i++) {
      if (i<ready_num)   gotoxyDraw(SPACE_X + MAP_X + 7 + i, SPACE_Y + MAP_Y / 4 + 2, "�� ", C_YELLOW);//�����ο�
      else gotoxyDraw(SPACE_X + MAP_X + 7 + i, SPACE_Y + MAP_Y / 4 + 2, "�� ", C_YELLOW);//���尡���� �ο�
   }
   //ex)30:00���
   gotoxyDrawInt(SPACE_X + MAP_X + 5, SPACE_Y + MAP_Y / 4 + 5, (ready_timer / 60) / 10, C_RED);
   gotoxyDrawInt(SPACE_X + MAP_X + 7, SPACE_Y + MAP_Y / 4 + 5, ready_timer / 60, C_RED);
   gotoxyDraw(SPACE_X + MAP_X + 9, SPACE_Y + MAP_Y / 4 + 5, ":", C_RED);
   gotoxyDrawInt(SPACE_X + MAP_X + 11, SPACE_Y + MAP_Y / 4 + 5, (ready_timer % 60) / 10, C_RED);
   gotoxyDrawInt(SPACE_X + MAP_X + 13, SPACE_Y + MAP_Y / 4 + 5, (ready_timer % 60) % 10, C_RED);
   //-----------------------------------Interface----------------------------------------------

   send_msg(1);

   while (ready) {
      Sleep(100);
      multiplexing();//��Ƽ�÷���
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

         if (ready_count >= 10) {//1������
            ready_count = 0;
            ready_timer--;//1�� ����
                       //�پ�� �ð� ���
            gotoxyDrawInt(SPACE_X + MAP_X + 5, SPACE_Y + MAP_Y / 4 + 5, (ready_timer / 60) / 10, C_RED);
            gotoxyDrawInt(SPACE_X + MAP_X + 7, SPACE_Y + MAP_Y / 4 + 5, ready_timer / 60, C_RED);
            gotoxyDraw(SPACE_X + MAP_X + 9, SPACE_Y + MAP_Y / 4 + 5, ":", C_RED);
            gotoxyDrawInt(SPACE_X + MAP_X + 11, SPACE_Y + MAP_Y / 4 + 5, (ready_timer % 60) / 10, C_RED);
            gotoxyDrawInt(SPACE_X + MAP_X + 13, SPACE_Y + MAP_Y / 4 + 5, (ready_timer % 60) % 10, C_RED);
         }

         if (ready_timer <= 0) {//���ӽ���
            send_msg(2);
            break;
         }
      }
   }
}

void map_edge() {//�׵θ� �׸���
   int i, j;
   for (j = 0; j < MAP_Y; j++) {
      for (i = 0; i < MAP_X; i++) {
         if (j == 0 || j == MAP_Y - 1 || i == 0 || i == MAP_X - 1) {
            gotoxyDraw(i + SPACE_X + 1, j + SPACE_Y + 1, "��", C_GRAY);
         }
      }
   }
}

int multiplexing() {//��Ƽ�÷���
   while (1) {
      cpyReads = reads;//����

      //Ÿ�̸� ����.
      timeout.tv_sec = 0;
      timeout.tv_usec = 1;

      //select �Լ� ȣ�� (���� �̺�Ʈ ���)
      fdNum = select(1, &cpyReads, 0, 0, &timeout);
      if (fdNum == 0)   return 0; //timeout�϶�

      //select�� ��ȯ�� �̺�Ʈ�� ó��...
      //server�� �޼��� ���´��� Ȯ��
      if (FD_ISSET(hSocket, &cpyReads)) {
         strLen = recv(reads.fd_array[0], recvData, BUF_SIZE - 1, 0);
         if (strLen > 0) {
            recv_msg();//���ŵ����ͺ�
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

void recv_msg() {//���ŵ����ͺ�
   int i, j, x, y;

   switch (recvData[0]) {
   case 1://��⿭ ������
         /*
         recvData[0]
         recvData[1] �����ο�
         */
      if (ready_num != recvData[1]) {//�ο�����
         ready_num = recvData[1];//��⿭ �ο� ����

         for (i = 0; i<4; i++) {
            if (i<ready_num)   gotoxyDraw(SPACE_X + MAP_X + 7 + i, SPACE_Y + MAP_Y / 4 + 2, "�� ", C_YELLOW);//�����ο�
            else gotoxyDraw(SPACE_X + MAP_X + 7 + i, SPACE_Y + MAP_Y / 4 + 2, "�� ", C_YELLOW);//���尡���� �ο�
         }

         //���ð� �ٽ� �ʱ�ȭ 
		 //���ο� ����ڰ� �������� �� ���ð� �ʱ�ȭ
         ready_timer = ready_time;
         ready_count = 0;
      }
      break;

   case 2://���ӽ���
         /*
         recvData[0]
         recvData[1] ĳ������ȸ��ȣ(1,2,3,4)
         recvData[2~Mapũ��+2] ������
         */
      character = recvData[1];

	  //�������� �޾ƿ´�.
      for (i = 0; i < MAP_X; i++) {
         for (j = 0; j < MAP_Y; j++) {
            map[i][j] = recvData[i * MAP_X + j + 2];
         }
      }
      draw_map();//�� �׸��� �Լ� 
      draw_character();//ĳ���� �׸��� �Լ�
      ready = 0;
      break;
   case 3://move
         /*
         recvData[0]
         recvData[1] ĳ����
         recvData[2] dir
         recvData[3] ������X
         recvData[4] ������Y
         */
      trans_draw(1, recvData[2]);//��ȭ�Ȱ� �׸���
      break;

   case 4://�Ѿ� ����
         /*
         recvData[0] ==4
         recvData[1] �Ѿ�X
         recvData[2] �Ѿ�Y
         */
      trans_draw(2, 0);
      break;

   case 5://�Ѿ� �̵�
         /*
         recvData[0]
         recvData[1] dir
         recvData[2] �Ѿ�X
         recvData[3] �Ѿ�Y
         */
      trans_draw(2, recvData[1]);
      break;

   case 6://�Ѿ� �ı�
         /*
         recvData[0]
         recvData[1] dir
         recvData[2] �Ѿ�X
         recvData[3] �Ѿ�Y
         */
      trans_draw(2, recvData[1]);
      break;

   case 7://������ �ı� 
         /*
         recvData[0]
         recvData[1] ������ 0 ��ĭ 1 �߰��Ѿ� 2 ü��
         recvData[2] ����X
         recvData[3] ����Y

         */
      if (recvData[1] == 0) map[recvData[2]][recvData[3]] = 0;
      else map[recvData[2]][recvData[3]] = recvData[1] + 4; 
      trans_draw(recvData[1] + 3, 5); //5->������.
      break;

   case 8://�����ı�(�Ѿ� �̵�)
         /*
         recvData[0]
         recvData[1] ������ 0 ��ĭ 1 �߰��Ѿ� 2 ü��
         recvData[2] �Ѿ�X
         recvData[3] �Ѿ�Y
         recvData[4] dir
         */
      trans_draw(recvData[1] + 3, recvData[4]);
      break;

   case 9://�Ѿ˻��� �ɸ����̵�
         /*
         recvData[0]
         recvData[1] ü�´޾ƾ��� �ɸ���
         recvData[2] X
         recvData[3] Y
         */
      //�ش� Ŭ���̾�Ʈ ü�°���
      if (character == recvData[1] / 10) {
         helth--;
         trans_draw_interface(3);
      }
      break;

   case 10://�Ѿ� ĳ�����浹
         /*
         recvData[0]
         recvData[1] dir
         recvData[2] �Ѿ�X
         recvData[3] �Ѿ�Y
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
      if (X == x && Y == y) {//�ش�Ŭ���̾�Ʈ ü�°���
         helth--;
         trans_draw_interface(3);
      }
      break;


	
   case 11://���� Ŭ���̾�Ʈ �˻���
         /*
         recvData[0]
         */
      send_msg(11);
      break;

   case 12://������ Ŭ���̾�Ʈ
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
      //user_check�ʱ�ȭ
      for (i = 0; i < 4; i++) user_check[i] = 0;
      break;

   case 13://���
         /*
         recvData[0]
         recvData[1] character
         */
      user_die(recvData[1]);
      break;

   case 14://�¸�
         /*
         recvData[0]
         */
      user_victory();
      break;
   }
}

void send_msg(int opcode) {//������ �޼���
   int strLen, i;
   int x = X;
   int y = Y;

   sendData[0] = opcode;

   switch (opcode) {
   case 1://��⿭ ���� ����
      strLen = 1;
      break;

   case 2://���ð� ����
      system("cls");
      strLen = 1;
      break;

   case 3://move
      sendData[1] = map[X][Y];//ĳ����
      sendData[2] = direction;//dir
      sendData[3] = X;//X
      sendData[4] = Y;//Y
      strLen = 5;
      break;

   case 4://�Ѿ˻���
         //����Ѿ� �˻�
      for (i = 0; i < gun_max; i++) {
         if (gun[i][0] == 2) //(i,0)==2�̸� ���������.
            break;
      }
      gun[i][0] = 1; //(i,0)==1�̸� �Ұ���
      sendData[1] = gun[i][2];//�Ѿ�X
      sendData[2] = gun[i][3];//�Ѿ�Y
      strLen = 3;
      break;

   case 5://�Ѿ��̵�
         //����Ѿ� �˻�
      for (i = 0; i < gun_max; i++) {
         if (gun[i][0] == 3) {
            gun[i][0] = 1; 
            sendData[1] = gun[i][1];//dir
            sendData[2] = gun[i][2];//�Ѿ�X
            sendData[3] = gun[i][3];//�Ѿ�Y
            strLen = 4;
            send(hSocket, sendData, strLen, 0);
            break;
         }
      }
      return;
      break;

   case 6://�Ѿ��ı�
	   // sendData[0] = opcode;
      for (i = 0; i < gun_max; i++) {
         if (gun[i][0] == 3) {
            gun[i][0] = 0;
            gun_count--;
            sendData[1] = gun[i][1];//dir
            sendData[2] = gun[i][2];//�Ѿ�X
            sendData[3] = gun[i][3];//�Ѿ�Y
            strLen = 4;
            send(hSocket, sendData, strLen, 0);
            break;
         }
      }
      return;
      break;

   case 7://������ �ı�(�ٷξտ��� �Ѿ˹߻�)
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

   case 8://�����ı�(�Ѿ� �̵�)
      for (i = 0; i < gun_max; i++) {
         if (gun[i][0] == 3) {
            gun[i][0] = 0;
            gun_count--;
            sendData[1] = gun[i][1];//dir
            sendData[2] = gun[i][2];//�Ѿ�X
            sendData[3] = gun[i][3];//�Ѿ�Y
            strLen = 4;
            send(hSocket, sendData, strLen, 0);
            break;
         }
      }
      return;
      break;

   case 9://���ڸ� �Ѿ˹߻�
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
      sendData[1] = map[x][y];//ü�´޾ƾ��� �ɸ���
      sendData[2] = x;//X
      sendData[3] = x;//Y
      strLen = 3;
      break;

   case 10://�Ѿ� ĳ�����浹
      for (i = 0; i < gun_max; i++) {
         if (gun[i][0] == 3) {
            gun[i][0] = 0;
            gun_count--;
            sendData[1] = gun[i][1];//dir
            sendData[2] = gun[i][2];//�Ѿ�X
            sendData[3] = gun[i][3];//�Ѿ�Y
            strLen = 4;
            send(hSocket, sendData, strLen, 0);
            break;
         }
      }
      return;
      break;
      
   case 11://�����Ű�
      sendData[1] = character;
      strLen = 2;
      break;

   case 13://���
      sendData[1] = character;
      strLen = 2;
      break;
   }

   send(hSocket, sendData, strLen, 0);
   return;
}

void draw_map() {//�� �׸��� �Լ� 
   int i, j;
   //��!!!
   map_edge();//�׵θ� �׸���

   for (j = 0; j < MAP_Y; j++) {
      for (i = 0; i < MAP_X; i++) {
         switch (map[i][j])
         {
         case 0: gotoxyDraw(i + SPACE_X + 1, j + SPACE_Y + 1, "  ", C_GRAY);//�����
            break;
         case 1: gotoxyDraw(i + SPACE_X + 1, j + SPACE_Y + 1, "��", C_GRAY);//��
            break;
         case 2: gotoxyDraw(i + SPACE_X + 1, j + SPACE_Y + 1, "��", C_GREEN);//����
            break;
         case 3: gotoxyDraw(i + SPACE_X + 1, j + SPACE_Y + 1, "��", C_SKY_BLUE);//ĳ����
            break;
         case 4: gotoxyDraw(i + SPACE_X + 1, j + SPACE_Y + 1, "��", C_RED);//�Ѿ�
            break;
         case 5: gotoxyDraw(i + SPACE_X + 1, j + SPACE_Y + 1, "��", C_YELLOW);//�߰��Ѿ�
            break;
         case 6: gotoxyDraw(i + SPACE_X + 1, j + SPACE_Y + 1, "��", C_YELLOW);//ü��
            break;
         }
      }
   }
   gotoxyDraw(0, SPACE_Y, "", C_GRAY);
   gotoxyDraw(0, SPACE_Y + MAP_Y, "", C_GRAY);
}

void draw_character() {//ĳ���� �׸��� �Լ�

   //�� �׸���
   if (map[1][1] % 10 == 3) gotoxyDraw(SPACE_X + 2, SPACE_Y + 2, "��", C_RED);
   if (map[MAP_X - 2][1] % 10 == 3) gotoxyDraw(SPACE_X + MAP_X - 1, SPACE_Y + 2, "��", C_RED);
   if (map[1][MAP_Y - 2] % 10 == 3) gotoxyDraw(SPACE_X + 2, SPACE_Y + MAP_Y - 1, "��", C_RED);
   if (map[MAP_X - 2][MAP_Y - 2] % 10 == 3) gotoxyDraw(SPACE_X + MAP_X - 1, SPACE_Y + MAP_Y - 1, "��", C_RED);

   //���� �׸���
   switch (character) {
   case 1:
      X = 1; Y = 1;
      direction = 2;//RIGHT
      gotoxyDraw(SPACE_X + X + 1, SPACE_Y + Y + 1, "��", C_SKY_BLUE);
      break;

   case 2:
      X = MAP_X - 2; Y = MAP_Y - 2;
      direction = 4;//LEFT
      gotoxyDraw(SPACE_X + X + 1, SPACE_Y + Y + 1, "��", C_SKY_BLUE);
      break;

   case 3:
      X = MAP_X - 2; Y = 1; 
      direction = 4;//LEFT
      gotoxyDraw(SPACE_X + X + 1, SPACE_Y + Y + 1, "��", C_SKY_BLUE);
      break;

   case 4:
      X = 1; Y = MAP_Y - 2;
      direction = 2;//RIGHT
      gotoxyDraw(SPACE_X + X + 1, SPACE_Y + Y + 1, "��", C_SKY_BLUE);
      break;

   }
   gotoxyDraw(0, SPACE_Y, "", C_GRAY);
   gotoxyDraw(0, SPACE_Y + MAP_Y, "", C_GRAY);
}

void input_key() {//Ű�Է�
   int i, count;
   
   fflush(stdin);

   if (multiplexing()) return;//��Ƽ�÷���
   
   if (kbhit()) { //Ű�Է¹��� 
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
            create_gun(map[X][Y - 1], direction);//�Ѿ� ����
            break;

         case 2://RIGHT
            create_gun(map[X + 1][Y], direction);//�Ѿ� ����
            break;

         case 3://DOWN
            create_gun(map[X][Y + 1], direction);//�Ѿ� ����
            break;

         case 4://LEFT
            create_gun(map[X - 1][Y], direction);//�Ѿ� ����
            break;
         }
         break;
      }
      timer_key = 0;
   }
}

void move(int pos, int dir) {//�̵��� ��ġ�� ������
	
   switch (pos) {
   case 0://�����
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

   case 5://�߰��Ѿ�
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

   case 6://ü��
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

void trans_draw(int num, int dir) {//��ȭ�Ȱ� �׸���
   char *paint;

   switch (num) {
   case 1://ĳ����
      paint = "��";
      break;

   case 2://�Ѿ�
      paint = "��";
      break;

   case 3://�����
      paint = "  ";
      break;

   case 4://�߰��Ѿ�
      paint = "��";
      break;

   case 5://ü��
      paint = "��";
      break;
   }
  
   switch (dir) {
   case 0://�Ѿ�
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
         recvData[2] �Ѿ�X
         recvData[3] �Ѿ�Y
         */
		 // case 8: �����ı�(�Ѿ� �̵�)  
         if (recvData[0] == 8) {
            if (recvData[1] == 0) map[recvData[2]][recvData[3] - 1] = 0; 
            else map[recvData[2]][recvData[3] - 1] = recvData[1] + 4; 

            gotoxyDraw(SPACE_X + recvData[2] + 1, SPACE_Y + recvData[3], paint, C_YELLOW);
         }
      }
      break;

   case 2://RIGHT
      if (recvData[1] > 12) { 
         map[recvData[3]][recvData[4]] = 0; //map�� x,y�ʱ�ȭ
         map[recvData[3] + 1][recvData[4]] = recvData[1];
         gotoxyDraw(SPACE_X + recvData[3] + 1, 
			 SPACE_Y + recvData[4] + 1, 
			 "  ", C_GRAY); 
         if (recvData[1] / 10 == character) 
            gotoxyDraw(SPACE_X + recvData[3] + 2, SPACE_Y + recvData[4] + 1, paint, C_SKY_BLUE);
         else 
            gotoxyDraw(SPACE_X + recvData[3] + 2, SPACE_Y + recvData[4] + 1, paint, C_RED);
      }
      else { //�� ������ 
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
         map[recvData[3]][recvData[4]] = 0;//���� �ڸ� �� �������� ����
         map[recvData[3]][recvData[4] + 1] = recvData[1];//�̵��� �ڸ� ĳ���ͺ���
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
         map[recvData[3]][recvData[4]] = 0;//�����ڸ� ���������
         map[recvData[3] - 1][recvData[4]] = recvData[1];//�̵����ڸ� ĳ���ͺ���
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

   case 5://������
      gotoxyDraw(SPACE_X + recvData[2] + 1, SPACE_Y + recvData[3] + 1, paint, C_YELLOW);
      break;
   }
}

void create_gun(int pos, int dir) {//�Ѿ� ����
   int i;

   //���Ѿ� �˻�
   for (i = 0; i < gun_max; i++) {
      if (gun[i][0] == 0 && gun_startN > gun_count && pos != 1) {
         switch (pos % 10) {
         case 0://�����
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

         case 2://����
               //�μ���(�����ۻ���)
            send_msg(7);
            break;

         case 3://ĳ����
            send_msg(9);
            break;
         }
         break;
      }
   }
}

void gun_move() {//�Ѿ� �̵�
   int i;
   trans_draw_interface(2);
   for (i = 0; i < gun_max; i++) {
      if (gun[i][0] == 1) {
         multiplexing();//��Ƽ�÷���
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

void gun_collision(int i, int pos) {//�Ѿ� �浹
   switch (pos%10) {
   case 0://�����
      gun[i][0] = 3;

      send_msg(5); //case 5 -> �Ѿ��̵�
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
   case 1://��
      gun[i][0] = 3;
      send_msg(6);
      map[gun[i][2]][gun[i][3]] = 0;
      break;

   case 2://�����ı�
      gun[i][0] = 3;
      send_msg(8);
      map[gun[i][2]][gun[i][3]] = 0;
      break;
	   
   case 3://ĳ����
      gun[i][0] = 3;
      map[gun[i][2]][gun[i][3]] = 0;
      send_msg(10);
      break;

   case 5://������
   case 6://������
      gun[i][0] = 3;
      send_msg(6);
      map[gun[i][2]][gun[i][3]] = 0;
      break;
   }
}

void draw_interface() {//�������̽� �׸��� �Լ�
   int i;
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 10, "������ �Ѿ�", C_YELLOW);
   gotoxyDrawInt(SPACE_X + MAP_X + 14, SPACE_Y + 10, gun_startN, C_YELLOW);//������ �Ѿ�

   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 12, "���� �Ѿ�", C_YELLOW);
   gotoxyDrawInt(SPACE_X + MAP_X + 14, SPACE_Y + 12, gun_startN - gun_count, C_YELLOW);//�����Ѿ�

   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 14, "ü�� ��Ȳ", C_YELLOW);
   for (i = 0; i<helth_nowM; i++) {
      gotoxyDraw(SPACE_X + MAP_X + 14 + i, SPACE_Y + 14, "��", C_RED);
   }
   for (i = 0; i<helth; i++) {
      gotoxyDraw(SPACE_X + MAP_X + 14 + i, SPACE_Y + 14, "��", C_RED);
   }
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 18, "�� ��,��,��,�� : Move     ", C_GREEN);
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 19, "�� SPACE BAR : Bomb       ", C_GREEN);
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 20, "- �� : Character          ", C_SKY_BLUE);
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 21, "- �� : Other Character    ", C_RED);
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 22, "- �� : Block              ", C_GRAY);
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 23, "- �� : Tree               ", C_GREEN);
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 24, "- �� : Add Gun Item       ", C_YELLOW);
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 25, "- �� : Add Helth Item     ", C_YELLOW);
   gotoxyDraw(SPACE_X + MAP_X + 7, SPACE_Y + 26, "- �� : Bomb               ", C_GRAY);
}

void trans_draw_interface(int num) {//�������̽� ���� �׸���
   int i;

   switch (num) {
   case 1://�ִ��Ѿ�
      gotoxyDrawInt(SPACE_X + MAP_X + 14, SPACE_Y + 10, gun_startN, C_YELLOW);//������ �Ѿ�
      break;

   case 2://�����Ѿ�
      gotoxyDrawInt(SPACE_X + MAP_X + 14, SPACE_Y + 12, gun_startN - gun_count, C_YELLOW);//�����Ѿ�
      break;

   case 3://ü��
      for (i = 0; i<helth_nowM; i++) {
         gotoxyDraw(SPACE_X + MAP_X + 14 + i, SPACE_Y + 14, "��", C_RED);
      }
      for (i = 0; i<helth; i++) {
         gotoxyDraw(SPACE_X + MAP_X + 14 + i, SPACE_Y + 14, "��", C_RED);
      }
      break;
   }
}

void helth_test() {//ü�°˻�
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

void user_delete(int num) {//���� Ŭ���̾�Ʈ ����
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

void user_victory() {//�¸�
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 6, SPACE_Y + MAP_Y / 2 - 2, "+--------------------------+", C_SKY_BLUE);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 6, SPACE_Y + MAP_Y / 2 - 1, "|          VICTORY         |", C_SKY_BLUE);
   gotoxyDraw(SPACE_X + (MAP_X / 2) - 6, SPACE_Y + MAP_Y / 2, "+--------------------------+", C_SKY_BLUE);
   game = 0;
   victory = 0;
}