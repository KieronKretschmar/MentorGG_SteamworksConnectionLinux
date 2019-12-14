#include "Client.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <regex>

bool bReady = false;

void ReportError(const std::string& sMessage)
{
    std::cout << sMessage << std::endl;
    std::cin.get();
}

void MessageLoop(ISteamGameCoordinator* coordinator)
{
    std::cout << "MessageLoop started" << std::endl;

    while(true)
    {
        try 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            Steam_RunCallbacks(GetHSteamPipe(), false);
            bReady = true;
        } 
        catch(std::exception& e)
        {
            ReportError(e.what());
            return;
        }
    }
}

struct DecodedSharecode {
    uint64_t matchId;
    uint64_t outcomeId;
    uint64_t token;
};

bool DecodeSharecode(std::string code, DecodedSharecode* pOut)
{    
    std::cout << "Decoding: " << code << std::endl;

    std::string sharecode = code;
    sharecode = std::regex_replace(sharecode, std::regex("CSGO|-"), "");
    sharecode = std::string(sharecode.rbegin(), sharecode.rend());
    
    std::string dictionary("ABCDEFGHJKLMNOPQRSTUVWXYZabcdefhijkmnopqrstuvwxyz23456789");
    
    std::array<uint8_t, 18> result = {};
    
    
    for (char cur_char : sharecode)
    {
        std::array<uint8_t, 18> tmp = {};
    
        int addval = static_cast<int>(dictionary.find(cur_char));
        int carry = 0;
        int v = 0;
    
        for (int t = 17; t >= 0; t--) {
            carry = 0;
            for (int s = t; s >= 0; s--) {
                if (t - s == 0) {
                    v = tmp[s] + result[t] * 57;
                }
                else {
                    v = 0;
                }
                v = v + carry;
                carry = v >> 8;
                tmp[s] = v & 0xFF;
            }
        }
    
        result = tmp;
        carry = 0;
    
        for (int t = 17; t >= 0; t--) {
            if (t == 17) {
                v = result[t] + addval;
            }
            else {
                v = result[t];
            }
            v = v + carry;
            carry = v >> 8;
            result[t] = v & 0xFF;
        }
    }
    
    pOut->matchId = *reinterpret_cast<uint64_t*>(result.data());
    pOut->outcomeId = *reinterpret_cast<uint64_t*>(result.data() + 8);
    pOut->token = *reinterpret_cast<uint16_t*>(result.data() + 16);

    return true;
}

int main(int argc, char* argv[])
{
    std::cout << "steamworksconnectionlinux" << std::endl;

    if (SteamAPI_RestartAppIfNecessary(k_uAppIdInvalid))
    {
        ReportError("Steam requires a restart");
        return 0;
    }

    if(!SteamAPI_Init())
    {
        ReportError("SteamAPI_Init failed");
        return 0;
    }

    if(!SteamUser()->BLoggedOn())
    {
        ReportError("No user is currently logged into Steam");
        return 0;
    }

    std::cout << "Current user: " << SteamFriends()->GetPersonaName() << std::endl;

    auto coordinator = (ISteamGameCoordinator*)SteamClient()->GetISteamGenericInterface(
        GetHSteamUser(), 
        GetHSteamPipe(), 
        STEAMGAMECOORDINATOR_INTERFACE_VERSION
    );

    if (coordinator == nullptr)
    {
        ReportError("Failed to grab SteamGameCoordinator");
        return 0;
    }

    Client client(coordinator);

    CMsgClientWelcome welcomeRequest;
    MessageListener welcomeListener([&welcomeRequest, &client]() {
        std::cout << "Server welcomed us " << std::endl;

        /*
        std::cout << "Requesting match history" << std::endl;

        CMsgGCCStrike15_v2_MatchListRequestRecentUserGames matchHistoryRequest;
        matchHistoryRequest.set_accountid(SteamUser()->GetSteamID().GetAccountID());

        if(client.SendMessageToGC(k_EMsgGCCStrike15_v2_MatchListRequestRecentUserGames, &matchHistoryRequest) != k_EGCResultOK)
        {
            ReportError("Failed to request match history");
            return 0;
        }
        */

    }, &welcomeRequest);

    client.AddListener(k_EMsgGCClientWelcome, &welcomeListener);

    std::thread thread(MessageLoop, coordinator);

    while(!bReady)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    CMsgClientHello hello;
    hello.set_client_session_need(1);

    if(client.SendMessageToGC(k_EMsgGCClientHello, &hello) != k_EGCResultOK)
    {
        ReportError("Failed to request session");
        return 0;
    }

    std::cout << "writing" << std::endl;

    char szFifoI[] = "/tmp/swcpipei";
    mkfifo(szFifoI, 0666);

    char szFifoO[] = "/tmp/swcpipeo";
    mkfifo(szFifoO, 0666);

    auto fileI = open(szFifoI, O_RDONLY);
    auto fileO = open(szFifoO, O_WRONLY);

    char szMsg[256];

    CMsgGCCStrike15_v2_MatchList matchList;
    MessageListener matchlistListener([&matchList, &fileO]() {

        std::cout << "Received match history with " << matchList.matches_size() << " matches" << std::endl;

        for (int i = 0; i < matchList.matches_size(); i++)
        {
            auto match = matchList.matches(i);
            auto link = match.roundstatsall(match.roundstatsall_size() - 1).map();

            std::cout << "Link #" << i << ": " << link << std::endl;
            std::string sPipeMsg = "--demo " + link + "|" + std::to_string(match.matchtime());

            write(fileO, sPipeMsg.c_str(), sPipeMsg.length() + 1);
        }

        //if sharecode is too old, the match history will be empty
        if (!matchList.matches_size()) {
            std::string sError = "--demo SHARECODE_TOO_OLD";
            write(fileO, sError.c_str(), sError.length() + 1);
        }

    }, &matchList);

    client.AddListener(k_EMsgGCCStrike15_v2_MatchList, &matchlistListener);    

    while (true) {
        if(read(fileI, szMsg, 255)) {
            DecodedSharecode ds;
            DecodeSharecode(std::string(szMsg), &ds);

            CMsgGCCStrike15_v2_MatchListRequestFullGameInfo fgi;
            fgi.set_matchid(ds.matchId);
            fgi.set_outcomeid(ds.outcomeId);
            fgi.set_token(ds.token);

            if(client.SendMessageToGC(k_EMsgGCCStrike15_v2_MatchListRequestFullGameInfo, &fgi) != k_EGCResultOK) {
                std::cout << "failed to send message to GC" << std::endl;
                std::string sError = "--demo UNKNOWN_ERROR";
                write(fileO, sError.c_str(), sError.length() + 1);
            }
        }
    }

    close(fileI);
    close(fileO);

    std::cin.get();

    return 0;
}