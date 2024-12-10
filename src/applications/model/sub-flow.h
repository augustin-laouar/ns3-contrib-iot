#ifndef PACKET_CLASS
#define PACKET_CLASS
#include <cstdint> 
#include <memory>
#include "random-generator.h"
namespace ns3
{

/**
 * \ingroup applications
 * Modelize a packet class.
 */
class SubFlow 
{
public:

    SubFlow(
        uint16_t id,
        std::shared_ptr<RandomGenerator> payloadSizeGenerator, 
        std::shared_ptr<RandomGenerator> interPacketTimeGenerator);

    virtual ~SubFlow() = default;

    //bytes
    uint32_t GetPayloadSize() const;

    //seconds
    double GetInterPacketTime() const;
    
    uint16_t GetId();
protected: 
    uint16_t m_id;
    std::shared_ptr<RandomGenerator> m_payloadSizeGenerator;
    std::shared_ptr<RandomGenerator> m_interPacketTimeGenerator;
};

} // namespace ns3

#endif /* PACKET_CLASS */
