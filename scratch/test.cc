/*
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("IotExample");

int
main(int argc, char* argv[])
{
    // Declare variables used in command-line arguments
    bool useV6 = false;
    bool logging = true;
    Address clientAddress;

    CommandLine cmd(__FILE__);
    cmd.AddValue("useIpv6", "Use Ipv6", useV6);
    cmd.AddValue("logging", "Enable logging", logging);
    cmd.Parse(argc, argv);

    if (logging)
    {
        LogComponentEnable("IotCamera", LOG_LEVEL_INFO);
        LogComponentEnable("IotClient", LOG_LEVEL_INFO);
    }
    NS_LOG_INFO("Create nodes in above topology.");
    NodeContainer n;
    n.Create(2);

    InternetStackHelper internet;
    internet.Install(n);

    //Lien wifi CSMA
    NS_LOG_INFO("Create channel between the two nodes.");
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(5000000)));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
    csma.SetDeviceAttribute("Mtu", UintegerValue(1400));
    NetDeviceContainer d = csma.Install(n);

    //Assign ip addr
    NS_LOG_INFO("Assign IP Addresses.");
    if (!useV6)
    {
        Ipv4AddressHelper ipv4;
        ipv4.SetBase("10.1.1.0", "255.255.255.0");
        Ipv4InterfaceContainer i = ipv4.Assign(d);
        clientAddress = Address(i.GetAddress(1));
    }
    else
    {
        Ipv6AddressHelper ipv6;
        ipv6.SetBase("2001:0000:f00d:cafe::", Ipv6Prefix(64));
        Ipv6InterfaceContainer i6 = ipv6.Assign(d);
        clientAddress = Address(i6.GetAddress(1, 1));
    }


    NS_LOG_INFO("Create IotClient application on node 1.");
    uint16_t port = 4000;
    IotClientHelper client(port);
    ApplicationContainer apps = client.Install(n.Get(1));
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));

    NS_LOG_INFO("Create IotCamera application on node 0 to send to node 1.");
    uint32_t MaxPacketSize = 1024;
    Time interPacketInterval = Seconds(0.05);
    uint32_t maxPacketCount = 10;
    IotCameraHelper camera(clientAddress, port);
    camera.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
    camera.SetAttribute("Interval", TimeValue(interPacketInterval));
    camera.SetAttribute("PacketSize", UintegerValue(MaxPacketSize));
    apps = camera.Install(n.Get(0));
    apps.Start(Seconds(2.0));
    apps.Stop(Seconds(10.0));

    NS_LOG_INFO("Run Simulation.");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");
    return 0;
}
