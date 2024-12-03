#include "packet-class-distribution.h"

namespace ns3 
{

PacketClassDistribution::PacketClassDistribution(
    uint16_t id,
    const std::vector<std::pair<uint32_t, 
    double>>& packetSizeDistribution,
    const std::vector<std::pair<double, 
    double>>& interPacketTimeDistribution)
    : m_payloadSizeDistribution(packetSizeDistribution),
      m_interPacketTimeDistribution(interPacketTimeDistribution)
{
    m_id = id;
    // Initialize packet size distribution
    std::vector<double> sizeProbabilities;
    for (const auto& pair : packetSizeDistribution) {
        sizeProbabilities.push_back(pair.second);
    }
    m_payloadSizeDistributionObj = std::discrete_distribution<>(sizeProbabilities.begin(), sizeProbabilities.end());

    // Initialize inter-packet time distribution
    std::vector<double> timeProbabilities;
    for (const auto& pair : interPacketTimeDistribution) {
        timeProbabilities.push_back(pair.second);
    }
    m_interPacketTimeDistributionObj = std::discrete_distribution<>(timeProbabilities.begin(), timeProbabilities.end());
}

uint32_t PacketClassDistribution::GetPayloadSize() {
    // Use the random generator and size distribution to select a packet size
    int index = m_payloadSizeDistributionObj(m_rng);
    return m_payloadSizeDistribution[index].first;
}

double PacketClassDistribution::GetInterPacketTime() {
    // Use the random generator and time distribution to select an inter-packet time
    int index = m_interPacketTimeDistributionObj(m_rng);
    return m_interPacketTimeDistribution[index].first;
}


} // namespace ns3