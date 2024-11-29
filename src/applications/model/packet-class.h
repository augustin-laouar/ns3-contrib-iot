#ifndef PACKET_CLASS
#define PACKET_CLASS
#include <cstdint> 

namespace ns3
{

/**
 * \ingroup applications
 * Modelize a packet class.
 */
class PacketClass 
{
public:

    virtual ~PacketClass() = default;

    //bytes
    virtual uint32_t GetPayloadSize() = 0;

    //seconds
    virtual double GetInterPacketTime() = 0;
    
};

} // namespace ns3

#endif /* PACKET_CLASS */
