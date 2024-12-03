#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("IotBasicExample");

void 
CameraRx(Ptr<const Packet> packet, const Address& address) 
{
    NS_LOG_INFO("Camera received a packet of " << packet->GetSize() << " bytes from " << InetSocketAddress::ConvertFrom(address).GetIpv4());
}

void 
CameraTx(Ptr<const Packet> packet) 
{
    NS_LOG_INFO("Camera sent a packet of " << packet->GetSize() << " bytes.");
}

void 
ClientRx(Ptr<const Packet> packet, const Address& address) 
{
    NS_LOG_INFO("Client received a packet of " << packet->GetSize() << " bytes from " << InetSocketAddress::ConvertFrom(address).GetIpv4());
}

void 
LoadPacketClassFromFile(Ptr<IotCamera> camera, const std::string& fichierJson) 
{
    std::ifstream file(fichierJson);
    if (!file.is_open()) {
        NS_LOG_ERROR("Error: Unable to open the file " << fichierJson);
        return;
    }

    json j;
    try {
        file >> j;
    } catch (const json::parse_error& e) {
        NS_LOG_ERROR("Error parsing the JSON file: " << e.what());
        return;
    }

    if (!j.contains("packet-classes") || !j["packet-classes"].is_array()) {
        NS_LOG_ERROR("Error: JSON file must contain a 'packet-classes' array.");
        return;
    }

    for (const auto& packetClass : j["packet-classes"]) {
        if (!packetClass.contains("type") || !packetClass["type"].is_string()) {
            NS_LOG_WARN("Warning: Each packet class must have a 'type' field.");
            continue;
        }
        if (!packetClass.contains("id") || !packetClass["id"].is_number_integer()) {
            NS_LOG_WARN("Warning: Each packet class must have an 'id' field.");
            continue;
        }
        std::string type = packetClass["type"];
        uint16_t id = packetClass["id"];
        if (type == "distribution") {
            if (!packetClass.contains("payload-sizes") || !packetClass.contains("inter-packet-times")) {
                NS_LOG_WARN("Warning: 'distribution' type must contain 'payload-sizes' and 'inter-packet-times'.");
                continue;
            }

            std::vector<std::pair<uint32_t, double>> payloadSizes;
            for (const auto& size : packetClass["payload-sizes"]) {
                if (size.contains("size") && size.contains("prabability")) {
                    payloadSizes.emplace_back(size["size"].get<uint32_t>(), size["prabability"].get<double>());
                } else {
                    NS_LOG_WARN("Warning: Invalid format for 'payload-sizes'. Skipping entry.");
                }
            }

            std::vector<std::pair<double, double>> interPacketTimes;
            for (const auto& time : packetClass["inter-packet-times"]) {
                if (time.contains("time") && time.contains("prabability")) {
                    interPacketTimes.emplace_back(time["time"].get<double>(), time["prabability"].get<double>());
                } else {
                    NS_LOG_WARN("Warning: Invalid format for 'inter-packet-times'. Skipping entry.");
                }
            }

            std::shared_ptr<PacketClassDistribution> distributionClass = std::make_shared<PacketClassDistribution>(id, payloadSizes, interPacketTimes);
            camera->AddPacketClass(distributionClass);
            NS_LOG_INFO("Distribution packet class added successfully.");

        } else if (type == "basic") {
            if (!packetClass.contains("payload-size") || !packetClass.contains("inter-packet-times")) {
                NS_LOG_WARN("Warning: 'basic' type must contain 'payload-size' and 'inter-packet-times'.");
                continue;
            }

            auto payloadSize = packetClass["payload-size"];
            double minSize = payloadSize["min"];
            double maxSize = payloadSize["max"];
            double meanSize = payloadSize["mean"];
            double stdDevSize = payloadSize["std-dev"];

            auto interPacketTimes = packetClass["inter-packet-times"];
            double minTime = interPacketTimes["min"];
            double maxTime = interPacketTimes["max"];
            double meanTime = interPacketTimes["mean"];
            double stdDevTime = interPacketTimes["std-dev"];

            std::shared_ptr<PacketClassBasic> basicClass = std::make_shared<PacketClassBasic>(
                id, minSize, maxSize, meanSize, stdDevSize, minTime, maxTime, meanTime, stdDevTime);
            camera->AddPacketClass(basicClass);
            NS_LOG_INFO("Basic packet class added successfully.");

        } else {
            NS_LOG_WARN("Warning: Unknown type '" << type << "'. Skipping this entry.");
        }
    }
}

int 
main(int argc, char* argv[]) 
{
    double simTimeSec = 50.0;
    CommandLine cmd(__FILE__);
    cmd.AddValue("SimulationTime", "Length of simulation in seconds.", simTimeSec);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnableAll(LOG_PREFIX_TIME);
    //LogComponentEnable("IotBasicExample", LOG_INFO);
    LogComponentEnable("IotCamera", LOG_INFO);
    //LogComponentEnable("IotClient", LOG_INFO);
    //LogComponentEnable("ApWifiMac", LOG_LEVEL_ALL);
    //LogComponentEnable("StaWifiMac", LOG_LEVEL_ALL);
    //LogComponentEnable("WifiMac", LOG_LEVEL_ALL);

    NodeContainer wifiApNode;
    wifiApNode.Create(1); // AP node
    NodeContainer wifiStaNodes;
    wifiStaNodes.Create(1); // Client nodes
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

    //enable packet capture
    //phy.EnablePcap("move-basic", cameraDevice.Get(0), false);

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

    LoadPacketClassFromFile(camera, "/home/augustin/projects/ens/ns/ns-allinone-3.43/ns-3.43/scratch/tapo-c200-move-basic.json");
    camera->TraceConnectWithoutContext("Rx", MakeCallback(&CameraRx));
    camera->TraceConnectWithoutContext("Tx", MakeCallback(&CameraTx));
    camera->SetStartTime(Seconds(0.1));
    
    double delay = 0;
    for (uint32_t i = 0; i < wifiStaNodes.GetN(); ++i) {
        IotClientHelper clientHelper(Address(cameraAddress), cameraPort);
        ApplicationContainer clientApps = clientHelper.Install(wifiStaNodes.Get(i));
        Ptr<IotClient> client = clientApps.Get(0)->GetObject<IotClient>();

        client->TraceConnectWithoutContext("Rx", MakeCallback(&ClientRx));
        client->SetStartTime(Seconds(1.0 + delay));

        if (delay + 1.5 < simTimeSec)
        {
            delay += 0.5;
        }

        clientApps.Stop(Seconds(simTimeSec));
    }

    cameraApps.Stop(Seconds(simTimeSec));

    Simulator::Stop(Seconds(simTimeSec));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
