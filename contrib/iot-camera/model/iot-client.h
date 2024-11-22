#ifndef IOT_CLIENT_H
#define IOT_CLIENT_H

#include "ns3/application.h"
#include "ns3/traced-callback.h"

namespace ns3 
{
class Socket;
class Packet;

class IotClient : public Application
{

    public:
        static TypeId GetTypeId();

        IotClient();
        ~IotClient() override;
        uint64_t GetReceived() const;

    private:
        void StartApplication() override;
        void StopApplication() override;
        void HandleRead(Ptr<Socket> socket);

        uint16_t m_port;
        Ptr<Socket> m_socket;
        Ptr<Socket> m_socket6;
        uint64_t m_received;

        TracedCallback<Ptr<const Packet>> m_rxTrace;
        TracedCallback<Ptr<const Packet>, const Address&, const Address&> m_rxTraceWithAddresses;

};
} //namespace ns3
#endif /* IOT_CLIENT_H */
