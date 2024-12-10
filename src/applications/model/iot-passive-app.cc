#include "iot-passive-app.h"

#include <ns3/log.h>
#include <ns3/inet-socket-address.h>
#include <ns3/inet6-socket-address.h>
#include <ns3/socket.h>
#include <ns3/tcp-socket-factory.h>
#include <ns3/simulator.h>
#include <ns3/uinteger.h>
#include <random>
#include <ns3/pointer.h>

NS_LOG_COMPONENT_DEFINE("IotPassiveApp");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(IotPassiveApp);

IotPassiveApp::IotPassiveApp()
    : m_listeningSocket(nullptr), m_state(AppState::NOT_STARTED) 
{
    NS_LOG_FUNCTION(this);
}

TypeId IotPassiveApp::GetTypeId() 
{
    static TypeId tid = TypeId("ns3::IotPassiveApp")
                            .SetParent<Application>()
                            .AddConstructor<IotPassiveApp>()
                            .AddAttribute("LocalAddress",
                                          "The local address to bind the socket to.",
                                          AddressValue(),
                                          MakeAddressAccessor(&IotPassiveApp::m_localAddress),
                                          MakeAddressChecker())
                            .AddAttribute("LocalPort",
                                          "Port on which the application listens.",
                                          UintegerValue(8800),
                                          MakeUintegerAccessor(&IotPassiveApp::m_localPort),
                                          MakeUintegerChecker<uint16_t>())
                            .AddTraceSource("Tx",
                                            "A packet has been transmitted.",
                                            MakeTraceSourceAccessor(&IotPassiveApp::m_txTrace),
                                            "ns3::Packet::TracedCallback")
                            .AddTraceSource("Rx",
                                            "A packet has been received.",
                                            MakeTraceSourceAccessor(&IotPassiveApp::m_rxTrace),
                                            "ns3::Packet::PacketAddressTracedCallback");
                            
    return tid;
}


void 
IotPassiveApp::DoDispose() 
{
    NS_LOG_FUNCTION(this);
    StopApplication();
    m_clientSockets.clear();
    m_trafficProfile.clear();
    Application::DoDispose();
}

void 
IotPassiveApp::StartApplication() 
{
    NS_LOG_FUNCTION(this);

    if (!m_listeningSocket) {
        m_listeningSocket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());

        //define TCP segment size
        m_listeningSocket->SetAttribute("SegmentSize", UintegerValue(1448));
        
        if (Ipv4Address::IsMatchingType(m_localAddress)) {
            InetSocketAddress local = InetSocketAddress(Ipv4Address::ConvertFrom(m_localAddress), m_localPort);
            if (m_listeningSocket->Bind(local) == -1) {
                NS_FATAL_ERROR("Failed to bind IPv4 socket.");
            }
        } else if (Ipv6Address::IsMatchingType(m_localAddress)) {
            Inet6SocketAddress local = Inet6SocketAddress(Ipv6Address::ConvertFrom(m_localAddress), m_localPort);
            if (m_listeningSocket->Bind(local) == -1) {
                NS_FATAL_ERROR("Failed to bind IPv6 socket.");
            }
        } else {
            NS_FATAL_ERROR("Unsupported address type.");
        }

        m_listeningSocket->Listen();
        m_listeningSocket->SetAcceptCallback(
            MakeCallback(&IotPassiveApp::ConnectionRequestCallback, this),
            MakeCallback(&IotPassiveApp::NewConnectionCreatedCallback, this));
        m_listeningSocket->SetCloseCallbacks(
            MakeCallback(&IotPassiveApp::ConnectionClosedCallback, this),
            MakeNullCallback<void, Ptr<Socket>>());

        m_state = AppState::STARTED;
        NS_LOG_INFO("IoT application started, listening on port " << m_localPort);
    }
}

void 
IotPassiveApp::StopApplication() 
{
    NS_LOG_FUNCTION(this);

    m_state = AppState::STOPPED;

    if (m_listeningSocket) {
        m_listeningSocket->Close();
        m_listeningSocket->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address&>(),
                                             MakeNullCallback<void, Ptr<Socket>, const Address&>());
        m_listeningSocket = nullptr;
    }

    for (auto& entry : m_clientSockets) {
        entry.first->Close();
        entry.first->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
    m_clientSockets.clear();

    for (auto& entry : m_trafficProfileEvents) 
    {
        for (auto& event : entry.second) 
        {
            Simulator::Cancel(event);
        }
    }
    m_trafficProfileEvents.clear();


    NS_LOG_INFO("IoT application stopped.");
}


AppState 
IotPassiveApp::GetState() const 
{
    return m_state;
}

std::string 
IotPassiveApp::GetStateString() const
{
    switch (m_state)
    {
    case AppState::STOPPED:
        return "STOPPED";
    case AppState::NOT_STARTED:
        return "NOT_STARTED";
    case AppState::STARTED:
        return "STARTED";
    default:
        return "UNKNOWN";
    }
}

void 
IotPassiveApp::SetTrafficProfile(const std::vector<std::shared_ptr<PacketClass>>& trafficProfile)
{
    NS_LOG_FUNCTION(this);

    for (auto& entry : m_trafficProfileEvents) 
    {
        for (auto& event : entry.second) 
        {
            Simulator::Cancel(event);
        }
    }
    m_trafficProfileEvents.clear();

    
    m_trafficProfile = trafficProfile;

    NS_LOG_INFO("Traffic profile configured with " << trafficProfile.size() << " PacketClass objects.");
}


bool 
IotPassiveApp::ConnectionRequestCallback(Ptr<Socket> socket, const Address &address) 
{
    NS_LOG_FUNCTION(this << socket << address);
    if (InetSocketAddress::IsMatchingType(address))
    {
        InetSocketAddress inetSocketAddress = InetSocketAddress::ConvertFrom(address);
        Ipv4Address ipv4Address = inetSocketAddress.GetIpv4();
        uint16_t port = inetSocketAddress.GetPort();
        NS_LOG_INFO("Incoming connection request from " << ipv4Address
                    << " port " << port);
    }
    else if (Ipv6Address::IsMatchingType(address))
    {
        const Inet6SocketAddress inetSocket6Address = Inet6SocketAddress::ConvertFrom(address);
        Ipv6Address ipv6Address = inetSocket6Address.GetIpv6();
        uint16_t port = inetSocket6Address.GetPort();
        NS_LOG_INFO("Incoming connection request from " << ipv6Address
                    << " port " << port);
    }
    return true; // Accept all connections
}

void 
IotPassiveApp::NewConnectionCreatedCallback(Ptr<Socket> socket, const Address &address) 
{
    NS_LOG_FUNCTION(this << socket << address);
    if (InetSocketAddress::IsMatchingType(address))
    {
        InetSocketAddress inetSocketAddress = InetSocketAddress::ConvertFrom(address);
        Ipv4Address ipv4Address = inetSocketAddress.GetIpv4();
        uint16_t port = inetSocketAddress.GetPort();
        NS_LOG_INFO("New connection established with " << ipv4Address
                    << " port " << port);
    }
    else if (Ipv6Address::IsMatchingType(address))
    {
        const Inet6SocketAddress inetSocket6Address = Inet6SocketAddress::ConvertFrom(address);
        Ipv6Address ipv6Address = inetSocket6Address.GetIpv6();
        uint16_t port = inetSocket6Address.GetPort();
        NS_LOG_INFO("New connection established with " << ipv6Address
                    << " port " << port);
    }    

    m_clientSockets[socket] = address;

    socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    socket->SetSendCallback(MakeNullCallback<void, Ptr<Socket>, uint32_t>());
    
    for (auto& packetClass : m_trafficProfile) 
    {
        double interPacketInterval = packetClass->GetInterPacketTime();
        EventId event = Simulator::Schedule(Seconds(interPacketInterval), &IotPassiveApp::SendData, this, socket, packetClass);
        m_trafficProfileEvents[socket].push_back(event);

    }
}

void 
IotPassiveApp::ConnectionClosedCallback(Ptr<Socket> socket) 
{
    NS_LOG_FUNCTION(this << socket);

    auto socketIt = m_clientSockets.find(socket);
    if (socketIt != m_clientSockets.end()) {
        if (InetSocketAddress::IsMatchingType(socketIt->second))
        {
            InetSocketAddress inetSocketAddress = InetSocketAddress::ConvertFrom(socketIt->second);
            Ipv4Address ipv4Address = inetSocketAddress.GetIpv4();
            uint16_t port = inetSocketAddress.GetPort();
            NS_LOG_INFO("Connection with " << ipv4Address
                        << " port " << port << " closed");
        }
        else if (Ipv6Address::IsMatchingType(socketIt->second))
        {
            const Inet6SocketAddress inetSocket6Address = Inet6SocketAddress::ConvertFrom(socketIt->second);
            Ipv6Address ipv6Address = inetSocket6Address.GetIpv6();
            uint16_t port = inetSocket6Address.GetPort();
            NS_LOG_INFO("New connection established with " << ipv6Address
                        << " port " << port << " closed");
        }    
        m_clientSockets.erase(socketIt);

        auto eventIt = m_trafficProfileEvents.find(socket);
        if (eventIt != m_trafficProfileEvents.end()) 
        {
            for (auto& event : eventIt->second) 
            {
                Simulator::Cancel(event);
            }
            m_trafficProfileEvents.erase(eventIt); 
        }

    }
}


void 
IotPassiveApp::SendData(Ptr<Socket> socket, std::shared_ptr<PacketClass> packetClass)
{
    NS_LOG_FUNCTION(this << socket << packetClass);

    if (m_state != AppState::STARTED) 
    {
        NS_LOG_WARN("SendPacketForClass invoked, but the application is not in the STARTED state.");
        return;
    }

    if (!packetClass)
    {
        NS_LOG_ERROR("SendPacketForClass received a null PacketClass pointer.");
        return;
    }

    uint32_t packetSize = packetClass->GetPayloadSize();
    double interPacketInterval = packetClass->GetInterPacketTime();

    Ptr<Packet> packet = Create<Packet>(packetSize);
    int bytesSent = socket->Send(packet);

    if (bytesSent > 0)
    {
        Address clientAddress;
        socket->GetPeerName(clientAddress);
        if (InetSocketAddress::IsMatchingType(clientAddress))
        {
            InetSocketAddress inetSocketAddress = InetSocketAddress::ConvertFrom(clientAddress);
            Ipv4Address ipv4Address = inetSocketAddress.GetIpv4();
            uint16_t port = inetSocketAddress.GetPort();

            NS_LOG_INFO("Sent " << packetSize
                      << " bytes to " << ipv4Address
                      << " port " << port);
        }
        else if (Ipv6Address::IsMatchingType(clientAddress))
        {
            const Inet6SocketAddress inetSocket6Address = Inet6SocketAddress::ConvertFrom(clientAddress);
            Ipv6Address ipv6Address = inetSocket6Address.GetIpv6();
            uint16_t port = inetSocket6Address.GetPort();
            
            NS_LOG_INFO("Sent " << packetSize
                      << " bytes to " << ipv6Address
                      << " port " << port);
        }
        m_txTrace(packet, clientAddress, packetClass->GetId());
    }
    else
    {
        NS_LOG_ERROR("Failed to send packet. Socket error: " << socket->GetErrno());
    }

    EventId nextEvent = Simulator::Schedule(Seconds(interPacketInterval), &IotPassiveApp::SendData, this, socket, packetClass);
    m_trafficProfileEvents[socket].push_back(nextEvent);
}



} // namespace ns3
