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
  nodes.Create (8);

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

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  AnimationInterface anim("network.xml");
  anim.SetConstantPosition(nodes.Get(0), -8.0, -2.0);
  anim.SetConstantPosition(nodes.Get(1), -8.0, 2.0);
  anim.SetConstantPosition(nodes.Get(2), -4.0, -6.0);
  anim.SetConstantPosition(nodes.Get(3), 0.0, -4.0);
  anim.SetConstantPosition(nodes.Get(4), -4.0, -2.0);
  anim.SetConstantPosition(nodes.Get(5), -4.0, 2.0);
  anim.SetConstantPosition(nodes.Get(6), 0.0, 0.0);
  anim.SetConstantPosition(nodes.Get(7), 0.0, -4.0);

  Simulator::Run ();
  Simulator::Destroy ();
}