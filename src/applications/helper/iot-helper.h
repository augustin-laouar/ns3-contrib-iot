#ifndef IOT_HELPER_H
#define IOT_HELPER_H

#include <ns3/application-helper.h>
#include <ns3/iot-client.h>
#include <ns3/iot-camera.h>

#include <stdint.h>

namespace ns3
{

class IotClientHelper : public ApplicationHelper
{
  public:

    IotClientHelper();

    IotClientHelper(uint16_t port);
};

class IotCameraHelper : public ApplicationHelper
{
  public:
    IotCameraHelper();

    IotCameraHelper(const Address& ip, uint16_t port);

    IotCameraHelper(const Address& addr);
};

} // namespace ns3

#endif /* IOT_HELPER_H */
