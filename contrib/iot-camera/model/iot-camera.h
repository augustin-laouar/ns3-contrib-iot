#ifndef IOT_CAMERA_H
#define IOT_CAMERA_H

// Add a doxygen group for this module.
// If you have more than one file, this should be in only one of them.
/**
 * \defgroup iot-camera Description of the iot-camera
 */

#include "ns3/application.h"
#include <ns3/traced-callback.h>

namespace ns3
{

class Socket;
class Packet;

// Each class should be documented using Doxygen,
// and have an \ingroup iot-camera directive

/* ... */
class IotCamera : public Application
{
    public: 
        static TypeId GetTypeId();

        IotCamera();
        ~IotCamera() override;

        void SetRemote(Address ip, uint16_t port);
        void SetRemote(Address addr);
        uint64_t GetTotalTx() const;
    private:
        void StartApplication() override;
        void StopApplication() override;
        void Send();

        TracedCallback<Ptr<const Packet>> m_txTrace;
        TracedCallback<Ptr<const Packet>, const Address&, const Address&> m_txTraceWithAddresses;
        uint32_t m_count; //!< Maximum number of packets the application will send
        Time m_interval;  //!< Packet inter-send time
        uint32_t m_size;  //!< Size of the sent packet (including the SeqTsHeader)

        uint32_t m_sent;       //!< Counter for sent packets
        uint64_t m_totalTx;    //!< Total bytes sent
        Ptr<Socket> m_socket;  //!< Socket
        Address m_peerAddress; //!< Remote peer address
        uint16_t m_peerPort;   //!< Remote peer port
        EventId m_sendEvent;   //!< Event to send the next packet


};

} //namespace ns3

#endif /* IOT_CAMERA_H */
