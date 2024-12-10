#include "sub-flow.h"

namespace ns3 
{

SubFlow::SubFlow(
        uint16_t id,
        std::shared_ptr<RandomGenerator> payloadSizeGenerator, 
        std::shared_ptr<RandomGenerator> interPacketTimeGenerator):
        m_id(id),
        m_payloadSizeGenerator(payloadSizeGenerator),
        m_interPacketTimeGenerator(interPacketTimeGenerator)
{
}

uint16_t
SubFlow::GetId() 
{
    return m_id;
}

uint32_t
SubFlow::GetPayloadSize() const
{
    return m_payloadSizeGenerator->GetRandom();
}

double
SubFlow::GetInterPacketTime() const
{
    return m_interPacketTimeGenerator->GetRandom();
}
} // namespace ns3