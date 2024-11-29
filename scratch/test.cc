#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("IotExample");

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
    // Lire le fichier JSON
    std::ifstream file(fichierJson);
    if (!file.is_open()) {
        std::cerr << "Erreur: impossible d'ouvrir le fichier " << fichierJson << std::endl;
        return;
    }

    // Parse le fichier en objet JSON
    json j;
    try {
        file >> j;
    } catch (const json::parse_error& e) {
        std::cerr << "Erreur de parsing du fichier JSON: " << e.what() << std::endl;
        return;
    }

    // Vérifie si le fichier contient des classes de paquets
    if (!j.contains("packet-classes") || !j["packet-classes"].is_array()) {
        std::cerr << "Erreur: le fichier JSON doit contenir un tableau 'packet-classes'." << std::endl;
        return;
    }

    // Parcourir chaque définition de classe de paquet
    for (const auto& packetClass : j["packet-classes"]) {
        // Vérifie le type de la classe
        if (!packetClass.contains("type") || !packetClass["type"].is_string()) {
            std::cerr << "Erreur: chaque classe de paquet doit avoir un champ 'type'." << std::endl;
            continue;
        }

        std::string type = packetClass["type"];
        if (type == "distribution") {
            // Charger une classe de type distribution
            if (!packetClass.contains("payload-sizes") || !packetClass.contains("inter-packet-times")) {
                std::cerr << "Erreur: type 'distribution' doit contenir 'payload-sizes' et 'inter-packet-times'." << std::endl;
                continue;
            }

            // Charger les distributions pour les tailles de paquets
            std::vector<std::pair<uint32_t, double>> payloadSizes;
            for (const auto& size : packetClass["payload-sizes"]) {
                if (size.contains("size") && size.contains("prabability")) {
                    payloadSizes.emplace_back(size["size"].get<uint32_t>(), size["prabability"].get<double>());
                }
            }

            // Charger les distributions pour les temps inter-paquets
            std::vector<std::pair<double, double>> interPacketTimes;
            for (const auto& time : packetClass["inter-packet-times"]) {
                if (time.contains("time") && time.contains("prabability")) {
                    interPacketTimes.emplace_back(time["time"].get<double>(), time["prabability"].get<double>());
                }
            }

            // Créer l'objet PacketClassDistribution
            std::shared_ptr<PacketClassDistribution> distributionClass = std::make_shared<PacketClassDistribution>(payloadSizes, interPacketTimes);
            camera->AddPacketClass(distributionClass);

        } else if (type == "basic") {
            // Charger une classe de type basic
            if (!packetClass.contains("payload-size") || !packetClass.contains("inter-packet-times")) {
                std::cerr << "Erreur: type 'basic' doit contenir 'payload-size' et 'inter-packet-times'." << std::endl;
                continue;
            }

            // Charger les statistiques pour les tailles de paquets
            auto payloadSize = packetClass["payload-size"];
            double minSize = payloadSize["min"];
            double maxSize = payloadSize["max"];
            double meanSize = payloadSize["mean"];
            double stdDevSize = payloadSize["std-dev"];

            // Charger les statistiques pour les temps inter-paquets
            auto interPacketTimes = packetClass["inter-packet-times"];
            double minTime = interPacketTimes["min"];
            double maxTime = interPacketTimes["max"];
            double meanTime = interPacketTimes["mean"];
            double stdDevTime = interPacketTimes["std-dev"];

            // Créer l'objet PacketClassBasic
            std::shared_ptr<PacketClassBasic> basicClass = std::make_shared<PacketClassBasic>(
                minSize, maxSize, meanSize, stdDevSize, minTime, maxTime, meanTime, stdDevTime);
            camera->AddPacketClass(basicClass);

        } else {
            std::cerr << "Erreur: type inconnu '" << type << "'." << std::endl;
        }
    }
}

int 
main(int argc, char* argv[]) 
{
    double simTimeSec = 10.0;
    CommandLine cmd(__FILE__);
    cmd.AddValue("SimulationTime", "Length of simulation in seconds.", simTimeSec);
    cmd.Parse(argc, argv);

    Time::SetResolution(Time::NS);
    LogComponentEnableAll(LOG_PREFIX_TIME);
    LogComponentEnable("IotExample", LOG_INFO);
    //LogComponentEnable("IotCamera", LOG_INFO);
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

    /*std::vector<std::pair<uint32_t, double>> packetSizes = {
        {100, 0.5},
        {500, 0.3},
        {1000, 0.2}
    };
    std::vector<std::pair<double, double>> interPacketTimes = {
        {0.1, 0.4},
        {0.5, 0.4},
        {1.0, 0.2}
    };

    std::shared_ptr<PacketClassDistribution> 
        packetClass1 = std::make_shared<PacketClassDistribution>(packetSizes, interPacketTimes);
    
    std::vector<std::pair<uint32_t, double>> packetSizes2 = {
        {5000, 0.5},
        {10000, 0.3},
        {20000, 0.2}
    };
    std::vector<std::pair<double, double>> interPacketTimes2 = {
        {0.2, 0.4},
        {0.8, 0.4},
        {1.0, 0.2}
    };
    
    std::shared_ptr<PacketClassDistribution> 
        packetClass2 = std::make_shared<PacketClassDistribution>(packetSizes2, interPacketTimes2);
    std::shared_ptr<PacketClassBasic> packetClass1 = std::make_shared<PacketClassBasic>(
        691, 1448, 744.381, 191.231, 0.00008, 2.019497, 0.05936, 0.077852);
    camera->AddPacketClass(packetClass1);
    camera->AddPacketClass(packetClass2);*/

    LoadPacketClassFromFile(camera, "/home/augustin/projects/ens/ns/ns-allinone-3.43/ns-3.43/scratch/packet_class_example.json");
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
