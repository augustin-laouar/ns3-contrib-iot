#include "iot-helper.h"

#include <ns3/string.h>
#include <ns3/uinteger.h>

namespace ns3
{

IotClientHelper::IotClientHelper()
    : ApplicationHelper(IotClient::GetTypeId())
{
}

IotClientHelper::IotClientHelper(uint16_t port)
    : IotClientHelper()
{
    SetAttribute("Port", UintegerValue(port));
}

IotCameraHelper::IotCameraHelper()
    : ApplicationHelper(IotCamera::GetTypeId())
{
}

IotCameraHelper::IotCameraHelper(const Address& address)
    : IotCameraHelper()
{
    SetAttribute("RemoteAddress", AddressValue(address));
}

IotCameraHelper::IotCameraHelper(const Address& address, uint16_t port)
    : IotCameraHelper(address)
{
    SetAttribute("RemotePort", UintegerValue(port));
}

} // namespace ns3
