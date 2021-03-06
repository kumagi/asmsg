#include "asmsg.hpp"
#include <numeric>

class calc_var{
  const double avg;
  double sum;
public:
  calc_var(const double& a):avg(a),sum(0.0){}
  void operator()(const double& dat){
    double diff = std::abs(dat-avg);
    sum+=diff*diff;
  }
  double accum()const{ return sum;}
};


void global_nodes::put_key(const key& newkey, uint64_t putnode){
  node_list::iterator target_node =
    lower_bound(nodes.begin(),nodes.end(),node(putnode));
  if(target_node != nodes.begin() && target_node != nodes.end()){target_node--;}
  if(target_node == nodes.end()){target_node = --nodes.end();}
  target_node->add_key(newkey);
}

void global_nodes::key_dump(size_t maxlevel)const{
  typedef std::pair<key, const node*> kn;
  BOOST_FOREACH(const kn& knp, keys){
    knp.first.dump_lr(maxlevel);
    std::cerr << std::endl;
  }
}

void global_nodes::set_nodes(size_t targets){
  //assert(targets > 1);
  while(nodes.size() < targets-1){
    const int targetsize = targets - nodes.size();
    nodes.reserve(nodes.size() + targetsize);
    for(int i=0; i<targetsize-1 ;i++){
      nodes.push_back(node(rand64(rand)));
    }
    std::sort(nodes.begin(),nodes.end());
    nodes.erase(std::unique(nodes.begin(),nodes.end()),nodes.end());
  }
  if(!nodes.empty()){
    nodes.insert(nodes.begin(),node(0,nodes[0].vector_range_end_));
  }else{
    nodes.insert(nodes.begin(),node(0,0));
  }
  assert(nodes.size() == targets);
  // assign range
  for(size_t i=0; i<nodes.size()-1; i++){
    nodes[i].vector_range_end_ = nodes[i+1].vector_range_begin_;
  }
  nodes.rbegin()->vector_range_end_ = 0;
}


void global_nodes::organize_skipgraph(int maxlevel){
  fprintf(stderr,"organizing skip graph");
  // create key-node pair list
  key_map tmpmap;
  {// count all key-node pair
    BOOST_FOREACH(node& n, nodes){
      // fixme: 
      BOOST_FOREACH(const key& k, n.keys_){
	tmpmap.insert(std::make_pair(k,&n));
      }
    }
  }
  
  int cnt = 0;
  fprintf(stderr,"\rrefreshing keymap   ");
  for(key_map::iterator kn = tmpmap.begin(); kn != tmpmap.end(); ++kn){
    key& k =  const_cast<key&>(kn->first);
    key_map::iterator it = boost::next(kn);
    if(it == tmpmap.end()) break;
    for(int i=0; i<maxlevel;){
      key& rkn = const_cast<key&>(it->first);
      int matched = k.mv.match(rkn.mv);
	
      while(matched >= i){
	k.right_[i] = rkn.key_;
	rkn.left_[i] = k.key_;
	++i;
      }
	
      ++it;
      if(it == tmpmap.end()){break;}
    }
    cnt++;
    fprintf(stderr,"\rorganize skip graph: %.2lf percent...",
	    static_cast<double>(cnt) * 100 / tmpmap.size());
  }
  fprintf(stderr,"\r                                        \r");
  
  key_map::iterator it = tmpmap.begin();
  for(;it != tmpmap.end(); ++it){
    it->second->update_key(it->first);
  }
  keys.swap(tmpmap);
}


void global_nodes::node_dump(int maxlevel)const{
  std::vector<std::string> lines;
  node_list::const_iterator it = nodes.begin();
  BOOST_FOREACH(const node& n, nodes){
    n.range_dump();
    std::cerr << " ";
    n.key_dump(maxlevel);
    std::cerr << std::endl;
  }
}

global_nodes::avg_var global_nodes::count_average_hop(int maxlevel)const{
  std::vector<key> allkey;
  { // create keylist
    typedef std::pair<key,const node*> kn;
    allkey.reserve(keys.size());
    BOOST_FOREACH(kn k, keys){
      allkey.push_back(k.first);
    }
  }
  std::vector<double> hops;
  //std::vector<int> counter;
  int cnt = 0;
  BOOST_FOREACH(const node& from, nodes){
    if(from.empty())continue;
    int sum=0;
    BOOST_FOREACH(const key& target, allkey){
      const node* fromnode = &from;
      int hop = 0;
      while(1){
	int next = fromnode->next_key(maxlevel,target.key_);
	if(next == node::key_found) break;
	
	hop++;
	fromnode = &which_node(next);
	  
      }
      sum+=hop;
    }
    hops.push_back(static_cast<double>(sum) / allkey.size());
    cnt++;
      
    fprintf(stderr,"\rhop accumurate: %.1lf percent....",
	    static_cast<double>(cnt) * 100 / nodes.size());
  }
  fprintf(stderr,"\r                                     \r");
  {
    const double avg = std::accumulate(hops.begin(),hops.end(),0.0) / hops.size();
    const double var =
      std::for_each(hops.begin(),hops.end(),calc_var(avg)).
      accum() / hops.size();
    return std::make_pair(avg,var);
  }
  
}

void global_nodes::dump(const size_t maxlevel)const{
  // graphical dump skipgraph
  int linemax = 1 << maxlevel;
  std::vector<std::string> lines(linemax, std::string());
  // 1=level0 / 2,3=level1 / 4,5,6,7=level2
  
  std::vector<int> rightlist(linemax, key::not_exist);
  for(int i=1;i<linemax; i++){
    char buff[16];
    sprintf(buff,"%d:",which_level(i)-1);
    lines[i].append(buff);
  }

  typedef std::pair<key, const node*> kn;
  BOOST_FOREACH(const kn& k, keys){
    std::vector<char> flag(maxlevel+1,0);      
    for(int i=1; i<linemax; i++){
      int level = which_level(i)-1;
      int width = get_width(k.first.key_);
      if(rightlist[i] == k.first.key_ ||
	 (rightlist[i] == key::not_exist && flag[level] == 0)){
	char buff[16];
	sprintf(buff,"[%d]",k.first.key_);
	lines[i] += buff;
	rightlist[i] = (k.first.right_[level] == key::not_exist)
	  ? key::invalid : k.first.right_[level];
	flag[level] = 1;
      }else if(rightlist[i] == key::not_exist){
	width+=1;
	for(;width>=0;--width){
	  lines[i].append(" ");
	}
      }else if(rightlist[i] == key::invalid){
	// nothing to do
      }else{
	width+=1;
	for(;width>=0;--width){
	  lines[i].append("-");
	}
      }
    }
  }
  for(int i=linemax-1;i>0;i--){
    if(line_is_empty(lines[i])) continue;
    std::cerr << lines[i] << std::endl;
  }
}

global_nodes::avg_var global_nodes::count_neighbor(int maxlevel)const{
  std::vector<int> counter;
  
  // for every nodes
  BOOST_FOREACH(const node& n, nodes){
    std::vector<int> unique_key;
    
    { // count all keys
      unique_key.reserve(maxlevel * n.keys_.size() * 2);
      BOOST_FOREACH(const key& k, n.keys_){
	for(int i=0;i<maxlevel; i++){
	  unique_key.push_back(k.left_[i]);
	  unique_key.push_back(k.right_[i]);
	}
      }
      {
	std::sort(unique_key.begin(),unique_key.end());
	unique_key.erase(std::unique(unique_key.begin(),unique_key.end())
			 ,unique_key.end());
	unique_key.erase(std::remove(unique_key.begin(), unique_key.end(),
				     key::invalid)
			 , unique_key.end());
	unique_key.erase(std::remove(unique_key.begin(),unique_key.end(),
				     key::not_exist)
			 , unique_key.end());
      }
    }
      
    std::vector<uint64_t> neighbors;
    { // count neighbors
      BOOST_FOREACH(const int& k, unique_key){
	const node& which = which_node(k);
	if(!(which == n))
	  neighbors.push_back(which.vector_range_begin_.vector);
      }
      {
	std::sort(neighbors.begin(),neighbors.end());
	std::vector<uint64_t>::iterator new_end =
	  std::unique(neighbors.begin(),neighbors.end());
	neighbors.erase(new_end,neighbors.end());
      }
    }
    counter.push_back(neighbors.size());
    //std::cout << "neighbor:" << neighbors.size() << std::endl;
  }
  double avg = std::accumulate(counter.begin(),counter.end(),0.0) / nodes.size();
  double var = std::for_each(counter.begin(),counter.end(),calc_var(avg))
    .accum() / nodes.size();
  return std::make_pair(avg,var);
}
