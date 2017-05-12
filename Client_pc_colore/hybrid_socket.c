#include "hybrid_socket.h"

int close_socket_wrapper(HYBRID_SOCKET sd)
{
    #ifdef WINDOWS
        int code = closesocket((SOCKET) sd); // SPECIFICO PER WINDOWS, funziona solo su socket
                                         // e non su file descriptor come la close(fd) di unix
        WSACleanup();
        return code;
    #endif // WINDOWS

    #ifdef LINUX
        int code = close((int) sd);
    #endif // LINUX
}

HYBRID_SOCKET init_socket_wrapper(long buffer, struct sockaddr_in *clientaddr, struct sockaddr_in *servaddr)
{
    #ifdef WINDOWS

        WSADATA wsa;
        //Initialise winsock
        printf("[INFO] Initialising sockets\n");
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        {
            printf("[ERROR] Failed: error code %d", WSAGetLastError());
            exit(EXIT_FAILURE);
        }

        /* PREPARAZIONE INDIRIZZO CLIENT E SERVER ----------------------------- */
        memset((char *)clientaddr, 0, sizeof(struct sockaddr_in));
        (*clientaddr).sin_family = AF_INET;
        (*clientaddr).sin_addr.s_addr = INADDR_ANY;
        (*clientaddr).sin_port = htons(5555);

        /* CREAZIONE SOCKET ---------------------------- */
        SOCKET sd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sd == INVALID_SOCKET) { perror("[ERROR] Opening socket"); exit(3); }

        printf("[INFO] Created socket sd = %d\n", (int) sd);

        /* BIND SOCKET --------------- */
        if (bind(sd, (struct sockaddr *) clientaddr, sizeof(*clientaddr)) == SOCKET_ERROR) //dubbio puntatore
        {
            perror("[ERROR] Bind socket failed\n");
            exit(1);
        }
        printf("[INFO] Binding socket completed, port %i\n", ntohs((*clientaddr).sin_port));

        if (setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (char*)&buffer, 8) == -1) {
            fprintf(stderr, "[ERROR] Setting socket opts: %d\n", WSAGetLastError());
        }
        else printf("[INFO] Setting sockets opts completed\n");
    #endif // WINDOWS
    #ifdef LINUX
        printf("[INFO] Initialising sockets\n");

        /* PREPARAZIONE INDIRIZZO CLIENT E SERVER ----------------------------- */
        memset((char *)clientaddr, 0, sizeof(struct sockaddr_in));
        (*clientaddr).sin_family = AF_INET;
        (*clientaddr).sin_addr.s_addr = INADDR_ANY;
        (*clientaddr).sin_port = htons(5555);

        /* CREAZIONE SOCKET ---------------------------- */
        int sd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sd < 0) { perror("[ERROR] Opening socket"); exit(3); }

        printf("[INFO] Created socket sd = %d\n", (int) sd);

        /* BIND SOCKET --------------- */
        if (bind(sd, (struct sockaddr *) clientaddr, sizeof(*clientaddr)) < 0) //dubbio puntatore
        {
            perror("[ERROR] Bind socket failed\n");
            exit(1);
        }
        printf("[INFO] Binding socket completed, port %i\n", ntohs((*clientaddr).sin_port));

        if (setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (char*)&buffer, 8) == -1) {
            fprintf(stderr, "[ERROR] Setting socket opts\n");
        }

    #endif // LINUX

    return (HYBRID_SOCKET) sd;
}

BYTES_NUM recvfrom_socket_wrapper(HYBRID_SOCKET s, void *buf, int len, int flags, struct sockaddr *from, int *fromlen)
{
    #ifdef WINDOWS
        return (BYTES_NUM) recvfrom((SOCKET) s, (char*)buf, len, flags, (struct sockaddr *)&from, fromlen);
    #endif // WINDOWS
    #ifdef LINUX
        return (BYTES_NUM) recvfrom((int) s, (char*)buf, len, flags, (struct sockaddr *)&from, (socklen_t*) fromlen);
        // return ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
    #endif // LINUX
}
