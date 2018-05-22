#include "net.h"

bool testIP(char* ip, char* port)
{
	SOCKET  cfd;
	SOCKADDR_IN  cadd;


	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return false;
	}
	//����socket����  
	if (-1 == (cfd = socket(AF_INET, SOCK_STREAM, 0)))
	{
		return false;
	}

	//����Ҫ���ӵķ���˵�ַ
	cadd.sin_family = AF_INET;
	cadd.sin_port = htons(atoi(port));
	cadd.sin_addr.s_addr = inet_addr(ip);

	//���ӷ�����  
	int cc;
	if (-1 == (cc = connect(cfd, (struct sockaddr*)&cadd, sizeof(SOCKADDR))))
	{
		return false;
	}

	closesocket(cfd);
	return 1;
}