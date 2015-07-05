#include <stdio.h>
#include <winsock2.h>

int InitWinSock2_0(void);

int main(void)
{
    char szServerIPAddr[20];     // Put here the IP address of the server
    int nServerPort = 5050; // The server port that will be used by
    // clients to talk with the server
    printf("Enter the server IP Address: ");
    scanf("%s", szServerIPAddr);
    printf("Enter the server port number: ");
    scanf("%d", &nServerPort);
	if (!InitWinSock2_0()) {
        printf("Unable to Initialize Windows Socket environment %d\n",
               WSAGetLastError());
        return -1;
	}
	SOCKET hClientSocket;
	hClientSocket = socket(AF_INET, // The address family. AF_INET specifies TCP/IP
			SOCK_STREAM,    // Protocol type. SOCK_STREM specified TCP
			0            // Protoco Name. Should be 0 for AF_INET address family
			);
	if (hClientSocket == INVALID_SOCKET) {
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
	// Connect to the server
	if (connect(hClientSocket, (struct sockaddr *) &serverAddr,
			sizeof(serverAddr)) < 0) {
        printf("Unable to connect to %s on port %d\n", szServerIPAddr, nServerPort);
        closesocket(hClientSocket);
		WSACleanup();
		return -1;
	}
	char szBuffer[1024] = "";
	while (strcmp(szBuffer, "QUIT") != 0) {
        printf("Enter the string to send (QUIT) to stop: ");
        scanf("%s", szBuffer);
		int nLength = strlen(szBuffer);
		// send( ) may not be able to send the complete data in one go.
		// So try sending the data in multiple requests
		int nCntSend = 0;
		char *pBuffer = szBuffer;
		while ((nCntSend = send(hClientSocket, pBuffer, nLength, 0) != nLength)) {
			if (nCntSend == -1) {
                printf("Error sending the data to server\n");
				break;
			}
            if (nCntSend == nLength) {
				break;
            }
			pBuffer += nCntSend;
			nLength -= nCntSend;
		}
		strupr(szBuffer);
		if (strcmp(szBuffer, "QUIT") == 0) {
			break;
		}
		nLength = recv(hClientSocket, szBuffer, sizeof(szBuffer), 0);
		if (nLength > 0) {
			szBuffer[nLength] = '\0';
            printf("Received %s from server\n", szBuffer);
        }
	}
	closesocket(hClientSocket);
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
