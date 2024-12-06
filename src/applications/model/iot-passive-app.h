#ifndef IOT_PASSIVE_APP
#define IOT_PASSIVE_APP

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <ns3/address.h>
#include <ns3/application.h>
#include <ns3/event-id.h>
#include <ns3/nstime.h>
#include <ns3/ptr.h>
#include <ns3/traced-callback.h>
#include "packet-class.h"
namespace ns3
{

class Socket;
class Packet;

enum class AppState
{
    STOPPED,
    NOT_STARTED,
    STARTED
};

/**
 * \brief Iot Camera Application capable of handling multiple clients.
 *
 * This application passively listens for incoming TCP connections and can handle
 * multiple clients simultaneously.
 */
class IotPassiveApp : public Application
{
public:
    /**
     * Creates a new instance of camera application.
     */
    IotPassiveApp();

    /**
     * Default constructor.
     */
    virtual ~IotPassiveApp() = default;

    /**
     * Returns the object TypeId.
     * \return The object TypeId.
     */
    static TypeId GetTypeId();

    /**
     * Returns the current state of the application in string format.
     * \return The current state of the application in string format.
     */
    std::string GetStateString() const;

    /**
     * Returns the current state of the application.
     * \return The current state of the application.
     */
    AppState GetState() const;

    /**
     * Set the traffic profile using a list of PacketClass objects.
     * This function replaces any existing packet classes with the provided list.
     * 
     * \param packetClasses A vector of shared pointers to PacketClass objects.
     */
    void SetTrafficProfile(const std::vector<std::shared_ptr<PacketClass>>& trafficProfile);

protected:
    void DoDispose() override;

private:
    void StartApplication() override;
    void StopApplication() override;
    // SOCKET CALLBACK METHODS

    /**
     * Invoked when the listening socket receives a connection request.
     * \param socket Pointer to the socket where the event originates from.
     * \param address The address of the remote client where the connection
     *                request comes from.
     * \return Always true, to indicate that the connection request is accepted.
     */
    bool ConnectionRequestCallback(Ptr<Socket> socket, const Address& address);

    /**
     * Invoked when a new connection has been established.
     * \param socket Pointer to the socket that maintains the connection to the
     *               remote client.
     * \param address The address the connection is incoming from.
     */
    void NewConnectionCreatedCallback(Ptr<Socket> socket, const Address& address);

    /**
     * Invoked when a connection with a client is terminated.
     * \param socket Pointer to the socket where the event originates from.
     */
    void ConnectionClosedCallback(Ptr<Socket> socket);

    /**
     * Send video data.
     * \param socket Pointer to the socket to send data.
     * \param packetClass Packet class associated.
     */
    void SendData(Ptr<Socket> socket, std::shared_ptr<PacketClass> packetClass);

    /// List of PacketClass objects (abstract or derived)
    std::vector<std::shared_ptr<PacketClass>> m_trafficProfile;

    std::map<Ptr<Socket>, std::vector<EventId>> m_trafficProfileEvents;
    /// The listening socket for receiving connection requests from clients.
    Ptr<Socket> m_listeningSocket;
    /// Collection of accepted sockets.
    std::map<Ptr<Socket>, Address> m_clientSockets;
    /// The state of the application.
    AppState m_state;

    // ATTRIBUTES
    Address m_localAddress; ///< The local address to bind the socket to.
    uint16_t m_localPort;   ///< The local port to bind the socket to.

    // TRACE SOURCES
    TracedCallback<Ptr<Socket>, const Address&> m_newConnectionTrace; ///< Trace for new connections.
    TracedCallback<Ptr<const Packet>, const Address&> m_rxTrace;      ///< Trace for received packets.
    TracedCallback<Ptr<const Packet>, uint16_t> m_txTrace;            ///< Trace for transmitted packets.
};

} // namespace ns3

#endif /* IOT_PASSIVE_APP */
