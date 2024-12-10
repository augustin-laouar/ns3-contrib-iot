#include "packet-class.h"

namespace ns3 
{

PacketClass::PacketClass(
        uint16_t id,
        std::shared_ptr<RandomGenerator> payloadSizeGenerator, 
        std::shared_ptr<RandomGenerator> interPacketTimeGenerator):
        m_id(id),
        m_payloadSizeGenerator(payloadSizeGenerator),
        m_interPacketTimeGenerator(interPacketTimeGenerator)
{
}

uint16_t
PacketClass::GetId() 
{
    return m_id;
}

uint32_t
PacketClass::GetPayloadSize() const
{
    return m_payloadSizeGenerator->GetRandom();
}

double
PacketClass::GetInterPacketTime() const
{
    return m_interPacketTimeGenerator->GetRandom();
}
} // namespace ns3