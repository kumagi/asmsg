
#include <stdlib.h>
#include <boost/program_options.hpp>
#include <boost/noncopyable.hpp>
#include <boost/random.hpp>
#include "asmsg.hpp"




int main(int argc, char** argv){
  
  srand(time(NULL));
  config& c = config::instance();
  
  // parse options
  namespace po = boost::program_options;
  po::options_description opt("options");
  std::string master;
  opt.add_options()
    ("nodes,n", po::value<int>
     (&c.nodes)->default_value(100), "node number")
    ("keys,k", po::value<int>
     (&c.keys)->default_value(100000), "key number")
    ("vector,v", po::value<int>
     (&c.vector_len)->default_value(32), "key number");
  boost::mt19937 rand;
  std::vector<node> nodes;
  
  
}
