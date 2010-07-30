#include <stdint.h>

#include <vector>
#include <map>
#include <stdio.h>
#include <boost/noncopyable.hpp>
#include <boost/random.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/utility.hpp>



template<typename rnd>
uint64_t rand64(rnd& random){
  return (static_cast<uint64_t>(random()) << 32) |
    static_cast<uint64_t>(random());
}


template<typename Derived>
class singleton : private boost::noncopyable {
public:
  static Derived& instance() { static Derived the_inst; return the_inst; }
protected:
  singleton(){}
  ~singleton(){}
};

struct config : public singleton<config>{
  size_t nodes;
  size_t keys;
  size_t vector_len; // = max level
  boost::mt19937 rand;
  config():rand(0){};
  void dump()const{
    std::cout << "nodes:" << nodes << std::endl
	      << "keys:" << keys << std::endl
	      << "vector_len:" << vector_len << std::endl;
  }
};

struct membership_vector{
  uint64_t vector;
  enum length{
    vectormax = 64,
  };
  membership_vector(uint64_t v):vector(v){} // not explicit!
  membership_vector():vector(0){}
  membership_vector(const membership_vector& org):vector(org.vector){}

  int match(const membership_vector& o)const{
    const uint64_t matched = ~(vector ^ o.vector);
    uint64_t bit = 1;
    int cnt = 0;
    while((matched & bit) && cnt < vectormax){
      bit *= 2;
      cnt++;
    }
    return cnt;
  }
  void dump()const{
    const char* bits = reinterpret_cast<const char*>(&vector);
    for(int i=7;i>=0;--i){
      fprintf(stderr,"%02x",(unsigned char)255&bits[i]);
    }
  }
  bool operator==(const membership_vector& rhs)const{
    return vector == rhs.vector;
  }
  bool operator<(const membership_vector& rhs)const{
    return vector < rhs.vector;
  }
};

struct key{
  int key_;
  std::vector<int> left_;
  std::vector<int> right_;
  membership_vector mv;
  key(){
    left_.reserve(64);
    left_.reserve(64);
  };
  explicit key(const int& k,const membership_vector& m)
    :key_(k),left_(64,-1),right_(64,-1),mv(m){}
  key(const key& o)
    :key_(o.key_),left_(o.left_),right_(o.right_),mv(o.mv){
    left_.reserve(64);
    left_.reserve(64);
  }
  bool operator<(const key& rhs)const{
    return key_ < rhs.key_;
  }
  bool operator==(const key& rhs)const{
    return key_ == rhs.key_;
  }
  void dump()const{
    fprintf(stderr,"[key:%d with mv:",key_);
    mv.dump();
    fprintf(stderr,"]");
  }
  void dump_lr(int level)const{
    assert(level > 0);
    fprintf(stderr,"[");
    for(int i=level-1;i>=0;i--){
      fprintf(stderr," l%d:%d ",i,left_[i]);
    }
    fprintf(stderr,"[%d]",key_);
    for(int i=0;i<level;i++){
      fprintf(stderr," r%d:%d ",i,right_[i]);
    }
    fprintf(stderr,"]");
  }
};

struct node{
  typedef std::vector<key> key_list;
  key_list keys_;
  membership_vector vector_range_begin_;
  membership_vector vector_range_end_;
  explicit node(const membership_vector& r):vector_range_begin_(r){}
  bool operator<(const node& rhs)const{
    return vector_range_begin_ < rhs.vector_range_begin_;
  }
  bool operator==(const node& rhs)const{
    return vector_range_begin_ == rhs.vector_range_begin_;
  }
  const key& get_key(const key& k){
    //std::find(keys_.begin(),keys_.end(),k);
    for(size_t i =0;i<keys_.size(); i++){
      if(keys_[i] == k){return keys_[i];}
    }
    assert(false);
  }
  key* add_key(const key& k){
    keys_.push_back(k);
    sort(keys_.begin(),keys_.end());
  }
  void key_dump()const{
    BOOST_FOREACH(const key& k, keys_){
      k.dump();
    }
  }
  void range_dump()const{
    vector_range_begin_.dump();
    std::cerr << "----";
    vector_range_end_.dump();
  }
};

struct global_nodes{
  typedef std::vector<node> node_list;
  typedef std::map<key,const node*> key_map;
  node_list nodes;
  key_map keys;
  boost::mt19937 rand;
  global_nodes(int seed):rand(seed){}
  void set_nodes(size_t targets){
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
  void put_key(const key& newkey, uint64_t putnode){
    node_list::iterator target_node =
      lower_bound(nodes.begin(),nodes.end(),node(putnode));
    if(target_node != nodes.begin() && target_node != nodes.end()){target_node--;}
    if(target_node == nodes.end()){target_node = --nodes.end();}
    target_node->add_key(newkey);
    refresh_keymap();
  }
  void refresh_keymap(){
    key_map tmpmap;
    BOOST_FOREACH(const node& n, nodes){
      BOOST_FOREACH(const key& k, n.keys_){
	tmpmap.insert(std::make_pair(k,&n));
      }
    }
    
    for(key_map::iterator kn = tmpmap.begin(); kn != tmpmap.end(); ++kn){
      key& k =  const_cast<key&>(kn->first);
      key_map::iterator it = boost::next(kn);
      if(it == tmpmap.end()) break;
      for(int i=0; i<64;){
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
    keys = tmpmap;
  }
  void dumpkeys(size_t maxlevel)const{
    typedef std::pair<key, const node*> kn;
    BOOST_FOREACH(const kn& knp, keys){
      knp.first.dump_lr(maxlevel);
      std::cerr << std::endl;
    }
  }
  void node_dump()const{
    std::vector<std::string> lines;
    node_list::const_iterator it = nodes.begin();
    BOOST_FOREACH(const node& n, nodes){
      n.range_dump();
      std::cerr << " ";
      n.key_dump();
      std::cerr << std::endl;
    }
  }
  void dump(const size_t maxlevel)const{
    std::vector<std::string> lines; // 1=level0 / 2,3=level1 / 4,5,6,7=level2
    int linemax = 1 << maxlevel;
    lines.reserve(linemax);
    std::vector<int> rightlist(linemax,-1);
    
    typedef std::pair<key, const node*> kn;
    BOOST_FOREACH(const kn& k, keys){
      for(int i=0; i<linemax; i++){
	if(rightlist[i] == k.first.key_){
	  std::string buf;
	  buf.reserve(16);
	  sprintf(&buf[0],"[%d]",k.first.key_);
	  lines[i].append(buf);
	}else if(rightlist[i] == -1){
	  lines[i].append("   ");
	}else{
	  lines[i].append("===");
	}
      }
    }
    BOOST_FOREACH(const std::string& s, lines){
      std::cerr << s << std::endl;
    }
  }
  
  
};
