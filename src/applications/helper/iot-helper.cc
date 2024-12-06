#include "iot-helper.h"
#include <ns3/uinteger.h>
#include <ns3/packet-class-basic.h>
#include <ns3/packet-class-distribution.h>
#include <ns3/packet-class.h>
#include <ns3/pointer.h>
#include <ns3/attribute.h>

namespace ns3 {

// IOT CLIENT HELPER /////////////////////////////////////////////////////////


IotClientHelper::IotClientHelper(const Address& address, uint16_t port)
    : ApplicationHelper("ns3::IotClient") 
{
    m_factory.Set("RemoteCameraAddress", AddressValue(address));
    m_factory.Set("RemoteCameraPort", UintegerValue(port));
}

// IOT PASSIVE APP HELPER /////////////////////////////////////////////////////////

IotPassiveAppHelper::IotPassiveAppHelper(const Address& address, uint16_t port)
        : ApplicationHelper("ns3::IotPassiveApp")
    {
        m_factory.Set("LocalAddress", AddressValue(address));
        m_factory.Set("LocalPort", UintegerValue(port));
    }


} // namespace ns3
