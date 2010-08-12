#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <boost/program_options.hpp>
#include <boost/noncopyable.hpp>
#include <boost/random.hpp>
#include <algorithm>
#include "asmsg.hpp"

void dump_avg_hop(const std::string& msg, const global_nodes::avg_var& av);


int main(int argc, char** argv){
  config& c = config::instance();
  //exit(0);
  // parse options
  {
    namespace po = boost::program_options;
    po::options_description opt("options");
    std::string master;
    opt.add_options()
      ("help,h", "display this help")
      ("hops,H", "count hops mode")
      ("connections,c", "count average connections")
      ("nodes,n", po::value<size_t>
       (&c.nodes)->default_value(20), "node quontum")
      ("keys,k", po::value<size_t>
       (&c.keys)->default_value(20), "key number")
      ("level,l", po::value<size_t>
       (&c.level)->default_value(8), "max level")
      ("dump,d", "node-key map dump")
      ("graph,g", "draw graph")
      ("seed,s", po::value<int>
       (&c.seed)->default_value(time(0)), "random seed");
    po::variables_map vm;
    store(parse_command_line(argc,argv,opt), vm);
    notify(vm);
    if(vm.count("help")){
      std::cout << opt << std::endl;
      return 0;
    }
    
    if(vm.count("graph")){c.graph = true;}
    if(vm.count("dump")){c.nodedump = true;}
    if(vm.count("connections")){c.connections = true;}
    if(vm.count("hops")){c.hopdump = true;}
    if(c.keys < c.nodes){
      std::cerr << "key quantum must be larger than node quantum." << std::endl;
      exit(0);
    }
    assert(c.nodes > 0);
    assert(c.level < 64);
  }
  c.dump(); // setting dump 
  
  int seed = c.seed;
  global_nodes world(seed);
  world.set_nodes(c.nodes);
  boost::mt19937 wrand(c.seed);
  
  size_t nextkey = 0;
  BOOST_FOREACH(node n, world.nodes){
    world.put_key(key(nextkey++,rand64(wrand)),n.vector_range_begin_.vector+1);
  }
  
  while(nextkey<c.keys){
    world.put_key(key(nextkey++,rand64(wrand)),rand64(wrand));
  }
  world.organize_skipgraph(c.level);
  //world.dump(c.level);
  if(c.nodedump){world.node_dump(c.level);}
  if(c.connections){
    global_nodes::avg_var result = world.count_neighbor(10);
    dump_avg_hop("connections ", result);
  }
  if(c.hopdump){
    global_nodes::avg_var result = world.count_average_hop(c.level);
    dump_avg_hop("hops ", result);
  }
  if(c.graph){ world.dump(c.level); }
}

void dump_avg_hop(const std::string& msg, const global_nodes::avg_var& av){
  std::cout << msg.c_str() << " avg:" << av.first << " var:" << av.second << std::endl; 
}
