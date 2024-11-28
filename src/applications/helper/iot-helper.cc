#include "iot-helper.h"
#include <ns3/uinteger.h>

namespace ns3 {

// IOT CLIENT HELPER /////////////////////////////////////////////////////////


IotClientHelper::IotClientHelper(const Address& address, uint16_t port)
    : ApplicationHelper("ns3::IotClient") {
    m_factory.Set("RemoteCameraAddress", AddressValue(address));
    m_factory.Set("RemoteCameraPort", UintegerValue(port));
}

// IOT CAMERA HELPER /////////////////////////////////////////////////////////

IotCameraHelper::IotCameraHelper(const Address& address, uint16_t port)
    : ApplicationHelper("ns3::IotCamera") {
    m_factory.Set("LocalAddress", AddressValue(address));
    m_factory.Set("LocalPort", UintegerValue(port));
}

} // namespace ns3
