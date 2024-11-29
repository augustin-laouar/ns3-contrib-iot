#include "iot-camera.h"

#include <ns3/log.h>
#include <ns3/inet-socket-address.h>
#include <ns3/inet6-socket-address.h>
#include <ns3/socket.h>
#include <ns3/tcp-socket-factory.h>
#include <ns3/simulator.h>
#include <ns3/uinteger.h>
#include <random>

NS_LOG_COMPONENT_DEFINE("IotCamera");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(IotCamera);

IotCamera::IotCamera()
    : m_listeningSocket(nullptr), m_state("NOT_STARTED") 
{
    NS_LOG_FUNCTION(this);
}

// static
TypeId IotCamera::GetTypeId() 
{
    static TypeId tid = TypeId("ns3::IotCamera")
                            .SetParent<Application>()
                            .AddConstructor<IotCamera>()
                            .AddAttribute("LocalAddress",
                                          "The local address to bind the socket to.",
                                          AddressValue(),
                                          MakeAddressAccessor(&IotCamera::m_localAddress),
                                          MakeAddressChecker())
                            .AddAttribute("LocalPort",
                                          "Port on which the camera listens.",
                                          UintegerValue(8800),
                                          MakeUintegerAccessor(&IotCamera::m_localPort),
                                          MakeUintegerChecker<uint16_t>());
    return tid;
}

void 
IotCamera::DoDispose() 
{
    NS_LOG_FUNCTION(this);
    StopApplication();
    m_clientSockets.clear();
    Application::DoDispose();
}

void 
IotCamera::StartApplication() 
{
    NS_LOG_FUNCTION(this);

    if (!m_listeningSocket) {
        m_listeningSocket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());

        //define TCP segment size
        m_listeningSocket->SetAttribute("SegmentSize", UintegerValue(1514));
        
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
            MakeCallback(&IotCamera::ConnectionRequestCallback, this),
            MakeCallback(&IotCamera::NewConnectionCreatedCallback, this));
        m_listeningSocket->SetCloseCallbacks(
            MakeCallback(&IotCamera::ConnectionClosedCallback, this),
            MakeNullCallback<void, Ptr<Socket>>());
        m_state = "STARTED";
        NS_LOG_INFO("Camera started, listening on port " << m_localPort);
    }
}

void 
IotCamera::StopApplication() 
{
    NS_LOG_FUNCTION(this);

    m_state = "STOPPED";

    if (m_listeningSocket) {
        m_listeningSocket->Close();
        m_listeningSocket->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address&>(),
                                             MakeNullCallback<void, Ptr<Socket>, const Address&>());
        m_listeningSocket = nullptr;
    }

    for (auto &entry : m_clientSockets) {
        entry.first->Close();
        entry.first->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        entry.first->SetSendCallback(MakeNullCallback<void, Ptr<Socket>, uint32_t>());
    }
    m_clientSockets.clear();

    NS_LOG_INFO("Camera stopped.");
}

std::string 
IotCamera::GetStateString() const 
{
    return m_state;
}

void 
IotCamera::AddPacketClass(std::shared_ptr<PacketClass> packetClass) 
{
    m_packetClasses.push_back(packetClass);
    NS_LOG_INFO("Added a new PacketClass object.");
}

void 
IotCamera::RemovePacketClass(std::shared_ptr<PacketClass> packetClass) 
{
    auto it = std::find(m_packetClasses.begin(), m_packetClasses.end(), packetClass);

    if (it != m_packetClasses.end()) {
        m_packetClasses.erase(it);
        NS_LOG_INFO("Removed a PacketClass object.");
    } else {
        NS_LOG_WARN("PacketClass object not found in the list.");
    }
}

void 
IotCamera::ClearPacketClasses() 
{
    m_packetClasses.clear();
    NS_LOG_INFO("Cleared all PacketClass objects.");
}

bool 
IotCamera::ConnectionRequestCallback(Ptr<Socket> socket, const Address &address) 
{
    NS_LOG_FUNCTION(this << socket << address);
    NS_LOG_INFO("Incoming connection request from " << address);
    return true; // Accept all connections
}

void 
IotCamera::NewConnectionCreatedCallback(Ptr<Socket> socket, const Address &address) 
{
    NS_LOG_FUNCTION(this << socket << address);

    NS_LOG_INFO("New connection established with " << address);
    m_clientSockets[socket] = address;

    socket->SetRecvCallback(MakeCallback(&IotCamera::ReceivedDataCallback, this));
    socket->SetSendCallback(MakeNullCallback<void, Ptr<Socket>, uint32_t>());
}

void 
IotCamera::ConnectionClosedCallback(Ptr<Socket> socket) 
{
    NS_LOG_FUNCTION(this << socket);

    auto it = m_clientSockets.find(socket);
    if (it != m_clientSockets.end()) {
        NS_LOG_INFO("Connection with " << it->second << " closed.");
        m_clientSockets.erase(it);
    }
}


void 
IotCamera::ReceivedDataCallback(Ptr<Socket> socket) 
{
    NS_LOG_FUNCTION(this << socket);

    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom(from))) 
    {
        if (packet->GetSize() == 0) 
        {
            break; // EOF
        }

        uint8_t *buffer = new uint8_t[packet->GetSize()];
        packet->CopyData(buffer, packet->GetSize());
        std::string data(reinterpret_cast<char*>(buffer), packet->GetSize());
        delete[] buffer;

        NS_LOG_INFO("Camera received " << packet->GetSize() << " bytes from " << from);

        if (data == "GET_STREAM") 
        {
            NS_LOG_INFO("Camera start video stream");

            for (const auto& packetClass : m_packetClasses) 
            {
                double interPacketInterval = packetClass->GetInterPacketTime();
                Simulator::Schedule(Seconds(interPacketInterval), &IotCamera::SendData, this, socket, packetClass);
            }
        } else 
        {
            NS_LOG_WARN("Unknown data received: " << data);
        }    
    }
}


void
IotCamera::SendData(Ptr<Socket> socket, std::shared_ptr<PacketClass> packetClass)
{
    NS_LOG_FUNCTION(this << socket << packetClass);

    if (m_state == "STOPPED") 
    {
        NS_LOG_INFO("Camera is stopped. No more video data will be sent.");
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
        NS_LOG_INFO("Camera sent packet of " << bytesSent << " bytes to " << clientAddress);
        m_txTrace(packet);
    }
    else
    {
        NS_LOG_ERROR("Failed to send packet. Socket error: " << socket->GetErrno());
    }


    Simulator::Schedule(Seconds(interPacketInterval), &IotCamera::SendData, this, socket, packetClass);
}


} // namespace ns3
