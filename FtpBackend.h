/* 
 * @file   ftpBackend.cpp
 * @brief  Implements server interaction for the FTP client. Commands from the
 *          frontend are formed into FTP control commands and relayed to the
 *          server. Where possible, a response from the server is passed back
 *          to the frontend to be displayed to the user.
 * @author Brendan Sweeney, ID #1161836
 * @date   December 13, 2012
 */

#ifndef FTPBACKEND_H
#define	FTPBACKEND_H

#include <arpa/inet.h>      // inet_ntoa
#include <netinet/in.h>     // htonl, htons, inet_ntoa
#include <netinet/tcp.h>    // TCP_NODELAY
#include <sys/socket.h>     // socket, bind, listen, inet_ntoa
#include <sys/stat.h>
#include <sys/types.h>      // socket, bind
#include <sys/uio.h>        // writev
#include <sys/wait.h>       // for wait
#include <fcntl.h>          // fcntl
#include <netdb.h>          // gethostbyname
#include <poll.h>
#include <signal.h>         // sigaction
#include <stdio.h>          // for NULL, perror
#include <string.h>
#include <unistd.h>         // read, write, close
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include "Timer.h"

using namespace std;


class FtpBackend {
public:
    // FTP message codes
    static const int POS_PRE   = 1,
                     POS_COMPL = 2,
                     POS_INTER = 3,
                     NEG_TRANS = 4,
                     NEG_PERM  = 5;
    FtpBackend();
    string ftpOpen(string hostname, string port);
    string ftpUser(string username);
    string ftpPass(string password);
    string ftpCd(string subdir);
    string ftpLs(void);
    string ftpGet(string filename, string newname);
    string ftpPut(string filename, string newname);
    string ftpClose(void);
    string ftpQuit(void);
private:
    static const int DEF_PORT_NUM = 21,
                     BUFLEN       = 1448;
    int    portNum;                     // a server port number
    int    clientSd;                    // for the client-side socket
    struct hostent *host;               // for resolved server from host name
    struct sockaddr_in sendSockAddr;    // address data structure
    
    string ftpOpen(char *hostname, int port, int& sd);
    string reply(int delay);
    string pasv(char address[], int& port);
}; // end class FtpBackend

#endif	/* FTPBACKEND_H */
