#include "Client.h"

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
        std::cout << "Requesting match history" << std::endl;

        CMsgGCCStrike15_v2_MatchListRequestRecentUserGames matchHistoryRequest;
        matchHistoryRequest.set_accountid(SteamUser()->GetSteamID().GetAccountID());

        if(client.SendMessageToGC(k_EMsgGCCStrike15_v2_MatchListRequestRecentUserGames, &matchHistoryRequest) != k_EGCResultOK)
        {
            ReportError("Failed to request match history");
            return 0;
        }
    }, &welcomeRequest);

    CMsgGCCStrike15_v2_MatchList matchList;
    MessageListener matchlistListener([&matchList]() {

        std::cout << "Received match history with " << matchList.matches_size() << " matches" << std::endl;

        for (int i = 0; i < matchList.matches_size(); i++)
        {
            auto match = matchList.matches(i);
            auto link = match.roundstatsall(match.roundstatsall_size() - 1).map();

            std::cout << "Link #" << i << ": " << link << std::endl;
        }

    }, &matchList);

    client.AddListener(k_EMsgGCClientWelcome, &welcomeListener);
    client.AddListener(k_EMsgGCCStrike15_v2_MatchList, &matchlistListener);

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

    std::cin.get();

    return 0;
}