#ifndef IOT_HELPER
#define IOT_HELPER

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
 * Helper to make it easier to instantiate a IotPassiveApp on a set of nodes.
 */
class IotPassiveAppHelper : public ApplicationHelper {
public:
    /**
     * Create a IotPassiveAppHelper to make it easier to work with IotPassiveApp
     * applications.
     * \param address The address on which the server will listen.
     * \param port The port on which the server will listen.
     */
    IotPassiveAppHelper(const Address& address, uint16_t port);
};

} // namespace ns3

#endif /* IOT_HELPER */
