#include <winsock2.h>
#include <stdio.h>

typedef struct CLIENT_INFO CLIENT_INFO;

struct CLIENT_INFO
{
	SOCKET hClientSocket;
	struct sockaddr_in clientAddr;
};

int InitWinSock2_0(void);
BOOL WINAPI ClientThread(LPVOID lpData);

int main(void)
{
	if (!InitWinSock2_0()) {
        printf("Unable to Initialize Windows Socket environment %d\n",
               WSAGetLastError());
		return -1;
	}

    char szServerIPAddr[] = "127.0.0.1";    // Put here the IP address of the server
    int nServerPort = 5050;                  // The server port that will be used by
    // clients to talk with the server
	SOCKET hServerSocket;

	hServerSocket = socket(AF_INET, // The address family. AF_INET specifies TCP/IP
			SOCK_STREAM,    // Protocol type. SOCK_STREM specified TCP
			0            // Protoco Name. Should be 0 for AF_INET address family
			);
	if (hServerSocket == INVALID_SOCKET) {
        printf("Unable to create Server socket\n");
		// Cleanup the environment initialized by WSAStartup()
		WSACleanup();
		return -1;
	}

	// Create the structure describing various Server parameters
	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;     // The address family. MUST be AF_INET
	serverAddr.sin_addr.s_addr = inet_addr(szServerIPAddr);
	serverAddr.sin_port = htons(nServerPort);

	// Bind the Server socket to the address & port
	if (bind(hServerSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr))
			== SOCKET_ERROR) {
        printf("Unable to bind to %s port %d", szServerIPAddr, nServerPort);
		// Free the socket and cleanup the environment initialized by WSAStartup()
		closesocket(hServerSocket);
		WSACleanup();
		return -1;
	}

	// Put the Server socket in listen state so that it can wait for client connections
	if (listen(hServerSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Unable to put server in listen state\n");
		// Free the socket and cleanup the environment initialized by WSAStartup()
		closesocket(hServerSocket);
		WSACleanup();
		return -1;
	}

	// Start the infinite loop
    while (1) {
		// As the socket is in listen mode there is a connection request pending.
		// Calling accept( ) will succeed and return the socket for the request.
		SOCKET hClientSocket;
		struct sockaddr_in clientAddr;
		int nSize = sizeof(clientAddr);

		hClientSocket = accept(hServerSocket, (struct sockaddr *) &clientAddr,
				&nSize);
		if (hClientSocket == INVALID_SOCKET) {
            printf("accept( ) failed\n");
		} else {
			HANDLE hClientThread;
			struct CLIENT_INFO clientInfo;
			DWORD dwThreadId;

			clientInfo.clientAddr = clientAddr;
			clientInfo.hClientSocket = hClientSocket;

            printf("Client connected from %s\n", inet_ntoa(clientAddr.sin_addr));

			// Start the client thread
			hClientThread = CreateThread(NULL, 0,
					(LPTHREAD_START_ROUTINE) ClientThread,
					(LPVOID) & clientInfo, 0, &dwThreadId);
			if (hClientThread == NULL) {
                printf("Unable to create client thread\n");
			} else {
				CloseHandle(hClientThread);
			}
		}
	}

	closesocket(hServerSocket);
	WSACleanup();
	return 0;
}

int InitWinSock2_0(void)
{
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 0);

    if (!WSAStartup(wVersion, &wsaData)) {
        return 1;
    }

    return 0;
}

BOOL WINAPI ClientThread(LPVOID lpData)
{
    CLIENT_INFO *pClientInfo = (CLIENT_INFO *) lpData;
    char szBuffer[1024];
    int nLength;

    while (1) {
        nLength = recv(pClientInfo->hClientSocket, szBuffer, sizeof(szBuffer),
                0);
        if (nLength > 0) {
            szBuffer[nLength] = '\0';
            printf("Received %s from %s\n", szBuffer,
                   inet_ntoa(pClientInfo->clientAddr.sin_addr));

            // Convert the string to upper case and send it back, if its not QUIT
            strupr(szBuffer);
            if (strcmp(szBuffer, "QUIT") == 0) {
                closesocket(pClientInfo->hClientSocket);
                return TRUE;
            }
            // send( ) may not be able to send the complete data in one go.
            // So try sending the data in multiple requests
            int nCntSend = 0;
            char *pBuffer = szBuffer;

            while ((nCntSend = send(pClientInfo->hClientSocket, pBuffer,
                    nLength, 0) != nLength)) {
                if (nCntSend == -1) {
                    printf("Error sending the data to %s\n",
                           inet_ntoa(pClientInfo->clientAddr.sin_addr));
                    break;
                }
                if (nCntSend == nLength)
                    break;

                pBuffer += nCntSend;
                nLength -= nCntSend;
            }
        } else {
            printf("Error reading the data from %s\n",
                   inet_ntoa(pClientInfo->clientAddr.sin_addr));
        }
    }

    return TRUE;
}
