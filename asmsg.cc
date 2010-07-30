
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <boost/program_options.hpp>
#include <boost/noncopyable.hpp>
#include <boost/random.hpp>
#include "asmsg.hpp"



int main(int argc, char** argv){
  config& c = config::instance();
  
  // parse options
  {
    namespace po = boost::program_options;
    po::options_description opt("options");
    std::string master;
    opt.add_options()
      ("nodes,n", po::value<size_t>
       (&c.nodes)->default_value(10), "node number")
      ("keys,k", po::value<size_t>
       (&c.keys)->default_value(100), "key number")
      ("vector,v", po::value<size_t>
       (&c.vector_len)->default_value(32), "key number");
    po::variables_map vm;
    store(parse_command_line(argc,argv,opt), vm);
    notify(vm);
    if(vm.count("help")){
      std::cout << opt << std::endl;
      return 0;
    }
    assert(c.nodes > 0);
  }
  c.dump(); // setting dump
  
  int seed = 0;
  global_nodes world(seed);
  world.set_nodes(c.nodes);
  boost::mt19937 wrand;
  for(int i=0;i<1;i++){
    world.put_key(key(i,rand64(wrand)),rand64(wrand));
  }
  world.dump(4);
  /*
  world.put_key(key(3,rand64(rand)), 3);
  world.dumpkeys(3);
  std::cout << "3 inserted" << std::endl;
  world.put_key(key(5,rand64(rand)), 3);
  world.dumpkeys(3);
  std::cout << "5 inserted" << std::endl;
  */
}
