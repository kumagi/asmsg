#include "asmsg.hpp"
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
  while(nodes.size() < targets){
    const int targetsize = targets - nodes.size();
    for(int i=0; i<targetsize ;i++){
      nodes.push_back(node(rand64(rand)));
    }
    sort(nodes.begin(),nodes.end());
    unique(nodes.begin(),nodes.end());
  }
  assert(nodes.size() == targets);
  // assign range
  for(size_t i=0; i<nodes.size()-1; i++){
    nodes[i].vector_range_end_ = nodes[i+1].vector_range_begin_;
  }
  nodes.rbegin()->vector_range_end_ = 0;
}

void global_nodes::refresh_keymap(int maxlevel){
  // create key-node pair list
  key_map tmpmap;
  BOOST_FOREACH(node& n, nodes){
    BOOST_FOREACH(const key& k, n.keys_){
      tmpmap.insert(std::make_pair(k,&n));
    }
  }
    
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
  }
    
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

void global_nodes::dump(const size_t maxlevel)const{
  int linemax = 1 << maxlevel;
  std::vector<std::string> lines(linemax, std::string());
  // 1=level0 / 2,3=level1 / 4,5,6,7=level2
  enum const_key{notexist = -2, endlist = -3};
    
  std::vector<int> rightlist(linemax, notexist);
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
	 (rightlist[i] == notexist && flag[level] == 0)){
	char buff[16];
	sprintf(buff,"[%d]",k.first.key_);
	lines[i] += buff;
	rightlist[i] = (k.first.right_[level] == -1)
	  ? endlist : k.first.right_[level];
	flag[level] = 1;
      }else if(rightlist[i] == notexist){
	width+=1;
	for(;width>=0;--width){
	  lines[i].append(" ");
	}
      }else if(rightlist[i] == endlist){
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

void global_nodes::count_neighbor(int maxlevel)const{
  std::vector<int> counter;
  BOOST_FOREACH(const node& n, nodes){
    std::vector<int> unique_key;
    BOOST_FOREACH(const key& k, n.keys_){
      for(int i=0;i<maxlevel; i++){
	unique_key.push_back(k.left_[i]);
      }
      for(int i=0;i<maxlevel; i++){
	unique_key.push_back(k.right_[i]);
      }
    }
    {
      std::sort(unique_key.begin(),unique_key.end());
      std::vector<int>::iterator new_end =
	std::unique(unique_key.begin(),unique_key.end());
      unique_key.erase(new_end,unique_key.end());
      unique_key.erase(std::remove(unique_key.begin(), unique_key.end(),-1)
		       , unique_key.end());
    }
      
    std::vector<uint64_t> neighbors;
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
    counter.push_back(neighbors.size());
    //std::cout << "neighbor:" << neighbors.size() << std::endl;
  }
  double sum=0;
  BOOST_FOREACH(const int& c, counter){
    sum+=c;
  }
  double avg = sum/nodes.size();
  double v=0;
  BOOST_FOREACH(const int& c, counter){
    v += (c-avg)*(c-avg);
  }
  double var=v/nodes.size();
  std::cout << "avg:" << avg << std::endl;
  std::cout << "var:" << var << std::endl;
}