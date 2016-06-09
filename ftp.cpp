/* 
 * @file   ftp.cpp
 * @brief  Simple driver to run the FTP client.
 * @author Brendan Sweeney, ID #1161836
 * @date   December 13, 2012
 */

#include <cstdlib>
#include "FtpBackend.h"
#include "FtpFrontend.h"

using namespace std;


int main(int argc, char** argv) {
    FtpFrontend go;
    
    go.run();
    
    return 0;
} // end main(int, char**)
