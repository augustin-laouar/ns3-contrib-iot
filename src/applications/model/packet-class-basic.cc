#include "packet-class-basic.h"

namespace ns3 
{

PacketClassBasic::PacketClassBasic(
    double minPacketSize, double maxPacketSize, double meanPacketSize, double stdDevPacketSize,
    double minInterPacketTime, double maxInterPacketTime, double meanInterPacketTime, double stdDevInterPacketTime)
    : m_minPacketSize(minPacketSize), m_maxPacketSize(maxPacketSize), 
      m_meanPacketSize(meanPacketSize), m_stdDevPacketSize(stdDevPacketSize),
      m_minInterPacketTime(minInterPacketTime), m_maxInterPacketTime(maxInterPacketTime),
      m_meanInterPacketTime(meanInterPacketTime), m_stdDevInterPacketTime(stdDevInterPacketTime)
{
}

uint32_t PacketClassBasic::GetPayloadSize() 
{
    return static_cast<uint32_t>(GenerateRandom(m_minPacketSize, m_maxPacketSize, m_meanPacketSize, m_stdDevPacketSize));
}

double PacketClassBasic::GetInterPacketTime() 
{
    return GenerateRandom(m_minInterPacketTime, m_maxInterPacketTime, m_meanInterPacketTime, m_stdDevInterPacketTime);
}

double PacketClassBasic::GenerateRandom(double min, double max, double mean, double stdDev) 
{
    // Generate a value around the mean within the range [min, max]
    double random = mean + ((rand() / (double)RAND_MAX) - 0.5) * 2 * stdDev;
    if (random < min) return min;
    if (random > max) return max;
    return random;
}

} // namespace ns3
