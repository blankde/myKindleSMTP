#pragma once
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>  
#include <string.h>  
#include "stdafx.h"   
#define BUF_SIZE 1024  


bool testIP(char* ip, char* port);
bool GetInterNetURLText(LPSTR lpcInterNetURL, char* buff);