// portscan.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <signal.h>
////#include <unistd.h>
#include <sys/types.h>
////#include <sys/wait.h>

using namespace std;
#pragma comment(lib,"Ws2_32.lib")
char IP[] = "23.123.23.13";//"192.168.89.254";//"122.10.91.48";//
int start = 80,temp, err, nret;

SOCKET sock;
SOCKADDR_IN Info;
WSADATA wsadata;
#ifndef USE_REFERENCE
int _tmain(int argc, _TCHAR* argv[])
{
   int end = 100;
   err = WSAStartup(MAKEWORD(2, 2), &wsadata);
   if(err != 0)
   {
	  cout << "Error with winsock. Will Now Exit." << endl;
	  cin.get();
	  return 0;
   }
#if 0   
   cout << "Target IP: ";
   cin>>IP;
   cout << "Starting Port: ";
   cin>>start;
   cout << "End Port: ";
   cin>>end;
   
   cin.ignore();

#endif
   cout << endl << endl << "Starting Scan..." << endl << endl;
   
   temp = start;
   while(temp < end)
   {
	   sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

#define USE_IOCTSOCKET   
#ifdef USE_IOCTSOCKET
		//int nNetTimeout = 1000; //1秒
		//发送时限
		//setsockopt( sock, SOL_SOCKET, SO_SNDTIMEO, ( char * )&nNetTimeout, sizeof( int ) );
		//接收时限
		//setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, ( char * )&nNetTimeout, sizeof( int ) );

		u_long ul=1;//代表非阻塞
		int iResult = ioctlsocket(sock,FIONBIO,&ul);//设置为非阻塞连接
		if(iResult != NO_ERROR)
		{
			cout << "ioctlsocket failed with error" << endl;
		}
#endif
	  
	  Info.sin_family = AF_INET;
	  Info.sin_port = htons(temp);
	  ////////Info.sin_addr.S_un.S_addr = inet_addr(IP);
	  Info.sin_addr.s_addr = inet_addr(IP);
	  nret = connect(sock, (struct sockaddr *)&Info, sizeof(Info));
	  ul = 0;
	  ioctlsocket(sock,FIONBIO,&ul);  
#ifdef USE_IOCTSOCKET
	  
	  	fd_set rfd,err1;      //描述符集 这个将测试连接是否可用
		struct timeval timeout;  //时间结构体
		FD_ZERO(&rfd);//先清空一个描述符集
		FD_ZERO(&err1);//先清空一个描述符集
		timeout.tv_sec = 2;//秒
		timeout.tv_usec = 0;//一百万分之一秒，微秒
 
		FD_SET(sock,&rfd);
		FD_SET(sock,&err1);

		int ret = select(0,NULL,&rfd,&err1,&timeout);//select(0, 0, &rfd, 0, &timeout);
		//if(ret<=0)
		//{
		////     说明超时，仍然没有连接成功
		////     然后可以在程序中做相应的处理
		//	nret = 0;
		//}
		//else
		//{
		//	nret = 1;
		//}

#endif
//	  fd_set Write, Err;
 //   FD_ZERO(&Write);
 //   FD_ZERO(&Err);
 //   FD_SET(sock, &Write);
 //   FD_SET(sock, &Err);
 //
 //   // check if the socket is ready
 //   select(0,NULL,&Write,&Err,&timeout);			
    if(FD_ISSET(sock, &rfd)) 
    {	
       cout << "Port " << temp << " - OPEN! " << endl;
    }
	  if(nret != SOCKET_ERROR)//if( -1 == bind(sock,(SOCKADDR*)&Info,sizeof(SOCKADDR)))//把网卡地址强行绑定到Soket//
	  {
		 cout << "Port " << temp << " - OPEN! " << endl;
	  }
	  else
	  {
		  cout << "Port " << temp << " - CLOSE! " << endl;
	  }
	  temp++;
	  closesocket(sock);
	}
	cout << endl << "Finished With Scan..." << endl;
	
	cin.get();
	return 0;
}
#else

//A socket is in blocking mode by default. If you switch it to non-blocking mode using ioctlsocket(FIONBIO), you can use select() to manage timeouts:
int _tmain(int argc, _TCHAR* argv[])
{
	SOCKET sock, connected;
	int bytes_recieved;  
	char send_data [128] , recv_data[128];         
	SOCKADDR_IN server_addr,client_addr;      
	int sin_size;  
	int j = 0, ret;  
	fd_set fd;
	timeval tv;

	sock = ::socket(AF_INET, SOCK_STREAM, 0);  

	server_addr.sin_family = AF_INET;           
	server_addr.sin_port = htons(start);       
	server_addr.sin_addr.s_addr = INADDR_ANY;   
	//server_addr.sin_addr.S_un.S_addr = inet_addr(IP);

	::bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));  
	::listen(sock, 1);  
	::fflush(stdout);  

	u_long nbio = 1;
	::ioctlsocket(sock, FIONBIO, &nbio);

	while(1) 
	{   
		FD_ZERO(&fd);
		FD_SET(sock, &fd);

		tv.tv_sec = 5;
		tv.tv_usec = 0;

		if (select(0, &fd, NULL, NULL, &tv) > 0)
		{
			sin_size = sizeof(struct sockaddr_in); 
			connected = ::accept(sock, (struct sockaddr *)&client_addr, &sin_size); 

			nbio = 1;
			::ioctlsocket(connected, FIONBIO, &nbio);

			while (1) 
			{ 
				j++; 

				if (::send(connected, send_data, strlen(send_data), 0) < 0)
				{
					//dealing with lost communication ?  
					//and reastablishing communication 
					//set timeout and reset on timeout error     

					if (WSAGetLastError() == WSAEWOULDBLOCK)
					{
						FD_ZERO(&fd);
						FD_SET(connected, &fd);

						tv.tv_sec = 5;
						tv.tv_usec = 0;

						if (select(0, NULL, &fd, NULL, &tv) > 0)
							continue;
					}

					break;
				}
			} 

			closesocket(connected);
		} 
	}
	return 0;
}

#endif

