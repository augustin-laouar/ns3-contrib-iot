#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <ns3/address.h>
#include <ns3/application.h>
#include <ns3/traced-callback.h>

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup applications
 * Simple TCP client application for sending and receiving data.
 *
 * This application establishes a connection to a remote server,
 * sends data, and handles incoming data.
 */
class IotClient : public Application {
public:
    IotClient();

    static TypeId GetTypeId();

    /**
     * Get the socket used by this client.
     * \return The client socket.
     */
    Ptr<Socket> GetSocket() const;

protected:
    void DoDispose() override;

private:
    void StartApplication() override;
    void StopApplication() override;

    // SOCKET CALLBACK METHODS

    /**
     * Called when the connection is successfully established.
     * \param socket The connected socket.
     */
    void ConnectionSucceededCallback(Ptr<Socket> socket);

    /**
     * Called when the connection attempt fails.
     * \param socket The socket.
     */
    void ConnectionFailedCallback(Ptr<Socket> socket);

    /**
     * Called when data is received on the socket.
     * \param socket The socket receiving the data.
     */
    void ReceivedDataCallback(Ptr<Socket> socket);

    /**
     * Called when the socket has available buffer space for sending.
     * \param socket The socket.
     * \param availableBufferSize The available buffer size.
     */
    void SendCallback(Ptr<Socket> socket, uint32_t availableBufferSize);

    /// The socket for sending and receiving data.
    Ptr<Socket> m_socket;

    /// Remote camera address.
    Address m_remoteCameraAddress;

    /// Remote camera port.
    uint16_t m_remoteCameraPort;

    /// Buffer size for sending data.
    uint32_t m_sendBufferSize;
    
    /// Trace for received packets.
    TracedCallback<Ptr<const Packet>, const Address&> m_rxTrace;

    /// Trace for sent packets.
    TracedCallback<Ptr<const Packet>> m_txTrace;
};

} // namespace ns3

#endif /* TCP_CLIENT_H */
