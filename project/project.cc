#include "ns3/core-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/animation-interface.h"

#include <iostream>
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ScratchSimulator");

class LinearCongruentialGenerator {

public:
  const uint32_t m;
  const uint32_t a;
  const uint32_t c;
  uint32_t prev;

  LinearCongruentialGenerator(uint32_t m, uint32_t a, uint32_t c, uint32_t seed = 0)
  : m(m), a(a), c(c), prev(seed) {}

  uint32_t gen(){
    prev = (a*prev + c) % m;
    return prev;
  }
};

double exp_dist_from_uniform(double lambda, double u)
{
  double ret = -log(u) / lambda;
  return ret;
}

static void received_msg (Ptr<Socket> socket1, Ptr<Socket> socket2, Ptr<const Packet> p, const Address &srcAddress , const Address &dstAddress)
{
	std::cout << "::::: A packet received at the Server! Time:   " << Simulator::Now ().GetSeconds () << std::endl;
	
	Ptr<UniformRandomVariable> rand=CreateObject<UniformRandomVariable>();
	
	if(rand->GetValue(0.0,1.0)<=0.5){
		std::cout << "::::: Transmitting from Server to Router   "  << std::endl;
		socket1->Send (Create<Packet> (p->GetSize ()));
	}
	else{
		std::cout << "::::: Transmitting from GW to Controller   "  << std::endl;
		socket2->SendTo(Create<Packet> (p->GetSize ()),0,srcAddress);
	}
}

static void GenerateTraffic (Ptr<Socket> socket, Ptr<ExponentialRandomVariable> randomSize,	Ptr<ExponentialRandomVariable> randomTime)
{
	uint32_t pktSize = randomSize->GetInteger (); //Get random value for packet size
	// std::cout << "::::: A packet is generate at Node "<< socket->GetNode ()->GetId () << " with size " << pktSize << " bytes ! Time:   " << Simulator::Now ().GetSeconds () << std::endl;
	
	// We make sure that the message is at least 12 bytes. The minimum length of the UDP header. We would get error otherwise.
	if(pktSize<12){
		pktSize=12;
	}
	
	socket->Send (Create<Packet> (pktSize));

	Time pktInterval = Seconds(randomTime->GetValue ()); //Get random value for next packet generation time 
	Simulator::Schedule (pktInterval, &GenerateTraffic, socket, randomSize, randomTime); //Schedule next packet generation
}

void SchedulePackets(Ptr<Socket> source, double meanTime, double meanSize) {
  
  Ptr<ExponentialRandomVariable> randomTime = CreateObject<ExponentialRandomVariable> ();
  randomTime->SetAttribute ("Mean", DoubleValue (meanTime));
   
   Ptr<ExponentialRandomVariable> randomSize = CreateObject<ExponentialRandomVariable> ();
   randomSize->SetAttribute ("Mean", DoubleValue (meanSize));

   Simulator::ScheduleWithContext (source->GetNode()->GetId(), Seconds (2.0), &GenerateTraffic, source, randomSize, randomTime);
}

int main (int argc, char *argv[])
{
  const uint32_t m = 100;
  const uint32_t a = 21;
  const uint32_t c = 1;
  const uint32_t seed = 1;
  LinearCongruentialGenerator lcg(m, a, c, seed);

  //Ptr<UniformRandomVariable> urv = CreateObject<UniformRandomVariable>();

  Ptr<ExponentialRandomVariable> erv = CreateObject<ExponentialRandomVariable>();

  std::ofstream lcg_file;
  lcg_file.open ("lcg_exp_output.txt", std::fstream::in | std::fstream::out | std::fstream::trunc);

  std::ofstream erv_file;
  erv_file.open("erv_output.txt", std::fstream::in | std::fstream::out | std::fstream::trunc);

  const double lambda = 50;

  for(int i = 0; i < 1000; i++){
    //double rn;
    // rn = static_cast<double>(lcg.gen()) / static_cast<double>(m);
    //rn = urv->GetValue(0.0, 1.0);
    //std::cout << rn << "\n";

    double lcg_rn = 1-(static_cast<double>(lcg.gen()) / static_cast<double>(m));
    lcg_file << exp_dist_from_uniform(lambda, lcg_rn) << "\n";
    //lcg_file << lcg_rn << "\n";
    erv_file << erv->GetValue(1.0/lambda, 1.0) << "\n";
  }

  lcg_file.close();
  erv_file.close();

  NodeContainer nodes;
  nodes.Create(9);

  InternetStackHelper stack;
  stack.Install(nodes);

  PointToPointHelper p2pAE;
  p2pAE.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  
  PointToPointHelper p2pEG;
  p2pEG.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  
  PointToPointHelper p2pGS;
  p2pGS.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  
  PointToPointHelper p2pBF;
  p2pBF.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  
  PointToPointHelper p2pFG;
  p2pFG.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));

  PointToPointHelper p2pCF;
  p2pCF.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));

  PointToPointHelper p2pDG;
  p2pDG.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));

  PointToPointHelper p2pGR;
  p2pDG.SetDeviceAttribute ("DataRate", StringValue ("8Mbps"));

  Ipv4AddressHelper adress;
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;

  /*
    A: 0
    B: 1
    C: 2
    D: 3
    E: 4
    F: 5
    G: 6
    S: 7
  */

  devices.Add(p2pAE.Install(nodes.Get(0), nodes.Get(4)));
  adress.SetBase("10.1.1.0", "255.255.255.0");
  interfaces = adress.Assign(devices);

  devices.Add(p2pBF.Install(nodes.Get(1), nodes.Get(5)));
  adress.SetBase("10.1.2.0", "255.255.255.0");
  interfaces = adress.Assign(devices);

  devices.Add(p2pCF.Install(nodes.Get(2), nodes.Get(5)));
  adress.SetBase("10.1.3.0", "255.255.255.0");
  interfaces = adress.Assign(devices);

  devices.Add(p2pDG.Install(nodes.Get(3), nodes.Get(6)));
  adress.SetBase("10.1.4.0", "255.255.255.0");
  interfaces = adress.Assign(devices);

  devices.Add(p2pEG.Install(nodes.Get(4), nodes.Get(6)));
  adress.SetBase("10.1.5.0", "255.255.255.0");
  interfaces = adress.Assign(devices);

  devices.Add(p2pGS.Install(nodes.Get(6), nodes.Get(7)));
  adress.SetBase("10.1.6.0", "255.255.255.0");
  interfaces = adress.Assign(devices);

  devices.Add(p2pFG.Install(nodes.Get(5), nodes.Get(6)));
  adress.SetBase("10.1.7.0", "255.255.255.0");
  interfaces = adress.Assign(devices);

  devices.Add(p2pGR.Install(nodes.Get(6), nodes.Get(8)));
  adress.SetBase("10.1.8.0", "255.255.255.0");
  interfaces = adress.Assign(devices);


  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  AnimationInterface anim("network.xml");
  anim.SetConstantPosition(nodes.Get(0), -80.0, -20.0);
  anim.SetConstantPosition(nodes.Get(1), -80.0, 20.0);
  anim.SetConstantPosition(nodes.Get(2), -40.0, 60.0);
  anim.SetConstantPosition(nodes.Get(3), 0.0, 40.0);
  anim.SetConstantPosition(nodes.Get(4), -40.0, -20.0);
  anim.SetConstantPosition(nodes.Get(5), -40.0, 20.0);
  anim.SetConstantPosition(nodes.Get(6), 0.0, 0.0);
  anim.SetConstantPosition(nodes.Get(7), 0.0, -20.0);
  anim.SetConstantPosition(nodes.Get(8), 50.0, 0.0);

  NS_LOG_INFO ("Create Applications.");
	//
	// Create a UdpServer application on node Server (S).
	//
  uint16_t port_number = 9;  
  ApplicationContainer server_apps;
  UdpServerHelper serverS (port_number);
  server_apps.Add(serverS.Install(nodes.Get (7)));
   
  Ptr<UdpServer> S1 = serverS.GetServer();

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
   
   //Transmission Server (S)-> Router (R)
  Ptr<Socket> sourceRouter = Socket::CreateSocket (nodes.Get(7), tid);
  InetSocketAddress remoteRouter = InetSocketAddress (interfaces.GetAddress (15), port_number);
  sourceRouter->Connect (remoteRouter);
  
  //Transmission Server (S) -> Client (A or B)
  Ptr<Socket> sourceClients = Socket::CreateSocket (nodes.Get(7), tid);
   
  S1->TraceConnectWithoutContext ("RxWithAddresses", MakeBoundCallback (&received_msg, sourceRouter, sourceClients));
   
  server_apps.Start (Seconds (1.0));
  server_apps.Stop (Seconds (10.0));
   
  //
	// Create a UdpServer application on node A,B,C,D to receive the reply from the server.
	//
  UdpServerHelper server (port_number);
  server_apps.Add(server.Install(nodes.Get (0)));
  server_apps.Add(server.Install(nodes.Get (1)));
  server_apps.Add(server.Install(nodes.Get (2)));
  server_apps.Add(server.Install(nodes.Get (3)));

  Ptr<Socket> sourceA = Socket::CreateSocket (nodes.Get(0), tid);
  InetSocketAddress remoteA = InetSocketAddress (interfaces.GetAddress(11), port_number);
  sourceA->Connect (remoteA);

  Ptr<Socket> sourceB = Socket::CreateSocket (nodes.Get(1), tid);
  InetSocketAddress remoteB = InetSocketAddress (interfaces.GetAddress(11), port_number);
  sourceB->Connect (remoteB);

  Ptr<Socket> sourceC = Socket::CreateSocket (nodes.Get(2), tid);
  InetSocketAddress remoteC = InetSocketAddress (interfaces.GetAddress(11), port_number);
  sourceC->Connect (remoteC);

  Ptr<Socket> sourceD = Socket::CreateSocket (nodes.Get(3), tid);
  InetSocketAddress remoteD = InetSocketAddress (interfaces.GetAddress(11), port_number);
  sourceD->Connect (remoteD);

  SchedulePackets(sourceA, 0.002, 100);
  SchedulePackets(sourceB, 0.002, 100);
  SchedulePackets(sourceC, 0.0005, 100);
  SchedulePackets(sourceD, 0.001, 100);

  for(uint32_t i = 0; i < interfaces.GetN(); i++)
    std::cout << i << ": " << interfaces.GetAddress(i) << "\n";

  Simulator::Stop(Seconds (10));
  Simulator::Run();
  Simulator::Destroy();

}

