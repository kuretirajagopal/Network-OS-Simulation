#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/gnuplot.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TCPWiredProgram");


class MyApp : public Application
{
	public:
		
		MyApp ();
		
		virtual ~MyApp();
		void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, 
		uint32_t nPackets, DataRate dataRate);
  
	private:
		virtual void StartApplication (void);
		virtual void StopApplication (void);
		void ScheduleTx (void);
		void SendPacket (void);
		Ptr<Socket>     m_socket;
		Address         m_peer;
		uint32_t        m_packetSize;
		uint32_t        m_nPackets;
		DataRate        m_dataRate;
		EventId         m_sendEvent;
		bool            m_running;
		uint32_t        m_packetsSent;    
};
MyApp::MyApp ()
: m_socket (0),
m_peer (),
m_packetSize (0),
m_nPackets (0),
m_dataRate (0),
m_sendEvent (),
m_running (false),
m_packetsSent (0)
{
}
  
MyApp::~MyApp()
{
	m_socket = 0;
}
void MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
	m_socket = socket;
	m_peer = address;
	m_packetSize = packetSize;
	m_nPackets = nPackets;
	m_dataRate = dataRate;
}


void MyApp::StartApplication (void)
{
	m_running = true;
	m_packetsSent = 0;
	m_socket->Bind ();
	m_socket->Connect (m_peer);
	SendPacket ();
}
  
void MyApp::StopApplication (void)
{
	m_running = false;
	if (m_sendEvent.IsRunning ())
	{
		Simulator::Cancel (m_sendEvent);
	}
	if (m_socket)
	{
		m_socket->Close ();
	}
}
void MyApp::SendPacket (void)
{
	Ptr<Packet> packet = Create<Packet> (m_packetSize);
	m_socket->Send (packet);
	if (++m_packetsSent < m_nPackets)
	{
		ScheduleTx ();
	}
}
void MyApp::ScheduleTx (void)
{
	if (m_running)
	{
		Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
		m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
	}
}


int main(int argc, char* argv[])
{   
	Time::SetResolution(Time::NS);
	LogComponentEnable("TcpSocket", LOG_LEVEL_INFO);
	LogComponentEnable("PointToPointHelper", LOG_LEVEL_INFO);
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
	std::string fileNameWithNoExtension1 = "Wired-Throughput";
	std::string fileNameWithNoExtensionFairness = "Wired-Fairness";
	graphicsFileName1 = fileNameWithNoExtension1 + ".png";
	plotFileName1 = fileNameWithNoExtension1 + ".plt";
	plotTitle = "Throughput Vs Packet Sizes";
		    
	graphicsFileNameFairness        = fileNameWithNoExtensionFairness + ".png";
	plotFileNameFairness            = fileNameWithNoExtensionFairness + ".plt";
	uint32_t packetSize[10] = {50, 54, 58, 62, 70, 652, 676, 728, 1520,1600};
	std :: cout << "---------------------------------------------------------" << std::endl<< std::endl;
	for(int j = 0 ; j < 10 ; j++)
	{		     
		NodeContainer n2;
		n2.Create(1);
		NodeContainer n3;
		n3.Create(1);
		NodeContainer r1;
		r1.Create(1);
		NodeContainer r2;
		r2.Create(1);
		n2.Add(r1);
		PointToPointHelper pointToPoint1;
		pointToPoint1.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
		pointToPoint1.SetChannelAttribute("Delay", StringValue("20ms"));
		NetDeviceContainer n2r1;
		n2r1 = pointToPoint1.Install(n2);
		r1.Add(r2);
		PointToPointHelper pointToPoint2;
		pointToPoint2.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
		pointToPoint2.SetChannelAttribute("Delay", StringValue("50ms"));
		pointToPoint2.SetQueue ("ns3::DropTailQueue", "MaxSize", QueueSizeValue(QueueSize(QueueSizeUnit::BYTES, 62500)));
		NetDeviceContainer r1r2;
		r1r2 = pointToPoint2.Install(r1);
		r2.Add(n3);
		PointToPointHelper pointToPoint3;
		pointToPoint3.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
		pointToPoint3.SetChannelAttribute("Delay", StringValue("20ms"));
		NetDeviceContainer r2n3;
		r2n3 = pointToPoint3.Install(r2);
		n2.Add(r2);
		InternetStackHelper stack;
		stack.Install(n2);
		Ipv4AddressHelper address;
		address.SetBase("10.1.1.0", "255.255.255.0");
		Ipv4InterfaceContainer n2r1Interfaces = address.Assign(n2r1);
		address.SetBase("10.1.2.0", "255.255.255.0");
		Ipv4InterfaceContainer r1r2Interfaces = address.Assign(r1r2);
		address.SetBase("10.1.3.0", "255.255.255.0");
		Ipv4InterfaceContainer r2n3Interfaces = address.Assign(r2n3);
		Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
		uint32_t ps = packetSize[j];   	
		TypeId tid = TypeId::LookupByName("ns3::TcpWestwoodPlus");
		std::stringstream nodeId;
		nodeId << n2.Get(0)->GetId();
		std::string specificNode = "/NodeList/" + nodeId.str() + "/$ns3::TcpL4Protocol/SocketType";
		Config::Set(specificNode, TypeIdValue(tid));
		uint16_t sinkPort = 8080;
		Address sinkAddress (InetSocketAddress(r2n3Interfaces.GetAddress (1), sinkPort));
		PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
		ApplicationContainer sinkApps = packetSinkHelper.Install (n3.Get (0));
		sinkApps.Start (Seconds (0.));
		sinkApps.Stop (Seconds (10.));
		Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (n2.Get (0), TcpSocketFactory::GetTypeId ());
		Ptr<MyApp> app = CreateObject<MyApp> ();
		app->Setup (ns3TcpSocket, sinkAddress, ps, 100, DataRate ("100Mbps"));
		n2.Get (0)->AddApplication (app);
		app->SetStartTime(Seconds(1.0));
		app->SetStopTime(Seconds(10.0));
		tid = TypeId::LookupByName("ns3::TcpVeno");
		Config::Set(specificNode, TypeIdValue(tid));
		Ptr<Socket> ns3TcpSocket2 = Socket::CreateSocket (n2.Get (0), TcpSocketFactory::GetTypeId ());
		Ptr<MyApp> app2 = CreateObject<MyApp> ();
		app2->Setup (ns3TcpSocket2, sinkAddress, ps, 100, DataRate ("100Mbps"));
		n2.Get (0)->AddApplication (app2);
		app2->SetStartTime(Seconds(1.0));
		app2->SetStopTime(Seconds(10.0));
		tid = TypeId::LookupByName("ns3::TcpVegas");
		Config::Set(specificNode, TypeIdValue(tid));
		Ptr<Socket> ns3TcpSocket3 = Socket::CreateSocket (n2.Get (0), TcpSocketFactory::GetTypeId ()); 
		Ptr<MyApp> app3 = CreateObject<MyApp> ();
		app3->Setup (ns3TcpSocket3, sinkAddress, ps, 100, DataRate ("100Mbps"));
		n2.Get (0)->AddApplication (app3);
		app3->SetStartTime(Seconds(1.0));
		app3->SetStopTime(Seconds(10.0));
		Ptr<FlowMonitor> flowMonitor;
		FlowMonitorHelper flowHelper;
		flowMonitor = flowHelper.InstallAll();
		Simulator::Stop(Seconds(15.0));
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
			if(flowID == 1 || flowID == 2 || flowID == 3) std::cout << "n2 ---> n3" << std::endl;
			else std::cout << "n3 ---> n2" << std::endl;
			if(flowID == 1 || flowID == 4) std::cout << "TCP Flow Type : Westwood" << std::endl;
			if(flowID == 2 || flowID == 5) std::cout << "TCP Flow Type : Veno" << std::endl; 
			if(flowID == 3 || flowID == 6) std::cout << "TCP Flow Type : Vegas" << std::endl;  
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
		Simulator::Destroy();
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
