/* 
 * @file   ftpFrontend.cpp
 * @brief  Implements user interactions for FTP client. The prompt and parsing
 *          of commands is meant to be similar to that of a Unix FTP client.
 * @author Brendan Sweeney, ID #1161836
 * @date   December 13, 2012
 */

#include "FtpFrontend.h"


FtpFrontend::FtpFrontend() : PROMPT("ftp> "), opened(false), authed(false),
                             command(""),     hostname(""),  username(""),
                             param1(""),      param2(""),    backend() {
} // end default constructor


// when given a hostname, a connection should be immediately established
FtpFrontend::FtpFrontend(char *host) : PROMPT("ftp> "), backend(),
                                       hostname(host),  command(""),
                                       param1(""),      param2("") {
    string reply(backend.ftpOpen(hostname, "21"));
    cout << reply;
    try {
        opened = atoi(&reply.at(0)) / 100 == FtpBackend::POS_COMPL;

        if (opened) {
            authenticate();
        } // end if (opened)
    } catch (exception& e) {
        cerr << e.what() << endl;
        opened   = false;
        authed   = false;
        username = "";
    } // end try atoi()
} // end constructor


// perform user tasks until user is done
void FtpFrontend::run() {
    bool   done = false;
    string reply;   // for response from server
    
    while(!done) {
        // get action code based on user input; also sets data members
        switch (readInput()) {
            case OPEN:
                reply = backend.ftpOpen(hostname, param1);
                cout << reply;
                try {
                    opened = atoi(&reply.at(0)) / 100
                            == FtpBackend::POS_COMPL;

                    if (opened) {
                        authenticate();
                    } // end if (opened)
                } catch (exception& e) {
                    cerr << e.what() << endl;
                    opened = false;
                    authed = false;
                } // end try atoi()
                break;
            case CD:
                reply = backend.ftpCd(param1);
                cout << reply;
                break;
            case LS:
                reply = backend.ftpLs();
                cout << reply;
                break;
            case GET:
                reply = backend.ftpGet(param1, param2);
                cout << reply;
                break;
            case PUT:
                reply = backend.ftpPut(param1, param2);
                cout << reply;
                break;
            case CLOSE:
                reply = backend.ftpClose();
                cout << reply;
                opened = false;
                authed = false;
                break;
            case QUIT:
                if (opened) {
                    reply = backend.ftpClose();
                    cout << reply;
                } // end if (opened)
                done = true;
                break;
            case UNKNOWN:
                cerr << "Unrecognized command: " << command << endl;
                break;
            // paranoid: my switches break when I don't have a default
            default:
                break;
        } // end switch (command)
    } // end while(!done)
} // end run()


// read user input and set member variables accordingly; return an action code
int FtpFrontend::readInput() {
    command = "";       // first item read from command line
    cout << PROMPT;
    cin  >> command;
    
    if (command.compare("open") == 0) {
        if (opened) {
            cerr << "Already connected to " << hostname
                 << ", use close first."    << endl;
            return DEFAULT;
        } // end if (opened)
        else if (cin.get() != '\n') {
            cin >> hostname;
            if (cin.get() != '\n') {
                cin >> param1;
            } // end if (cin.get() != '\n')
            else {
                param1 = "21";  // use default port
            } // end else (cin.get() == '\n')
        } // end else if (cin.get() != '\n')
        else {
            cout << "(to) ";
            cin  >> hostname;
            param1 = "21";
        } // end else (cin.get() == '\n')
        
        return OPEN;
    } // end if (command.compare("open") == 0)
    else if (command.compare("cd") == 0) {
        if (!opened) {
            cerr << "Not connected." << endl;
            return DEFAULT;
        } // end if (!opened)
        
        param1 = ".";   // default to current directory
        
        if (cin.get() != '\n') {
            cin >> param1;
        } // end if (cin.get() != '\n')
        else {
            cout << "(remote-directory) ";
            cin  >> param1;
        } // end else ()
        
        return CD;
    } // end else if (command.compare("cd") == 0)
    else if (command.compare("ls") == 0) {
        if (!opened) {
            cerr << "Not connected." << endl;
            return DEFAULT;
        } // end if (!opened)
        
        return LS;
    } // end else if (command.compare("ls") == 0)
    else if (command.compare("get") == 0) {
        if (!opened) {
            cerr << "Not connected." << endl;
            return DEFAULT;
        } // end if (!opened)
        
        if (cin.get() != '\n') {
            cin >> param1;
            if (cin.get() != '\n') {
                cin >> param2;
            } // end if (cin.get() != '\n')
            else {
                param2 = param1;
            } // end else (cin.get() == '\n')
        } // end if (cin.get() != '\n')
        else {
            cout << "(remote-file) ";
            cin  >> param1;
            cout << "(local-file) ";
            cin  >> param2;
        
            if (param2.compare("") == 0) {
                param2 = param1;
            } // end if (param2.compare("") == 0)
        } // end else (cin.get() == '\n')
        
        return GET;
    } // end else if (command.compare("get") == 0)
    else if (command.compare("put") == 0) {
        if (!opened) {
            cerr << "Not connected." << endl;
            return DEFAULT;
        } // end if (!opened)
        
        if (cin.get() != '\n') {
            cin >> param1;
            if (cin.get() != '\n') {
                cin >> param2;
            } // end if (cin.get() != '\n')
            else {
                param2 = param1;
            } // end else (cin.get() == '\n')
        } // end if (cin.get() != '\n')
        else {
            cout << "(local-file) ";
            cin  >> param1;
            cout << "(remote-file) ";
            cin  >> param2;
            
            if (param2.compare("") == 0) {
                param2 = param1;
            } // end if (param2.compare("") == 0)
        } // end else (cin.get() == '\n')
        
        return PUT;
    } // end else if (command.compare("put") == 0)
    else if (command.compare("close") == 0) {
        if (!opened) {
            cerr << "Not connected." << endl;
            return DEFAULT;
        } // end if (!opened)
        
        return CLOSE;
    } // end else if (command.compare("close") == 0)
    else if (command.compare("quit") == 0) {
        return QUIT;
    } // end else if (command.compare("quit") == 0)
    
    return UNKNOWN;
} // end readInput()


// securely get user authentication after open()
void FtpFrontend::authenticate(void) {
    string reply, userString("<nullPtr>");
    int    pid;
    
    if (getlogin() != NULL) {   // does not work on my computer...
        userString = getlogin();
    } // end if (getlogin() != NULL)
    
    cout << "Name (" << hostname << ":" << userString << "): ";
    cin  >> username;
    reply = backend.ftpUser(username);
    cout << reply;
    
    try {
        if (atoi(&reply.at(0)) / 100 == FtpBackend::POS_INTER) {
            cout << "Password:";
            // turn off echo for password entry
            if ((pid = fork()) < 0)
            {
                cerr << "authenticate(): fork failed" << endl;
            } // end if ((pid = fork()) < 0)
            else if (pid > 0)
            {
                wait(NULL); // wait for child
            } // end else if (pid > 0)
            else
            {
                execlp("/bin/stty", "stty", "-echo", NULL);
            }
            
            cin  >> param1;
            
            // turn echo back on
            if ((pid = fork()) < 0)
            {
                cerr << "authenticate(): fork failed" << endl;
            } // end if ((pid = fork()) < 0)
            else if (pid > 0)
            {
                wait(NULL); // wait for child
                cout << endl;
            } // end else if (pid > 0)
            else
            {
                execlp("/bin/stty", "stty", "echo", NULL);
            } // end else ((pid = fork()) >= 0)
            
            reply = backend.ftpPass(param1);
            cout << reply;
        } // end if (atoi(reply.at(0)) == FtpBackend::POS_INTER)
    
        // ensure authentication succeeded
        authed = atoi(&reply.at(0)) / 100 == FtpBackend::POS_COMPL;
    } catch (exception& e) {
        cerr << e.what() << endl;
        opened = false;
        authed = false;
    } // end try atoi()
} // end authenticate()
