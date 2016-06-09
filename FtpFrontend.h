/* 
 * @file   ftpFrontend.h
 * @brief  Implements user interactions for FTP client. The prompt and parsing
 *          of commands is meant to be similar to that of a Unix FTP client.
 * @author Brendan Sweeney, ID #1161836
 * @date   December 13, 2012
 */

#ifndef FTPFRONTEND_H
#define	FTPFRONTEND_H

#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <string>
#include "FtpBackend.h"

using namespace std;


class FtpFrontend {
public:
    FtpFrontend();
    FtpFrontend(char *host);
    void run(void);
private:
    const  string PROMPT;
    enum   action {OPEN, CD, LS, GET, PUT, CLOSE, QUIT, UNKNOWN, DEFAULT};
    bool   opened, authed;
    string command, hostname, username, param1, param2;
    FtpBackend backend;     // handle all server communication
    
    int readInput(void);
    void authenticate(void);
}; // end class FtpFrontend

#endif	/* FTPFRONTEND_H */
