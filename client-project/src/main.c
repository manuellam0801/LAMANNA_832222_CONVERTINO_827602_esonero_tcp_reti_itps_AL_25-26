/*
 * main.c
 *
 * TCP Client - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP client
 * portable across Windows, Linux and macOS.
 */

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

#define NO_ERROR 0

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

void usage (const char* progname)
	{
	printf("Uso: %s [-s server] [-p port] -r \"type city\"\n",progname);
	}

void errorhandler(char *errorMessage)
{
	printf("%s", errorMessage);
}

int main(int argc, char *argv[])
{

	//Implement client logic

	char server_ip[64]="127.0.0.1";
	int port=SERVER_PORT;
	weather_response_t resp;//struct risposta al server
	memset(&resp, 0, sizeof(weather_response_t));
	weather_request_t req;//struct di richiesta al client
	memset(&req,0,sizeof(weather_request_t));


#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif


	for (int i=0;i<argc;i++)
	{
		if(strcmp(argv[i],"-s")==0 && i+1<argc)
		{
			strcpy(server_ip,argv[++i]);
		}

		else if(strcmp(argv[i],"-p")==0 && i+1<argc)
		{
			port=atoi(argv[++i]);
		}
		else if(strcmp(argv[i],"-r")==0 && i+1<argc)
		{
			if (sscanf(argv[++i]," %c %63s", &req.type, req.city) !=2)
			{
				usage(argv[0]);
				return 1;
			}
			for (int j=0;req.city[j];++j)
			{
				req.city[j]=tolower((unsigned char)(req.city[j]));
			}

			req.city[0]=toupper((unsigned char)(req.city[0]));
		}
	}

	if(req.type ==0 || strlen(req.city)==0)
	{
		usage(argv[0]);
		return 1;
	}


	//Create socket
	int c_socket;

	c_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(c_socket<0)
	{
		printf("Creazione della socket cliente fallita.\n");
		return -1;
	}

	//Configure server address
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(server_ip); // IP del server
	sad.sin_port = htons(port); // Server port

	//Connect to server
	if (connect(c_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		errorhandler("Connessione fallita.\n");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}

	//Implement communication logic
	if ((send(c_socket,(char*)&req, sizeof(req), 0)) <= 0) {
		errorhandler("send() fallita.\n");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}
	if ((recv(c_socket, (char*)&resp, sizeof(resp), 0)) <= 0) {
		errorhandler("recv() fallita.\n");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}
	if (resp.status==0)
	{
		switch (resp.type)
		{
			case 't':
				printf("Ricevuto risultato dal server ip %s. %s: Temperatura = %.1f%cC\n", server_ip,req.city, resp.value,248);
			break;

			case 'h':
				printf("Ricevuto risultato dal server ip %s. %s: Umidita' = %.1f%% \n", server_ip, req.city, resp.value);
				break;

			case 'w':
				printf("Ricevuto risultato dal server ip %s. %s: Vento = %.1f km/h\n", server_ip,req.city, resp.value);
				break;

			case 'p':
				printf("Ricevuto risultato dal server ip %s. %s: Pressione = %.1f hPa\n", server_ip,req.city, resp.value);
				break;
		}
	}
	else if(resp.status==1)
	{
		printf("Ricevuto risultato dal server ip %s. Citta' non disponibile\n",server_ip);
	}
	else
	{
		printf("Ricevuto risultato dal server ip %s. Richiesta non valida\n",server_ip);
	}

	//Close socket
	closesocket(c_socket);
	printf("Client terminato.\n");
	clearwinsock();
	return 0;
} // main end
