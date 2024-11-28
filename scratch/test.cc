#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("IotExample");

void CameraRx(Ptr<const Packet> packet, const Address& address) {
    NS_LOG_INFO("Camera received a packet of " << packet->GetSize() << " bytes from " << InetSocketAddress::ConvertFrom(address).GetIpv4());
}

void CameraTx(Ptr<const Packet> packet) {
    NS_LOG_INFO("Camera sent a packet of " << packet->GetSize() << " bytes.");
}

void ClientRx(Ptr<const Packet> packet, const Address& address) {
    NS_LOG_INFO("Client received a packet of " << packet->GetSize() << " bytes from " << InetSocketAddress::ConvertFrom(address).GetIpv4());
}

int main(int argc, char* argv[]) {
    double simTimeSec = 10.0;
    CommandLine cmd(__FILE__);
    cmd.AddValue("SimulationTime", "Length of simulation in seconds.", simTimeSec);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnableAll(LOG_PREFIX_TIME);
    LogComponentEnable("IotExample", LOG_ALL);
    LogComponentEnable("IotCamera", LOG_ALL);
    LogComponentEnable("IotClient", LOG_ALL);

    NodeContainer wifiApNode;
    wifiApNode.Create(1); // AP node
    NodeContainer wifiStaNodes;
    wifiStaNodes.Create(2); // Client nodes
    NodeContainer wifiCameraNode;
    wifiCameraNode.Create(1); // Camera node

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

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

    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNode);      // AP
    mobility.Install(wifiStaNodes);   // Clients
    mobility.Install(wifiCameraNode); // Camera

    InternetStackHelper stack;
    stack.Install(wifiApNode);
    stack.Install(wifiStaNodes);
    stack.Install(wifiCameraNode);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterface = address.Assign(apDevice);
    Ipv4InterfaceContainer staInterfaces = address.Assign(staDevices);
    Ipv4InterfaceContainer cameraInterface = address.Assign(cameraDevice);

    Ipv4Address cameraAddress = cameraInterface.GetAddress(0);

    uint16_t cameraPort = 8800;
    IotCameraHelper cameraHelper(Address(cameraAddress), cameraPort);
    ApplicationContainer cameraApps = cameraHelper.Install(wifiCameraNode.Get(0));
    Ptr<IotCamera> camera = cameraApps.Get(0)->GetObject<IotCamera>();

    camera->TraceConnectWithoutContext("Rx", MakeCallback(&CameraRx));
    camera->TraceConnectWithoutContext("Tx", MakeCallback(&CameraTx));


    for (uint32_t i = 0; i < wifiStaNodes.GetN(); ++i) {
        IotClientHelper clientHelper(Address(cameraAddress), cameraPort);
        ApplicationContainer clientApps = clientHelper.Install(wifiStaNodes.Get(i));
        Ptr<IotClient> client = clientApps.Get(0)->GetObject<IotClient>();

        client->TraceConnectWithoutContext("Rx", MakeCallback(&ClientRx));

        clientApps.Stop(Seconds(simTimeSec));
    }

    cameraApps.Stop(Seconds(simTimeSec));

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
