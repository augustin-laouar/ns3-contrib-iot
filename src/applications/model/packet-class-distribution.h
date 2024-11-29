#ifndef PACKET_CLASS_DISTRIBUTION
#define PACKET_CLASS_DISTRIBUTION

#include <vector>
#include <utility>
#include <random>
#include "packet-class.h"

namespace ns3 
{

/**
 * \ingroup applications
 * Modelize a packet class by a distribution.
 */
class PacketClassDistribution : public PacketClass 
{
public:

    PacketClassDistribution(
        const std::vector<std::pair<uint32_t, double>>& packetSizeDistribution,
        const std::vector<std::pair<double, double>>& interPacketTimeDistribution);

    virtual ~PacketClassDistribution() = default;
    
    //bytes
    uint32_t GetPayloadSize() override;

    //seconds
    double GetInterPacketTime() override;

private:

    // Distributions for packet sizes and inter-packet times
    std::vector<std::pair<uint32_t, double>> m_payloadSizeDistribution;
    std::vector<std::pair<double, double>> m_interPacketTimeDistribution;

    // Random number generator
    std::default_random_engine m_rng;

    // Discrete distributions
    std::discrete_distribution<> m_payloadSizeDistributionObj;
    std::discrete_distribution<> m_interPacketTimeDistributionObj;

};

} // namespace ns3

#endif /* PACKET_CLASS_DISTRIBUTION */
