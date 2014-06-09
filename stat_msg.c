#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

static const char* PORT = "8383";
static const int   MSG_LENGTH = 256;

void
error(char *msg)
{
    perror(msg);
    exit(1);
}

int
main(int argc, char *argv[])
{
    int    my_socket;
    int    port;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char   buffer[MSG_LENGTH];

    my_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (my_socket < 0)
        error("could not create my_socket");

    bzero((char *) &server_addr, sizeof(server_addr));
    port = atoi(PORT);

    server = gethostbyname("127.0.0.1");

    if (server == NULL)
        error("could not resolve hostname");

    server_addr.sin_family = AF_INET;
    bcopy( (char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr,
                                                           server->h_length );
    server_addr.sin_port = htons(port);

    if ( connect(my_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        error("could not connect");

    bzero(buffer,MSG_LENGTH);

    if ( argc < 2 )
    {
        fgets(buffer, MSG_LENGTH - 1, stdin);
        buffer[strlen(buffer)-1] = '\0';
    }
    else
        sscanf(argv[1], "%s", buffer);

    if ( write(my_socket, buffer, strlen(buffer) ) < 0 )
        error("could not write");

    bzero(buffer,MSG_LENGTH);
    if ( read(my_socket,buffer,255) < 0 )
        error("could not read");

    printf("%s\n", buffer);

    return 0;
}
