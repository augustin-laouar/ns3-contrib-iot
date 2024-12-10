#include "iot-helper.h"
#include <ns3/uinteger.h>

namespace ns3 {

// IOT CLIENT HELPER /////////////////////////////////////////////////////////


IotClientHelper::IotClientHelper(const Address& address, uint16_t port)
    : ApplicationHelper("ns3::IotClient") 
{
    m_factory.Set("RemoteAddress", AddressValue(address));
    m_factory.Set("RemotePort", UintegerValue(port));
}

// IOT PASSIVE APP HELPER /////////////////////////////////////////////////////////

IotPassiveAppHelper::IotPassiveAppHelper(const Address& address, uint16_t port)
        : ApplicationHelper("ns3::IotPassiveApp")
    {
        m_factory.Set("LocalAddress", AddressValue(address));
        m_factory.Set("LocalPort", UintegerValue(port));
    }


} // namespace ns3
