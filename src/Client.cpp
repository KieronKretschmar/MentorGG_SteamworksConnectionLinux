#include "Client.h"

Client::Client(ISteamGameCoordinator* pCoordinator)
    :CbOnMessageAvailable(this, &Client::OnMessageAvailable), m_pCoordinator(pCoordinator)
{

}

void Client::OnMessageAvailable(GCMessageAvailable_t* pMsg)
{
    std::vector<char> recvBuffer;
    recvBuffer.resize(pMsg->m_nMessageSize);

    uint32 msgType;
    uint32 msgSize;

    m_pCoordinator->RetrieveMessage(&msgType, recvBuffer.data(), recvBuffer.size(), &msgSize);

    if (msgType & PROTO_FLAG)
    {
        auto msg_id = msgType & (~PROTO_FLAG);        

        if (m_Listeners.find(msg_id) != m_Listeners.end()) {
            m_Listeners[msg_id]->pMsg->ParseFromArray(recvBuffer.data() + typeSize, msgSize - typeSize);
            m_Listeners[msg_id]->fn();
        }
        else
        {
            std::cout << "Received unhandled StoC message with id " << msg_id << std::endl;
        }        
    }
}

EGCResults Client::SendMessageToGC(uint32 uMsgType, google::protobuf::Message* msg)
{
    std::vector<char> sendBuffer;
    sendBuffer.resize(msg->ByteSize() + typeSize);

    uMsgType |= PROTO_FLAG;

    auto data = (uint32*)sendBuffer.data();

    data[0] = uMsgType;
    data[1] = 0;

    msg->SerializeToArray(sendBuffer.data() + typeSize, sendBuffer.size() - typeSize);

    return m_pCoordinator->SendMessage(uMsgType, sendBuffer.data(), msg->ByteSize() + typeSize);
}

void Client::AddListener(uint32 uMsgId, MessageListener* pListener)
{
    m_Listeners[uMsgId] = pListener;
}