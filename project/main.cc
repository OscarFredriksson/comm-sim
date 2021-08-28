#include "ns3/core-module.h"

#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ScratchSimulator");

class LinearCongruentialGenerator {

public:
  const int m;
  const int a;
  const int c;
  int prev;

  LinearCongruentialGenerator(int m, int a, int c, int seed = 0)
  : m(m), a(a), c(c), prev(seed) {}

  int gen(){
    prev = (a*prev + c) % m;
    return prev;
  }
};

int main (int argc, char *argv[])
{
  const int m = 100;
  const int a = 13;
  const int c = 1;
  const int seed = 1;
  LinearCongruentialGenerator lcg(m, a, c, seed);

  for(int i = 0; i < 1000; i++){
    std::cout << lcg.gen() << "\n";
  }

  Simulator::Run ();
  Simulator::Destroy ();
}