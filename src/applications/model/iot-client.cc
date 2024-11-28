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
    : m_socket(nullptr), m_remoteCameraPort(0), m_sendBufferSize(1024) {
    NS_LOG_FUNCTION(this);
}

TypeId IotClient::GetTypeId() {
    static TypeId tid = TypeId("ns3::IotClient")
                            .SetParent<Application>()
                            .AddConstructor<IotClient>()
                            .AddAttribute("RemoteCameraAddress",
                                          "The address of the remote camera.",
                                          AddressValue(),
                                          MakeAddressAccessor(&IotClient::m_remoteCameraAddress),
                                          MakeAddressChecker())
                            .AddAttribute("RemoteCameraPort",
                                          "The port of the remote camera.",
                                          UintegerValue(8080),
                                          MakeUintegerAccessor(&IotClient::m_remoteCameraPort),
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
        m_socket->SetSendCallback(MakeCallback(&IotClient::SendCallback, this));

        if (Ipv4Address::IsMatchingType(m_remoteCameraAddress)) {
            InetSocketAddress remote = InetSocketAddress(Ipv4Address::ConvertFrom(m_remoteCameraAddress), m_remoteCameraPort);
            m_socket->Connect(remote);
        } else if (Ipv6Address::IsMatchingType(m_remoteCameraAddress)) {
            Inet6SocketAddress remote = Inet6SocketAddress(Ipv6Address::ConvertFrom(m_remoteCameraAddress), m_remoteCameraPort);
            m_socket->Connect(remote);
        } else {
            NS_FATAL_ERROR("Unsupported address type.");
        }

        NS_LOG_INFO("Client started and connecting to " << m_remoteCameraAddress);
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

Ptr<Socket> IotClient::GetSocket() const {
    return m_socket;
}

void IotClient::ConnectionSucceededCallback(Ptr<Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    NS_LOG_INFO("Connection to camera succeeded.");

    // Send a request to watch stream
    std::string message = "GET_STREAM";
    Ptr<Packet> packet = Create<Packet>((uint8_t*)message.c_str(), message.length());
    socket->Send(packet);
    NS_LOG_INFO("Sent: " << message);
    m_txTrace(packet);
}

void IotClient::ConnectionFailedCallback(Ptr<Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    NS_LOG_ERROR("Connection to camera failed.");
}

void IotClient::ReceivedDataCallback(Ptr<Socket> socket) {
    NS_LOG_FUNCTION(this << socket);

    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom(from))) {
        if (packet->GetSize() == 0) {
            break; // EOF
        }

        NS_LOG_INFO("Received " << packet->GetSize() << " bytes from " << from);
        m_rxTrace(packet, from);
    }
}

void IotClient::SendCallback(Ptr<Socket> socket, uint32_t availableBufferSize) {
    NS_LOG_FUNCTION(this << socket << availableBufferSize);
    NS_LOG_INFO("Buffer space available for sending: " << availableBufferSize << " bytes.");
}

} // namespace ns3
