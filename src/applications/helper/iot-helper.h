#ifndef TCP_HELPER_H
#define TCP_HELPER_H

#include <ns3/application-helper.h>

namespace ns3 {

/**
 * \ingroup applications
 * Helper to make it easier to instantiate a IotClient on a set of nodes.
 */
class IotClientHelper : public ApplicationHelper {
public:
    /**
     * Create a IotClientHelper to make it easier to work with IotClient
     * applications.
     * \param address The address of the remote server node to send traffic to.
     * \param port The port of the remote server.
     */
    IotClientHelper(const Address& address, uint16_t port);
};

/**
 * \ingroup applications
 * Helper to make it easier to instantiate a IotCamera on a set of nodes.
 */
class IotCameraHelper : public ApplicationHelper {
public:
    /**
     * Create a IotCameraHelper to make it easier to work with IotCamera
     * applications.
     * \param address The address on which the server will listen.
     * \param port The port on which the server will listen.
     */
    IotCameraHelper(const Address& address, uint16_t port);
};

} // namespace ns3

#endif /* TCP_HELPER_H */
