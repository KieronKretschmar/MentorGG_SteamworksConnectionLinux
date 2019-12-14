#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
    std::cout << "fifotest started" << std::endl;

    char szFifoO[] = "/tmp/swcpipei";
    mkfifo(szFifoO, 0666);

    char szFifoI[] = "/tmp/swcpipeo";
    mkfifo(szFifoI, 0666);

    char szMsg[256];

    auto fileO = open(szFifoO, O_WRONLY);
    auto fileI = open(szFifoI, O_RDONLY);
    std::string sc1 = "CSGO-U6MWi-hYFWJ-opPwD-JciHm-qOijD";
    std::string sc2 = "CSGO-H9CGB-PRAWb-m7m9S-2PUGP-9v4ZJ";

    write(fileO, sc1.c_str(), sc1.length() + 1);
    read(fileI, szMsg, 255);
    printf("%s\n", szMsg);

    write(fileO, sc2.c_str(), sc2.length() + 1);
    read(fileI, szMsg, 255);
    printf("%s\n", szMsg);

    close(fileO);
    close(fileI);

    std::cin.get();

}