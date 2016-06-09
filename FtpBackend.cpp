/* 
 * @file   ftpBackend.cpp
 * @brief  Implements server interaction for the FTP client. Commands from the
 *          frontend are formed into FTP control commands and relayed to the
 *          server. Where possible, a response from the server is passed back
 *          to the frontend to be displayed to the user.
 * @author Brendan Sweeney, ID #1161836
 * @date   December 13, 2012
 */

#include "FtpBackend.h"


FtpBackend::FtpBackend() : host(NULL) {
} // end default constructor


// converts strings to host address and numeric port
string FtpBackend::ftpOpen(string hostname, string port) {
    char serverIp[hostname.length() + 1];

    // store arguments in local variables
    try {
        hostname.copy(serverIp, hostname.length(), 0);
        serverIp[hostname.length()] = '\0';
    } catch (exception& e) {
        cerr << e.what() << endl;
    } // end try hostname.copy()
    
    return ftpOpen(serverIp, atoi(port.c_str()), clientSd);
} // end ftpOpen(string, string)


// establishes TCP connection to a host on a given port and socket descriptor
string FtpBackend::ftpOpen(char *hostname, int port, int& sd) {
    host    = gethostbyname(hostname);
    portNum = port;

    // ensure valid port
    if (portNum < 1024 || portNum > 65535) {
        portNum = DEF_PORT_NUM;
    } // end if (portNum < 1024...)

    // only continue if host name could be resolved
    if (!host)
    {
        cerr << "unknown hostname: " << hostname << endl;
        exit(EXIT_FAILURE);
    } // end if (!host)

    // build address data structure
    memset((char *)&sendSockAddr, '\0', sizeof(sendSockAddr));
    sendSockAddr.sin_family = AF_INET;
    sendSockAddr.sin_addr.s_addr =
        inet_addr(inet_ntoa(*(struct in_addr *) *host->h_addr_list));
    sendSockAddr.sin_port = htons(portNum);

    // active open, ensure success before continuing
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cerr << "socket failure" << endl;
        exit(EXIT_FAILURE);
    } // end if ((clientSd = socket(...)))

    // only continue if socket connection could be established
    if (connect(sd,
                (sockaddr *)&sendSockAddr,
                sizeof(sendSockAddr)) < 0)
    {
        cerr << "connect failure" << endl;
        close(sd);
        exit(EXIT_FAILURE);
    } // end if (connect(...) < 0)

    return reply(clientSd);
} // end ftpOpen(char*, int)


// sends a user name to the server for authentication
string FtpBackend::ftpUser(string username) {
    char sendCmd[username.length() + 7];
    
    strcpy(sendCmd, "USER ");
    strcat(sendCmd, username.c_str());
    strcat(sendCmd, "\r\n");
    write(clientSd, sendCmd, sizeof(sendCmd));
    
    return reply(clientSd);
} // end ftpUser(string)


// sends a password to the server for authentication
string FtpBackend::ftpPass(string password) {
    char sendCmd[password.length() + 7];
    
    strcpy(sendCmd, "PASS ");
    strcat(sendCmd, password.c_str());
    strcat(sendCmd, "\r\n");
    write(clientSd, sendCmd, sizeof(sendCmd));
    
    // return host system information
    string temp = reply(clientSd);
    temp.append(reply(clientSd));
    write(clientSd, "SYST\r\n", 6);
    temp.append(reply(clientSd));
    
    return temp;
} // end ftpPass(string)


// changes working directory on the server
string FtpBackend::ftpCd(string subdir) {
    char sendCmd[subdir.length() + 6];
    
    strcpy(sendCmd, "CWD ");
    strcat(sendCmd, subdir.c_str());
    strcat(sendCmd, "\r\n");
    write(clientSd, sendCmd, sizeof(sendCmd));
    
    return reply(clientSd);
} // end ftpCd()


// lists current directory contents from the server
string FtpBackend::ftpLs(void) {
    char   address[15];
    int    port;
    int    dataSd;
    int    pid;
    string message(pasv(address, port));
    // open data connection
    message.append(ftpOpen(address, port, dataSd));
    
    // only continue if child successfully forked
    if ((pid = fork()) < 0)
    {
        cerr << "ftpLs(): fork failed" << endl;
    } // end if ((pid = fork()) < 0)
    else if (pid > 0)
    {
        // This is the parent; wait for the child
        cout << message;
        write(clientSd, "LIST\r\n", 6);
        wait(NULL); // wait for child
    } // end else if (pid > 0)
    else
    {
        cout << reply(clientSd);
        do {    // avoid incomplete listing
            message = reply(dataSd);
            cout << message;
        } while(message.at(message.length() - 1) != '\n');
        exit(0);
    } // end else (pid == 0)
    
    close(dataSd);
    return reply(clientSd);
} // end ftpLs()


// download a file from the server and store it locally
string FtpBackend::ftpGet(string filename, string newname) {
    char   address[15];
    int    port;
    int    dataSd;
    int    pid;
    string message(pasv(address, port));
    // open data connection
    message.append(ftpOpen(address, port, dataSd));
    
    // only continue if child successfully forked
    if ((pid = fork()) < 0)
    {
        cerr << "ftpLs(): fork failed" << endl;
    } // end if ((pid = fork()) < 0)
    else if (pid > 0)
    {
        // This is the parent; wait for the child
        char sendCmd[filename.length() + 8];

        cout << message;
        strcpy(sendCmd, "RETR ");
        strcat(sendCmd, filename.c_str());
        strcat(sendCmd, "\r\n");
        write(clientSd, sendCmd, sizeof(sendCmd));
        wait(NULL); // wait for child
    } // end else if (pid > 0)
    else
    {
        long   time;
        Timer  tick;
        int    count = 0;
        mode_t mode  = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        int    file  = open(newname.c_str(), O_WRONLY | O_CREAT, mode);
        char   buffer[255];
        
        tick.start();
        cout << (reply(clientSd));
        
        while(true) {
            int l = read(dataSd, buffer, sizeof(buffer));
            if (l == 0)
                break;
            count += l;
            write(file, buffer, l);
        } // end while(true)
        
        close(file);
        time = tick.lap();
        cout << count << " bytes received in " << (double)time / 1000000.0
             << " seconds (" << 1000.0 * (double)count / time
             << " Kbytes/s)" << endl;
        exit(0);
    } // end else (pid == 0)
    
    close(dataSd);
    return reply(clientSd);
} // end ftpGet()


// upload a local file to the server
string FtpBackend::ftpPut(string filename, string newname) {
    char   address[15];
    int    port;
    int    dataSd;
    int    pid;
    string message(pasv(address, port));
    // open data connection
    message.append(ftpOpen(address, port, dataSd));
    
    // only continue if child successfully forked
    if ((pid = fork()) < 0)
    {
        cerr << "ftpLs(): fork failed" << endl;
    } // end if ((pid = fork()) < 0)
    else if (pid > 0)
    {
        // This is the parent; wait for the child
        char sendCmd[newname.length() + 8];

        cout << message;
        strcpy(sendCmd, "STOR ");
        strcat(sendCmd, newname.c_str());
        strcat(sendCmd, "\r\n");
        write(clientSd, sendCmd, sizeof(sendCmd));
        wait(NULL); // wait for child
    } // end else if (pid > 0)
    else
    {
        long  time;
        Timer tick;
        int   count = 0;
        int   file = open(filename.c_str(), O_RDONLY);
        char  buffer[255];
        
        tick.start();
        cout << (reply(clientSd));
        
        while(true) {
            int l = read(file, buffer, sizeof(buffer));
            if (l == 0)
                break;
            count += l;
            write(dataSd, buffer, l);
        } // end while(true)
        
        close(file);
        time = tick.lap();
        cout << count << " bytes sent in " << (double)time / 1000000.0
             << " seconds (" << 1000.0 * (double)count / time 
             << " Kbytes/s)" << endl;
        exit(0);
    } // end else (pid == 0)
    
    close(dataSd);
    return reply(clientSd);
} // end ftpPut()


// closes an active connection to the server
string FtpBackend::ftpClose(void) {
    write(clientSd, "QUIT\r\n", 6);
    string message = reply(clientSd);
    close(clientSd);
    return message;
} // end ftpClose()


// obtains a reply from a server and returns it as a string
string FtpBackend::reply(int sd) {
    struct pollfd ufds;
    ufds.fd      = sd;      // a socket descriptor to exmaine for read
    ufds.events  = POLLIN;  // check if this sd is ready to read
    ufds.revents = 0;       // simply zero-initialized
    string reply = "";
    int    nread;
    int    val = poll(&ufds, 1, 1000);
    
    if (val > 0) {                      // the socket is ready to read
        char buffer[BUFLEN];
        nread = read(sd, buffer, BUFLEN);
        reply.append(buffer, nread);
    } // end if (val > 0)
    
    return reply;
} // end reply()


// sends a passive command to the server and parses out the address and port
string FtpBackend::pasv(char address[], int &port) {
    int    index  = 0;
    int    touple = 0;
    int    offset = 0;
    
    write(clientSd, "PASV\r\n", 6);
    string temp = reply(clientSd);
    
    try {
        if (atoi(&temp.at(0)) / 100 == POS_COMPL) {
            offset = temp.find('(', 0) + 1;

            while(index < 15) {
                if (temp.at(offset + index) == ',') {
                    if (touple == 3) {
                        address[index] = '\0';
                        break;
                    }
                    address[index] = '.';
                    ++touple;
                } // end if (temp.at(offset + index) == ',')
                else {
                    address[index] = temp.at(offset + index);
                } // end else
                ++index;
            } // end while(index < 15)

            port  = atoi(&temp.at(offset + index + 1)) * 256;
            index = temp.rfind(',', temp.length() - 1) + 1;
            port += atoi(&temp.at(index));
        } // end if (atoi(&temp.at(0))...)
    } catch (exception& e) {
        cerr << e.what() << endl;
    } // end try atoi()
    
    return temp;
} // end pasv(char*, int)
