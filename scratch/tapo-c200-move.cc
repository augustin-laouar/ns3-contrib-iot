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


void TraceIotTxPacket(Ptr<const Packet> packet, const Address& clientAddress, uint16_t subFlowId)
{
    static std::ofstream csvFile("camera_tx_packets.csv", std::ios::out | std::ios::app);
    static bool isHeaderWritten = false;

    if (!csvFile.is_open())
    {
        NS_LOG_ERROR("TraceTxCameraPacket : Failed to open CSV file.");
        return;
    }

    if (!isHeaderWritten)
    {
        csvFile << "Timestamp,ClientAddress,SubFlowId,PacketSize\n";
        isHeaderWritten = true;
    }


    double timestamp = Simulator::Now().GetSeconds();
    uint32_t packetSize = packet->GetSize();
    if (InetSocketAddress::IsMatchingType(clientAddress))
    {
        InetSocketAddress inetSocketAddress = InetSocketAddress::ConvertFrom(clientAddress);
        uint16_t port = inetSocketAddress.GetPort();
        Ipv4Address ipv4Address = inetSocketAddress.GetIpv4();
        csvFile << timestamp << "," << ipv4Address << ":" << port << "," << subFlowId << "," << packetSize  << "\n";
    }
    else
    {
        csvFile << timestamp << "," << clientAddress << "," << subFlowId << "," << packetSize  << "\n";
    }
    

}


void TraceIotRxPacket(Ptr<const Packet> packet, const Address& from)
{
    static std::ofstream csvFile("camera_rx_packets.csv", std::ios::out | std::ios::app);
    static bool isHeaderWritten = false;

    if (!csvFile.is_open())
    {
        NS_LOG_ERROR("TraceRxCameraPacket : Failed to open CSV file.");
        return;
    }

    if (!isHeaderWritten)
    {
        csvFile << "Timestamp,PacketSize,FromAddress\n";
        isHeaderWritten = true;
    }

    double timestamp = Simulator::Now().GetSeconds();
    uint32_t packetSize = packet->GetSize();

    csvFile << timestamp << "," << packetSize << "," << from << "\n";
}


void 
LoadSubFlowFromFile(Ptr<IotPassiveApp> iotApp, const std::string& fichierJson) 
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

    if (!j.contains("sub-flows") || !j["sub-flows"].is_array()) {
        NS_LOG_ERROR("Error: JSON file must contain a 'sub-flows' array.");
        return;
    }

    std::vector<std::shared_ptr<SubFlow>> trafficProfile;
    for (const auto& entry : j["sub-flows"]) {
        if (!entry.contains("payload-size")) {
            NS_LOG_WARN("Warning: Each packet class must have a 'payload-size' field.");
            continue;
        }
        if (!entry.contains("inter-packet-times")) {
            NS_LOG_WARN("Warning: Each packet class must have a 'inter-packet-times' field.");
            continue;
        }
        if (!entry.contains("id") || !entry["id"].is_number_integer()) {
            NS_LOG_WARN("Warning: Each packet class must have an 'id' field.");
            continue;
        }
        uint16_t id = entry["id"];
        auto payloadSize = entry["payload-size"];
        auto interPacketTimes = entry["inter-packet-times"];
        std::shared_ptr<RandomGenerator> payloadSizeGenerator;
        std::shared_ptr<RandomGenerator> interPacketTimesGenerator;
        std::shared_ptr<SubFlow> subFlow;

        if (!payloadSize.contains("type") || !payloadSize["type"].is_string()) 
        {
            NS_LOG_WARN("Warning: payloadSize object must have a 'type' field.");
            continue;
        }
        if (!interPacketTimes.contains("type") || !interPacketTimes["type"].is_string()) 
        {
            NS_LOG_WARN("Warning: interPacketTimes object must have a 'type' field.");
            continue;
        }
        //payloadsize
        if (payloadSize["type"] == "rv") 
        {
            if (!(payloadSize.contains("min") && payloadSize.contains("max")
                && payloadSize.contains("mean") && payloadSize.contains("std-dev"))) 
            {
                NS_LOG_WARN("Warning: Invalid format for payloadSize. Skipping entry.");
                continue;
            }
            double min = payloadSize["min"].get<double>();
            double max = payloadSize["max"].get<double>();
            double mean = payloadSize["mean"].get<double>();
            double stdDev = payloadSize["std-dev"].get<double>();
            payloadSizeGenerator = std::make_shared<RandomGeneratorRv>(min, max, mean, stdDev);
        }
        else if (payloadSize["type"] == "dist") 
        {
            std::vector<std::pair<double, double>> distribution;
            for (const auto& value : payloadSize["distribution"]) 
            {
                if (!(value.contains("value") && value.contains("prabability")))
                {
                    NS_LOG_WARN("Warning: Invalid format for 'distribution'. Skipping entry.");
                    continue;
                } 
                distribution.emplace_back(value["value"].get<double>(), value["prabability"].get<double>());
            }
            payloadSizeGenerator = std::make_shared<RandomGeneratorDist>(distribution);

        }
        else if (payloadSize["type"] == "normal") 
        {
            if (!(payloadSize.contains("min") && payloadSize.contains("max")
                && payloadSize.contains("mean") && payloadSize.contains("std-dev"))) 
            {
                NS_LOG_WARN("Warning: Invalid format for payloadSize. Skipping entry.");
                continue;
            }
            double min = payloadSize["min"].get<double>();
            double max = payloadSize["max"].get<double>();
            double mean = payloadSize["mean"].get<double>();
            double stdDev = payloadSize["std-dev"].get<double>();
            payloadSizeGenerator = std::make_shared<RandomGeneratorNormal>(min, max, mean, stdDev);
        }
        else 
        {
            NS_LOG_WARN("Warning: Unknown type '" << payloadSize["type"] << "'. Skipping this entry.");
        }
        
        //interPacketTimes
        if (interPacketTimes["type"] == "rv") 
        {
            if (!(interPacketTimes.contains("min") && interPacketTimes.contains("max")
                && interPacketTimes.contains("mean") && interPacketTimes.contains("std-dev"))) 
            {
                NS_LOG_WARN("Warning: Invalid format for interPacketTimes. Skipping entry.");
                continue;
            }
            double min = interPacketTimes["min"].get<double>();
            double max = interPacketTimes["max"].get<double>();
            double mean = interPacketTimes["mean"].get<double>();
            double stdDev = interPacketTimes["std-dev"].get<double>();
            interPacketTimesGenerator = std::make_shared<RandomGeneratorRv>(min, max, mean, stdDev);
        }
        else if (interPacketTimes["type"] == "dist") 
        {
            std::vector<std::pair<double, double>> distribution;
            for (const auto& value : interPacketTimes["distribution"]) {
                if (!(value.contains("value") && value.contains("prabability"))) 
                {
                    NS_LOG_WARN("Warning: Invalid format for 'distribution'. Skipping entry.");
                    continue;
                } 
                distribution.emplace_back(value["value"].get<double>(), value["prabability"].get<double>());
            }
            interPacketTimesGenerator = std::make_shared<RandomGeneratorDist>(distribution);

        }
        else if (interPacketTimes["type"] == "normal") 
        {
            if (!(interPacketTimes.contains("min") && interPacketTimes.contains("max")
                && interPacketTimes.contains("mean") && interPacketTimes.contains("std-dev"))) 
            {
                NS_LOG_WARN("Warning: Invalid format for interPacketTimes. Skipping entry.");
                continue;
            }
            double min = interPacketTimes["min"].get<double>();
            double max = interPacketTimes["max"].get<double>();
            double mean = interPacketTimes["mean"].get<double>();
            double stdDev = interPacketTimes["std-dev"].get<double>();
            interPacketTimesGenerator = std::make_shared<RandomGeneratorNormal>(min, max, mean, stdDev);
        }
        else 
        {
            NS_LOG_WARN("Warning: Unknown type '" << interPacketTimes["type"] << "'. Skipping this entry.");
        }
        subFlow = std::make_shared<SubFlow>(id, payloadSizeGenerator, interPacketTimesGenerator);
        trafficProfile.push_back(subFlow);
    }
    iotApp->SetTrafficProfile(trafficProfile);
}

int 
main(int argc, char* argv[]) 
{
    double simTimeSec = 1.0;
    CommandLine cmd(__FILE__);
    cmd.AddValue("SimulationTime", "Length of simulation in seconds.", simTimeSec);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnableAll(LOG_PREFIX_TIME);
    LogComponentEnable("IotBasicExample", LOG_INFO);
    LogComponentEnable("IotBasicExample", LOG_WARN);
    LogComponentEnable("IotPassiveApp", LOG_INFO);
    LogComponentEnable("IotClient", LOG_INFO);
    //LogComponentEnable("ApWifiMac", LOG_LEVEL_ALL);
    //LogComponentEnable("StaWifiMac", LOG_LEVEL_ALL);
    //LogComponentEnable("WifiMac", LOG_LEVEL_ALL);

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

    LoadSubFlowFromFile(iotApp, "/home/augustin/projects/ens/ns/ns-allinone-3.43/ns-3.43/scratch/tapo-c200-move-rv.json");
    iotApp->SetStartTime(Seconds(0.0));
    iotApp->TraceConnectWithoutContext("Tx", MakeCallback(&TraceIotTxPacket));

    double delay = 0.2;
    for (uint32_t i = 0; i < wifiStaNodes.GetN(); ++i) {
        IotClientHelper clientHelper(Address(cameraAddress), cameraPort);
        ApplicationContainer clientApps = clientHelper.Install(wifiStaNodes.Get(i));
        Ptr<IotClient> client = clientApps.Get(0)->GetObject<IotClient>();

        client->SetStartTime(Seconds(0.1 + delay));

        if (delay + 0.2 < simTimeSec)
        {
            delay += 0.1;
        }
        
        if (i == 0)
        {
            clientApps.Stop(Seconds(simTimeSec - 0.5));
        }
        else 
        {
            clientApps.Stop(Seconds(simTimeSec));

        }
    }

    cameraApps.Stop(Seconds(simTimeSec));

    Simulator::Stop(Seconds(simTimeSec));
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
