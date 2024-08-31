#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/gnuplot.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TCPWirelessProgram");

int main(int argc, char* argv[])
{

	uint32_t nWifi = 1;

	std::string graphicsFileName1, graphicsFileNameFairness;
	std::string plotFileName1, plotFileNameFairness;
	std::string plotTitle;
	std::string dataTitle;

	Gnuplot2dDataset dataset1, datasetFairness;
	dataTitle = "Throughput";

	dataset1.SetTitle(dataTitle);
	dataset1.SetStyle(Gnuplot2dDataset::LINES_POINTS);

	datasetFairness.SetTitle("Fairness");
	datasetFairness.SetStyle(Gnuplot2dDataset::LINES_POINTS);

	std::string fileNameWithNoExtension1 = "Wireless-Throughput";
	std::string fileNameWithNoExtensionFairness = "Wireless-Fairness";

	graphicsFileName1 = fileNameWithNoExtension1 + ".png";
	plotFileName1 = fileNameWithNoExtension1 + ".plt";
	plotTitle = "Throughput Vs Packet Sizes";

	graphicsFileNameFairness = fileNameWithNoExtensionFairness + ".png";
	plotFileNameFairness = fileNameWithNoExtensionFairness + ".plt";

	uint32_t packetSize[10] = {50, 54, 58, 62, 70, 652, 676, 728, 1520,1600};
	std :: cout << "---------------------------------------------------------" << std::endl<< std::endl;
	for(int j = 0 ; j < 10 ; j++)
	{

		Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (packetSize[j]));

		NodeContainer p2pNodes;
		p2pNodes.Create (2);

		PointToPointHelper pointToPoint;
		pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
		pointToPoint.SetChannelAttribute ("Delay", StringValue ("100ms"));

		NetDeviceContainer p2pDevices;
		p2pDevices = pointToPoint.Install (p2pNodes);

		NodeContainer wifiStaNode0;
		wifiStaNode0.Create (nWifi);

		NodeContainer wifiStaNode1;
		wifiStaNode1.Create (nWifi);

		NodeContainer wifiApNodeBS1 = p2pNodes.Get (0);
		NodeContainer wifiApNodeBS2 = p2pNodes.Get (1);

		YansWifiChannelHelper channelBS1 = YansWifiChannelHelper::Default ();
		YansWifiPhyHelper phyBS1 = YansWifiPhyHelper();

		YansWifiChannelHelper channelBS2 = YansWifiChannelHelper::Default ();
		YansWifiPhyHelper phyBS2 = YansWifiPhyHelper();

		phyBS1.SetChannel (channelBS1.Create ());
		phyBS2.SetChannel (channelBS2.Create ());

		WifiMacHelper macBS1;
		Ssid ssidBS1 = Ssid ("ns-3-ssid");

		WifiMacHelper macBS2;
		Ssid ssidBS2 = Ssid ("ns-3-ssid");

		WifiHelper wifiBS1;
		WifiHelper wifiBS2;

		wifiBS1.SetStandard (WIFI_STANDARD_80211n);
		wifiBS2.SetStandard (WIFI_STANDARD_80211n);

		wifiBS1.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue ("HtMcs7"),"ControlMode", StringValue ("HtMcs0"));
		wifiBS2.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue ("HtMcs7"),"ControlMode", StringValue ("HtMcs0"));

		macBS1.SetType ("ns3::StaWifiMac",
		"Ssid", SsidValue (ssidBS1),
		"ActiveProbing", BooleanValue (false));

		macBS2.SetType ("ns3::StaWifiMac",
		"Ssid", SsidValue (ssidBS2),
		"ActiveProbing", BooleanValue (false));

		NetDeviceContainer staDevice0;
		staDevice0 = wifiBS1.Install (phyBS1, macBS1, wifiStaNode0);

		NetDeviceContainer staDevice1;
		staDevice1 = wifiBS2.Install (phyBS2, macBS1, wifiStaNode1);

		macBS1.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssidBS1));
		macBS2.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssidBS2));

		NetDeviceContainer apDevicesBS1;
		apDevicesBS1 = wifiBS1.Install (phyBS1, macBS2, wifiApNodeBS1);

		NetDeviceContainer apDevicesBS2;
		apDevicesBS2 = wifiBS2.Install (phyBS2, macBS2, wifiApNodeBS2);

		MobilityHelper mobility;

		mobility.SetPositionAllocator ("ns3::GridPositionAllocator", 
										"MinX", 
										DoubleValue (0.0), 
										"MinY", DoubleValue (0.0), 
										"DeltaX", DoubleValue (5.0),
										"DeltaY", DoubleValue (10.0), 
										"GridWidth", UintegerValue (4), 
										"LayoutType", 
										StringValue ("RowFirst"));

		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		mobility.Install (wifiStaNode0);
		mobility.Install (wifiApNodeBS1);
		mobility.Install (wifiApNodeBS2);
		mobility.Install (wifiStaNode1);

		InternetStackHelper stack;
		stack.Install (wifiApNodeBS1);
		stack.Install (wifiStaNode0);
		stack.Install (wifiApNodeBS2);
		stack.Install (wifiStaNode1);

		Ipv4AddressHelper address;

		address.SetBase ("10.1.1.0", "255.255.255.0");
		Ipv4InterfaceContainer p2pInterfaces;
		p2pInterfaces = address.Assign (p2pDevices);

		address.SetBase ("10.1.2.0", "255.255.255.0");
		Ipv4InterfaceContainer wifiInterfaceContainer0 = address.Assign (staDevice0);
		address.Assign (apDevicesBS1);

		address.SetBase ("10.1.3.0", "255.255.255.0");
		Ipv4InterfaceContainer wifiInterfaceContainer1 = address.Assign (staDevice1);
		address.Assign (apDevicesBS2);

		Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

		uint32_t ps = packetSize[j];

		TypeId tid = TypeId::LookupByName("ns3::TcpWestwoodPlus");
		std::stringstream nodeId;
		nodeId << wifiStaNode0.Get(0)->GetId();
		std::string specificNode = "/NodeList/" + nodeId.str() + "/$ns3::TcpL4Protocol/SocketType";
		Config::Set(specificNode, TypeIdValue(tid));

		uint16_t sinkPort = 8080;

		Address sinkAddress (InetSocketAddress(wifiInterfaceContainer1.GetAddress (0), sinkPort));
		PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
		ApplicationContainer sinkApps = packetSinkHelper.Install (wifiStaNode1.Get (0));
		sinkApps.Start (Seconds (0.));

		OnOffHelper serverHelper1 ("ns3::TcpSocketFactory", (InetSocketAddress (wifiInterfaceContainer1.GetAddress (0), sinkPort)));
		serverHelper1.SetAttribute ("PacketSize", UintegerValue (ps));
		serverHelper1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
		serverHelper1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
		serverHelper1.SetAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));

		ApplicationContainer serverApp1 = serverHelper1.Install (wifiStaNode0.Get(0));

		tid = TypeId::LookupByName("ns3::TcpVeno");
		Config::Set(specificNode, TypeIdValue(tid));

		OnOffHelper serverHelper2 ("ns3::TcpSocketFactory", (InetSocketAddress (wifiInterfaceContainer1.GetAddress (0), sinkPort)));
		serverHelper2.SetAttribute ("PacketSize", UintegerValue (ps));
		serverHelper2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
		serverHelper2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
		serverHelper2.SetAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));

		ApplicationContainer serverApp2 = serverHelper2.Install (wifiStaNode0.Get(0));

		tid = TypeId::LookupByName("ns3::TcpVegas");
		Config::Set(specificNode, TypeIdValue(tid));

		OnOffHelper serverHelper3 ("ns3::TcpSocketFactory", (InetSocketAddress (wifiInterfaceContainer1.GetAddress (0), sinkPort)));
		serverHelper3.SetAttribute ("PacketSize", UintegerValue (ps));
		serverHelper3.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
		serverHelper3.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
		serverHelper3.SetAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));

		ApplicationContainer serverApp3 = serverHelper3.Install (wifiStaNode0.Get(0));

		serverApp1.Start (Seconds (10.0));
		serverApp2.Start (Seconds (10.0));
		serverApp3.Start (Seconds (10.0));

		Ptr<FlowMonitor> flowMonitor;
		FlowMonitorHelper flowHelper;
		flowMonitor = flowHelper.InstallAll();

		Simulator::Stop(Seconds(11.0));
		Simulator::Run();

		FlowMonitor :: FlowStatsContainer fsc = flowMonitor->GetFlowStats();
		if(fsc.empty()) std::cout << "No flow";
		FlowMonitor :: FlowStatsContainerI it;

		uint32_t flowID;
		double avg_throughput = 0.0;
		double throughput_sum = 0.0, throughput_square_sum = 0.0;

		std::cout << "Packet-Size : " << ps << std::endl<< std::endl;
		for(it = fsc.begin() ; it != fsc.end(); it++)
		{
			FlowMonitor :: FlowStats fs = it->second;
			flowID = it->first;
			uint64_t recv_bytes = fs.rxBytes;
			Time t = fs.timeFirstTxPacket;
			double t1 = t.GetSeconds();
			t = fs.timeLastRxPacket;
			double t2 = t.GetSeconds();

			double throughput = (recv_bytes * 8.0) / (t2-t1);
			throughput = throughput / 1024;
			throughput_sum += throughput;
			throughput_square_sum += throughput * throughput;

			if(flowID == 1 || flowID == 2 || flowID == 3) 
			{	
				std::cout << "n0 ---> n1" << std::endl;
			}
			else 
			{
				std::cout << "n1 ---> n0" << std::endl;
			}
			if(flowID == 1 || flowID == 4) 
			{	
				std::cout << "TCP Flow Type : Westwood" << std::endl;
			}
			if(flowID == 2 || flowID == 5) 
			{	
				std::cout << "TCP Flow Type : Veno" << std::endl; 
			}
			if(flowID == 3 || flowID == 6) 
			{	
				std::cout << "TCP Flow Type : Vegas" << std::endl;  
			}
			std::cout << "Throughput : " << throughput << " Kbps"<< std::endl; 
			std::cout << "Recevied Bytes : " << recv_bytes << std::endl; 
			std::cout << "Time : " << t2-t1 << " s" << std::endl;
			std::cout<<"Lost Packets:= "<<fs.lostPackets<<std::endl<< std::endl;	

		}
		avg_throughput = throughput_sum / 6.0;
		dataset1.Add(packetSize[j], avg_throughput);

		double fairness = (throughput_sum * throughput_sum) / (6.0 * throughput_square_sum);
		datasetFairness.Add(packetSize[j], fairness);

		std :: cout << "Average Throughput: " << avg_throughput << " Kbps" << std::endl;
		std :: cout << "FairnessIndex: " << fairness << std::endl;

		std :: cout << "---------------------------------------------------------" << std::endl<< std::endl;

		Simulator::Destroy ();
	}

	Gnuplot plot1(graphicsFileName1);
	plot1.SetTitle(plotTitle);

	plot1.SetTerminal("png");

	plot1.SetLegend("Packet Size(Bytes)", "Throughput(Kbps)");

	plot1.AppendExtra("set xrange [0:1600]");

	plot1.AddDataset(dataset1);

	std::ofstream plotFile1(plotFileName1.c_str());

	plot1.GenerateOutput(plotFile1);

	plotFile1.close();

	Gnuplot plot4(graphicsFileNameFairness);
	plot4.SetTitle("Fairness Vs Packet Size");

	plot4.SetTerminal("png");

	plot4.SetLegend("Packet Size(Bytes)", "Fairness Index");

	plot4.AppendExtra("set xrange [0:1600]");
	plot4.AppendExtra("set yrange [0:1]");
	plot4.AddDataset(datasetFairness);

	std::ofstream plotFile4(plotFileNameFairness.c_str());

	plot4.GenerateOutput(plotFile4);

	plotFile4.close();
	return 0;
}
