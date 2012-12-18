#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  //inet_pton
#include <errno.h>
#include <stdio.h>

#include <sstream>

struct IP
{
    std::string proxyIp;
    unsigned int         proxyPort;
    std::string targetIp;
    unsigned int         targetPort;
};


void parseParameter(IP& ip, int argc, char* argv[])
{
    if (argc == 5)
    {
        std::stringstream ss;
        ip.proxyIp    = argv[1];

        ss << argv[2];
        ss >> ip.proxyPort;

        ip.targetIp   = argv[3];

        ss.clear();
        ss << argv[4];
        ss >> ip.targetPort;
    }
    else
    {
        std::cout << "Parameter missing! :(\n";
    }
}


int connectToSocket(std::string destIp, int destPort)
{
    int socketFd = socket(PF_INET, SOCK_STREAM, 0);
    if (socketFd == -1)
    {
        perror("socket");
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, destIp.c_str(), &(serverAddr.sin_addr));
    serverAddr.sin_port = htons(destPort);

    if (-1 == connect(socketFd, (struct sockaddr*)&serverAddr, sizeof(sockaddr_in)))
    {
        perror("connect");
    }
    else
    {
        std::cout << "Connected to server (" << destIp << ":" << destPort << ")\n";
    }

    return socketFd;
}


std::string createConnectionString(std::string destIp, int destPort)
{
    std::string destPortStr;
    std::stringstream ss;
    ss << destPort;
    ss >> destPortStr;

    std::string result = "CONNECT ";
    result += destIp;
    result += ":";
    result += destPortStr;
    result += " HTTP/1.0\n\n";

    return result;
}


bool remoteConnect(int socket, std::string connString)
{
    bool result = true;

    if (connString.size() != send(socket,connString.c_str(),connString.size(),0))
    {
        perror("TODO: do the send() in the correct way :)");
    }

    char buf[4094];
    recv(socket, buf, sizeof(buf), 0);
    std::string bufStr(buf);

    std::string needed = "Connection established";
    size_t pos = bufStr.find(needed);
    if (pos == std::string::npos)
    {
        return false;
    }

    return result;
}


void doProxy()
{
}


int main(int argc, char* argv[])
{
    IP ip;
    parseParameter(ip, argc, argv);

    int socket = connectToSocket(ip.proxyIp, ip.proxyPort);


    std::string connString = createConnectionString(ip.targetIp, ip.targetPort);
    std::cout << connString;

    if (remoteConnect(socket, connString))
    {
        doProxy();
    }
    else
    {
        std::cout << "Remote connection failure!\n";
        return 1;
    }

    return 0;
}

