#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("IotPassiveAppExample");


void TraceIotTxPacket(Ptr<const Packet> packet, const Address& clientAddress, uint16_t subFlowId)
{
    if (InetSocketAddress::IsMatchingType(clientAddress))
    {
        InetSocketAddress inetSocketAddress = InetSocketAddress::ConvertFrom(clientAddress);
        Ipv4Address ipv4Address = inetSocketAddress.GetIpv4();
        uint16_t port = inetSocketAddress.GetPort();
        NS_LOG_INFO("Sent " << packet->GetSize() << " bytes to " << ipv4Address << ":" << port);
    }
    else if (Ipv6Address::IsMatchingType(clientAddress))
    {
        const Inet6SocketAddress inetSocket6Address = Inet6SocketAddress::ConvertFrom(clientAddress);
        Ipv6Address ipv6Address = inetSocket6Address.GetIpv6();
        uint16_t port = inetSocket6Address.GetPort();
        NS_LOG_INFO("Sent " << packet->GetSize() << " bytes to " << ipv6Address << ":" << port);
    }    
}


void TraceIotRxPacket(Ptr<const Packet> packet, const Address& from)
{
    if (InetSocketAddress::IsMatchingType(from))
    {
        InetSocketAddress inetSocketAddress = InetSocketAddress::ConvertFrom(from);
        Ipv4Address ipv4Address = inetSocketAddress.GetIpv4();
        uint16_t port = inetSocketAddress.GetPort();
        NS_LOG_INFO("Received " << packet->GetSize() << " bytes from " << ipv4Address << ":" << port);
    }
    else if (Ipv6Address::IsMatchingType(from))
    {
        const Inet6SocketAddress inetSocket6Address = Inet6SocketAddress::ConvertFrom(from);
        Ipv6Address ipv6Address = inetSocket6Address.GetIpv6();
        uint16_t port = inetSocket6Address.GetPort();
        NS_LOG_INFO("Received " << packet->GetSize() << " bytes from " << ipv6Address << ":" << port);
    }    
}

void
LoadTapoC200TraficProfile(Ptr<IotPassiveApp> iotApp)
{
    std::vector<std::shared_ptr<SubFlow>> trafficProfile;

    std::shared_ptr<RandomGenerator> payloadSizeGen1 = std::make_shared<RandomGeneratorNormal>(691, 1448, 744.381, 191.231);
    std::shared_ptr<RandomGenerator> interPacketTimeGen1 = std::make_shared<RandomGeneratorNormal>(0.000008, 2.02, 0.059936, 0.077852);
    std::shared_ptr<SubFlow> subFlow1 = std::make_shared<SubFlow>(1, payloadSizeGen1, interPacketTimeGen1);
    trafficProfile.push_back(subFlow1);

    std::shared_ptr<RandomGenerator> payloadSizeGen2 = std::make_shared<RandomGeneratorNormal>(883, 1448, 977.167, 230.66);
    std::shared_ptr<RandomGenerator> interPacketTimeGen2 = std::make_shared<RandomGeneratorUniform>(5.046386, 21.891857);
    std::shared_ptr<SubFlow> subFlow2 = std::make_shared<SubFlow>(2, payloadSizeGen2, interPacketTimeGen2);
    trafficProfile.push_back(subFlow2);

    std::shared_ptr<RandomGenerator> payloadSizeGen3 = std::make_shared<RandomGeneratorNormal>(2004, 202720, 7761.412, 11299.521);
    std::shared_ptr<RandomGenerator> interPacketTimeGen3 = std::make_shared<RandomGeneratorNormal>(0.000006, 0.252874, 0.065435, 0.021381);
    std::shared_ptr<SubFlow> subFlow3 = std::make_shared<SubFlow>(3, payloadSizeGen3, interPacketTimeGen3);
    trafficProfile.push_back(subFlow3);

    std::shared_ptr<RandomGenerator> payloadSizeGen4 = std::make_shared<RandomGeneratorNormal>(5, 1420, 730.692, 451.447);
    std::shared_ptr<RandomGenerator> interPacketTimeGen4 = std::make_shared<RandomGeneratorNormal>(0.087334, 5.042865, 0.941867, 0.927757);
    std::shared_ptr<SubFlow> subFlow4 = std::make_shared<SubFlow>(4, payloadSizeGen4, interPacketTimeGen4);
    trafficProfile.push_back(subFlow4);

    iotApp->SetTrafficProfile(trafficProfile);

}
int 
main(int argc, char* argv[]) 
{
    double simTimeSec = 90;
    CommandLine cmd(__FILE__);
    cmd.AddValue("SimulationTime", "Length of simulation in seconds.", simTimeSec);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnableAll(LOG_PREFIX_TIME);
    LogComponentEnable("IotPassiveAppExample", LOG_INFO);
    LogComponentEnable("IotPassiveAppExample", LOG_WARN);
    //LogComponentEnable("IotPassiveApp", LOG_INFO);
    //LogComponentEnable("IotClient", LOG_INFO);

    NodeContainer wifiApNode;
    wifiApNode.Create(1); // AP node
    NodeContainer wifiStaNodes;
    wifiStaNodes.Create(2); // Client nodes
    NodeContainer wifiCameraNode;
    wifiCameraNode.Create(1); // Camera node

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());
    phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211n);
    wifi.SetRemoteStationManager("ns3::IdealWifiManager");

    Ssid ssid = Ssid("wifi-network");
    WifiMacHelper mac;

    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevice = wifi.Install(phy, mac, wifiApNode);

    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false));
    NetDeviceContainer staDevices = wifi.Install(phy, mac, wifiStaNodes);

    NetDeviceContainer cameraDevice = wifi.Install(phy, mac, wifiCameraNode);

    //enable packet capture
    phy.EnablePcap("move-basic", cameraDevice.Get(0), false);

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNode);      // AP
    mobility.Install(wifiStaNodes);   // Clients
    mobility.Install(wifiCameraNode); // Camera

    InternetStackHelper internet;
    internet.Install(wifiApNode);
    internet.Install(wifiStaNodes);
    internet.Install(wifiCameraNode);

    Ipv4AddressHelper ipv4Adress;
    ipv4Adress.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterface = ipv4Adress.Assign(apDevice);
    Ipv4InterfaceContainer staInterfaces = ipv4Adress.Assign(staDevices);
    Ipv4InterfaceContainer cameraInterface = ipv4Adress.Assign(cameraDevice);

    Ipv4Address cameraAddress = cameraInterface.GetAddress(0);
    uint16_t cameraPort = 8800;
    IotPassiveAppHelper cameraHelper(Address(cameraAddress), cameraPort);
    ApplicationContainer cameraApps = cameraHelper.Install(wifiCameraNode.Get(0));
    Ptr<IotPassiveApp> iotApp = cameraApps.Get(0)->GetObject<IotPassiveApp>();

    LoadTapoC200TraficProfile(iotApp);
    iotApp->SetStartTime(Seconds(0.0));
    iotApp->TraceConnectWithoutContext("Tx", MakeCallback(&TraceIotTxPacket));

    double delay = 0;
    for (uint32_t i = 0; i < wifiStaNodes.GetN(); ++i) {
        IotClientHelper clientHelper(Address(cameraAddress), cameraPort);
        ApplicationContainer clientApps = clientHelper.Install(wifiStaNodes.Get(i));
        Ptr<IotClient> client = clientApps.Get(0)->GetObject<IotClient>();

        client->SetStartTime(Seconds(1 + delay));

        if (delay + 6 < simTimeSec)
        {
            delay += 5;
        }

        int decrease = delay <= 20 ? 20 - delay : 20;
        clientApps.Stop(Seconds(simTimeSec - decrease));
    }

    cameraApps.Stop(Seconds(simTimeSec));

    Simulator::Stop(Seconds(simTimeSec));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
