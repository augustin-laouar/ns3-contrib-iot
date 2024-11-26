#include "iot-client.h"
#include "ns3/uinteger.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "seq-ts-header.h"

namespace ns3

{

NS_LOG_COMPONENT_DEFINE("IotClient");

NS_OBJECT_ENSURE_REGISTERED(IotClient);

TypeId
IotClient::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::IotClient")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<IotClient>()
            .AddAttribute("Port",
                          "Port on which we listen for incoming packets.",
                          UintegerValue(100),
                          MakeUintegerAccessor(&IotClient::m_port),
                          MakeUintegerChecker<uint16_t>())
            .AddTraceSource("Rx",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&IotClient::m_rxTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("RxWithAddresses",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&IotClient::m_rxTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback");
    return tid;
}

IotClient::IotClient()
    : m_received(0)
{
    NS_LOG_FUNCTION(this);
}

IotClient::~IotClient()
{
    NS_LOG_FUNCTION(this);
}

uint64_t
IotClient::GetReceived() const
{
    NS_LOG_FUNCTION(this);
    return m_received;
}

void
IotClient::StartApplication()
{
    NS_LOG_FUNCTION(this);

    if (!m_socket)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
        if (m_socket->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
    }
    m_socket->SetRecvCallback(MakeCallback(&IotClient::HandleRead, this));

    if(!m_socket6)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket6 = Socket::CreateSocket(GetNode(), tid);
        Inet6SocketAddress local = Inet6SocketAddress(Ipv6Address::GetAny(), m_port);
        if (m_socket6->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
    }
    
    m_socket6->SetRecvCallback(MakeCallback(&IotClient::HandleRead, this));

}

void
IotClient::StopApplication()
{
    NS_LOG_FUNCTION(this);

    if (m_socket)
    {
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

void
IotClient::HandleRead(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from)))
    {
        socket->GetSockName(localAddress);
        m_rxTrace(packet);
        m_rxTraceWithAddresses(packet, from, localAddress);
        if (packet->GetSize() > 0)
        {
            uint32_t receivedSize = packet->GetSize();
            SeqTsHeader seqTs;
            uint32_t currentSequenceNumber = seqTs.GetSeq();
            if (InetSocketAddress::IsMatchingType(from))
            {
                NS_LOG_INFO("TraceDelay: RX " << receivedSize << " bytes from "
                                << InetSocketAddress::ConvertFrom(from).GetIpv4()
                                << " Sequence Number: " << currentSequenceNumber
                                << " Uid: " << packet->GetUid() << " TXtime: "
                                << seqTs.GetTs() << " RXtime: " << Simulator::Now()
                                << " Delay: " << Simulator::Now() - seqTs.GetTs());
            }
            else if (Inet6SocketAddress::IsMatchingType(from))
            {
                NS_LOG_INFO("TraceDelay: RX " << receivedSize << " bytes from "
                                              << Inet6SocketAddress::ConvertFrom(from).GetIpv6()
                                              << " Sequence Number: " << currentSequenceNumber
                                              << " Uid: " << packet->GetUid() << " TXtime: "
                                              << seqTs.GetTs() << " RXtime: " << Simulator::Now()
                                              << " Delay: " << Simulator::Now() - seqTs.GetTs());
               
            }
            m_received++;
        }
    }
}

} //namespace ns3