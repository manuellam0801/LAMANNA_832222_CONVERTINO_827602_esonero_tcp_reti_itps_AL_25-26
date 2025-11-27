/*
 * main.c
 *
 * TCP Server - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP server
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
typedef unsigned int socklen_t;
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "protocol.h"

#define NO_ERROR 0

void clearwinsock()
{
	#if defined WIN32
		WSACleanup();
	#endif
}

int valid_type(const char c)
{
    return (c == 't' || c=='h' || c=='w' || c=='p');
}

// Verifica se la città è supportata
int supported_city(const char *city)
{
    const char *supported_cities[]={
        "Bari", "Roma", "Milano", "Napoli", "Torino",
        "Palermo", "Genova", "Bologna", "Firenze", "Venezia"};

	int num= sizeof(supported_cities) / sizeof(supported_cities[0]);

    char city_lower[64];
    strncpy(city_lower, city, sizeof(city_lower) - 1);
    city_lower[sizeof(city_lower) - 1] = '\0';
      for (int i = 0; city_lower[i]; i++)
		{
        	city_lower[i] = tolower((unsigned char)city_lower[i]);
    	}

		city_lower[0]=toupper((unsigned char)city_lower[0]);

    for (int i = 0; i < num; i++)
	{
        if (strcmp(city_lower, supported_cities[i]) == 0)
		{
            return 1;
        }
    }
    return 0;
}

void errorhandler(char *errorMessage)
{
	printf("%s", errorMessage);
}

//meteo functions
float get_temperature()
{
	return -10.0f + (rand() % 501) / 10.0f; //range fra -10.0 e 40.0 °c
}

float get_humidity()
{
	return 20.0f + (rand() % 81); // range 20.0 – 100.0 %
}

float get_wind()
{
	return ((float)(rand()%1001))/10.0; //range fra 0.0 e 100.0 km/h
}

float get_pressure()
{
	return 950.0f + (rand() % 101); // range 950.0 – 1050.0 hPa
}


int main(int argc, char *argv[])
{

	srand(time(NULL));
	char server_ip[64]="127.0.0.1";
	int port=SERVER_PORT;
	weather_response_t resp;//struct risposta del server
    memset(&resp, 0, sizeof(weather_response_t));
    weather_request_t req;//struct di richiesta del client
	memset(&req,0,sizeof(weather_request_t));
	// Initialize Winsock
    #if defined WIN32
		WSADATA wsa_data;
		int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
		if (result != NO_ERROR)
		{
			printf("Errore in WSAStartup()\n");
			return 0;
		}
    #endif

	if (argc > 2 && strcmp(argv[1], "-p") == 0)
	{
	        port = atoi(argv[2]);
	}

	//Create socket
	int my_socket;
	my_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(my_socket<0)
	{
		printf("creazione della socket fallita.\n");
		return -1;
	}


	//Configure server address
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(server_ip);
	sad.sin_port = htons(port);

	//Bind socket
	if(bind(my_socket, (struct sockaddr*) &sad, sizeof(sad))<0)
	{
		printf("bind() fallita\n");
		return -1;
	}

	//Set socket to listen
	if(listen(my_socket, QUEUE_SIZE)<0)
	{
		printf("listen() fallita\n");
		return -1;
	}

	//Implement connection acceptance loop
	struct sockaddr_in cad; // structure for the client address
	int client_socket;       // socket descriptor for the client
	int client_len;          // the size of the client address
	while (1)
    {
		client_len = sizeof(cad); // set the size of the client address
		if ((client_socket = accept(my_socket, (struct sockaddr*) &cad,(socklen_t*)&client_len)) < 0)
        {
			errorhandler("accept() fallita.\n");
			// close connection
			closesocket(client_socket);
			continue;  //al posto di return 0; e clearwinsock
		}

    //ricezione messaggio client
		if ((recv(client_socket,(char*)&req, sizeof(req), 0)) <= 0)
        {
			errorhandler("recv() fallita.\n");
			closesocket(client_socket);
			continue; //al posto di return -1
		}
    //visualizza il messaggio: Richiesta '<type><city?>' dal client ip <ip_client>
    printf("Richiesta %c %s dal client ip %s\n",req.type,req.city, inet_ntoa(cad.sin_addr));
    //validazione richiesta

if (!valid_type(req.type))
        {
            resp.status = STATUS_REQUEST_NOT_VALID;
            resp.type = '\0';
            resp.value = 0.0f;
        } else if (!supported_city(req.city))
		{
            resp.status = STATUS_CITY_NOT_AVAILABLE;
            resp.type = '\0';
            resp.value = 0.0f;
        }
        else
        {
            // Richiesta valida
            resp.status = STATUS_SUCCESS;
            resp.type = req.type;
            switch (req.type)
            {
            case 't':
            	resp.value = get_temperature();
            break;
            case 'h':
            	resp.value = get_humidity();
            break;
            case 'w':
            	resp.value = get_wind();
            break;
            case 'p':
            	resp.value = get_pressure();
            break;
            }
        }
    //invio risposta
    if ((send(client_socket,(char*)&resp,sizeof(resp),0)) <= 0)
        {
			errorhandler("send() fallita\n");
			closesocket(client_socket);
			continue; //return-1;
		}
        // Chiudi connessione client
        closesocket(client_socket);

}
    //chiusura della connessione
	printf("Chiusura server.\n");
	closesocket(my_socket);
	clearwinsock();
	return 0;
} // main end
