#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <functional>

#include "steamworks_sdk/public/steam/steam_api.h"
#include "steamworks_sdk/public/steam/isteamgamecoordinator.h"

#include "proto_build/cstrike15_gcmessages.pb.h"
#include "proto_build/gcsdk_gcmessages.pb.h"
#include "proto_build/gcsystemmsgs.pb.h"

constexpr uint32 PROTO_FLAG = (1 << 31);

class MessageListener {
    public:
        std::function<void()> fn;
        google::protobuf::Message* pMsg;

        MessageListener(const std::function<void()>& fn, google::protobuf::Message* pMsg) {
            this->fn = fn;
            this->pMsg = pMsg;            
        }
};

class Client {
    private:
        CCallback<Client, GCMessageAvailable_t, false> CbOnMessageAvailable;
        ISteamGameCoordinator* m_pCoordinator;
        static const uint32 typeSize = 2 * sizeof(uint32);
        std::unordered_map<uint32, MessageListener*> m_Listeners;

        void OnMessageAvailable(GCMessageAvailable_t* pMsg);
    public:
        Client(ISteamGameCoordinator*);
        EGCResults SendMessageToGC(uint32 uMsgType, google::protobuf::Message* msg);
        void AddListener(uint32 uMsgId, MessageListener* pListener);
};