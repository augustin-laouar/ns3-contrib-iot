#include "iot-client.h"
#include <ns3/log.h>
#include <ns3/inet-socket-address.h>
#include <ns3/inet6-socket-address.h>
#include <ns3/socket.h>
#include <ns3/simulator.h>
#include <ns3/tcp-socket-factory.h>
#include <ns3/uinteger.h>

NS_LOG_COMPONENT_DEFINE("IotClient");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(IotClient);

IotClient::IotClient()
    : m_socket(nullptr), m_remotePort(0), m_sendBufferSize(1024) {
    NS_LOG_FUNCTION(this);
}

TypeId IotClient::GetTypeId() {
    static TypeId tid = TypeId("ns3::IotClient")
                            .SetParent<Application>()
                            .AddConstructor<IotClient>()
                            .AddAttribute("RemoteAddress",
                                          "The address of the remote application.",
                                          AddressValue(),
                                          MakeAddressAccessor(&IotClient::m_remoteAddress),
                                          MakeAddressChecker())
                            .AddAttribute("RemotePort",
                                          "The port of the remote application.",
                                          UintegerValue(8080),
                                          MakeUintegerAccessor(&IotClient::m_remotePort),
                                          MakeUintegerChecker<uint16_t>())
                            .AddAttribute("SendBufferSize",
                                          "The size of the buffer for sending data.",
                                          UintegerValue(1024),
                                          MakeUintegerAccessor(&IotClient::m_sendBufferSize),
                                          MakeUintegerChecker<uint32_t>())
                            .AddTraceSource("Rx",
                                            "Trace for received packets.",
                                            MakeTraceSourceAccessor(&IotClient::m_rxTrace),
                                            "ns3::Packet::PacketAddressTracedCallback")
                            .AddTraceSource("Tx",
                                            "Trace for transmitted packets.",
                                            MakeTraceSourceAccessor(&IotClient::m_txTrace),
                                            "ns3::Packet::TracedCallback");
    return tid;
}

void IotClient::DoDispose() {
    NS_LOG_FUNCTION(this);
    m_socket = nullptr;
    Application::DoDispose();
}

void IotClient::StartApplication() {
    NS_LOG_FUNCTION(this);

    if (!m_socket) {
        m_socket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
        m_socket->SetConnectCallback(
            MakeCallback(&IotClient::ConnectionSucceededCallback, this),
            MakeCallback(&IotClient::ConnectionFailedCallback, this));
        m_socket->SetRecvCallback(MakeCallback(&IotClient::ReceivedDataCallback, this));
        m_socket->SetSendCallback(MakeNullCallback<void, Ptr<Socket>, uint32_t>());        

        if (Ipv4Address::IsMatchingType(m_remoteAddress)) {
            InetSocketAddress remote = InetSocketAddress(Ipv4Address::ConvertFrom(m_remoteAddress), m_remotePort);
            m_socket->Connect(remote);
            NS_LOG_INFO("Client started, Connecting to " << remote.GetIpv4() << " port " << m_remotePort);
        } else if (Ipv6Address::IsMatchingType(m_remoteAddress)) {
            Inet6SocketAddress remote = Inet6SocketAddress(Ipv6Address::ConvertFrom(m_remoteAddress), m_remotePort);
            m_socket->Connect(remote);
            NS_LOG_INFO("Client started, Connecting to " << remote.GetIpv6() << " port " << m_remotePort);
        } else {
            NS_FATAL_ERROR("Unsupported address type.");
        }
    }
}

void IotClient::StopApplication() {
    NS_LOG_FUNCTION(this);

    if (m_socket) {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());        
        m_socket->SetSendCallback(MakeNullCallback<void, Ptr<Socket>, uint32_t>());        
        m_socket = nullptr;
    }

    NS_LOG_INFO("Client stopped.");
}


void IotClient::ConnectionSucceededCallback(Ptr<Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    NS_LOG_INFO("Connection to remote IoT application succeeded.");
}

void IotClient::ConnectionFailedCallback(Ptr<Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    NS_LOG_ERROR("Connection to remote IoT application failed.");
}

void IotClient::ReceivedDataCallback(Ptr<Socket> socket) {
    NS_LOG_FUNCTION(this << socket);

    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom(from))) {
        uint32_t packetSize = packet->GetSize();
        if (packetSize == 0) {
            break; // EOF
        }
        if (InetSocketAddress::IsMatchingType(from))
        {
            InetSocketAddress inetSocketAddress = InetSocketAddress::ConvertFrom(from);
            Ipv4Address ipv4Address = inetSocketAddress.GetIpv4();
            uint16_t port = inetSocketAddress.GetPort();

            NS_LOG_INFO("Received " << packetSize
                      << " bytes from " << ipv4Address
                      << " port " << port);
        }
        else if (Ipv6Address::IsMatchingType(from))
        {
            const Inet6SocketAddress inetSocket6Address = Inet6SocketAddress::ConvertFrom(from);
            Ipv6Address ipv6Address = inetSocket6Address.GetIpv6();
            uint16_t port = inetSocket6Address.GetPort();
            
            NS_LOG_INFO("Received " << packetSize
                      << " bytes from " << ipv6Address
                      << " port " << port);
        }
        m_rxTrace(packet, from);
    }
}
        
} // namespace ns3
