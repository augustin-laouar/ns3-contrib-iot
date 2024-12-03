#ifndef PACKET_CLASS_BASIC
#define PACKET_CLASS_BASIC

#include <cstdlib>
#include <cmath>
#include "packet-class.h"

namespace ns3 
{

/**
 * \ingroup applications
 * Simple packet class using basic statistical values (min, max, mean, stdDev).
 */
class PacketClassBasic : public PacketClass 
{
public:

    PacketClassBasic(
        uint16_t id,
        double minPacketSize, double maxPacketSize, double meanPacketSize, double stdDevPacketSize,
        double minInterPacketTime, double maxInterPacketTime, double meanInterPacketTime, double stdDevInterPacketTime);

    virtual ~PacketClassBasic() = default;

    //bytes
    uint32_t GetPayloadSize() override;

    //seconds
    double GetInterPacketTime() override;

private:

    // Statistical parameters for packet size
    double m_minPacketSize, m_maxPacketSize, m_meanPacketSize, m_stdDevPacketSize;

    // Statistical parameters for inter-packet times
    double m_minInterPacketTime, m_maxInterPacketTime, m_meanInterPacketTime, m_stdDevInterPacketTime;

    // Random value generator
    double GenerateRandom(double min, double max, double mean, double stdDev);
};

} // namespace ns3

#endif /* PACKET_CLASS_BASIC */
