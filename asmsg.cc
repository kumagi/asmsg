
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
  c.dump();
  
  global_nodes world;
  world.set_nodes(c.nodes,c.rand);
  
  for(size_t i=0; i<c.keys; i++){
    key newkey(rand(),membership_vector(rand64(c.rand)));
    world.put_key(newkey,c.vector_len);
  }
  
}
