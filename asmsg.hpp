#include <stdint.h>
#include <algorithm>
#include <vector>
#include <map>
#include <stdio.h>
#include <boost/noncopyable.hpp>
#include <boost/random.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/utility.hpp>
#include <boost/timer.hpp>


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
  size_t level;
  bool graph;
  bool nodedump;
  int seed;
  config():graph(false),nodedump(false){};
  void dump()const{
    std::cout << "nodes:" << nodes << std::endl
	      << "keys:" << keys << std::endl
	      << "level:" << level << std::endl
	      << "seed:" << seed << std::endl;
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
  enum status{
    not_exist = -8,
    invalid = -14,
  };
  membership_vector mv;
  key(){
    left_.reserve(64);
    left_.reserve(64);
  };
  explicit key(const int k):key_(k){}
  key(const int& k,const membership_vector& m)
    :key_(k),left_(64,not_exist),right_(64,not_exist),mv(m){
    left_.reserve(64);
    left_.reserve(64);
  }
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
  enum status{
    key_found = -5,
  };
  typedef std::vector<key> key_list;
  key_list keys_;
  membership_vector vector_range_begin_;
  membership_vector vector_range_end_;
  explicit node(const membership_vector& r):vector_range_begin_(r){}
  explicit node(const membership_vector& b,const membership_vector& e)
    :vector_range_begin_(b),vector_range_end_(e){}
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
  bool empty()const{
    return keys_.empty();
  }
  void add_key(const key& k){
    keys_.push_back(k);
    sort(keys_.begin(),keys_.end());
  }
  void update_key(const key& k){
    key_list::iterator targetkey 
      = std::lower_bound(keys_.begin(),keys_.end(),k);
    assert(targetkey != keys_.end());
    *targetkey = k;
  }
  int next_key(int level, int target)const{
    assert(!keys_.empty());
    key_list::const_iterator lower = 
      std::lower_bound(keys_.begin(),keys_.end(),key(target));
    if(lower == keys_.end()){lower = keys_.begin();}

    if(*lower == key(target)) return key_found;
    
    key_list::const_iterator upper = 
      std::upper_bound(keys_.begin(),keys_.end(),key(target));
    if(upper == keys_.end()){upper = --keys_.end();}
    
    // search nearest key
    const key* from_key;
    if(std::abs(lower->key_ - target) <= std::abs(upper->key_ - target)){
      from_key = &*lower;
    }else{
      from_key = &*upper;
    }
    
    // search proper  level
    if(from_key->key_ < target){
      while(level > 0 &&
	    (target < from_key->right_[level] ||
	     from_key->right_[level] == key::not_exist)) level--;
      assert(level != -1);
      assert(from_key->right_[level] != key::not_exist);
      return from_key->right_[level];
    }else{
      while(level > 0 &&
	    (from_key->left_[level] < target)) level--;
      assert(level != -1);
      assert(from_key->left_[level] != key::not_exist);
      return from_key->left_[level];
    }
  }
  void key_dump(int maxlevel)const{
    BOOST_FOREACH(const key& k, keys_){
      k.dump_lr(maxlevel);
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
  typedef std::map<key,node* const> key_map;
  node_list nodes;
  key_map keys;
  boost::mt19937 rand;
  global_nodes(int seed = 0):rand(seed){}
  void set_nodes(size_t targets);
  void put_key(const key& newkey, uint64_t putnode);
  void refresh_keymap(int maxlevel);
  void key_dump(size_t maxlevel)const;
  void node_dump(int maxlevel)const;

  void dump(const size_t maxlevel)const;
  void count_neighbor(int maxlevel)const; 
  
  void count_average_hop(int maxlevel)const{
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
       	//std::cout <<  "=> " << target.key_ << " : ";
	while(1){
	  int next = fromnode->next_key(maxlevel,target.key_);
	  if(next == node::key_found) break;
	  
	  //std::cout << " -> " << next;
	  hop++;
	  fromnode = &which_node(next);
	  
	}
 	sum+=hop;
      }
      hops.push_back(static_cast<double>(sum) / allkey.size());
      cnt++;
      
      fprintf(stderr,"\rhop accumurate: %.1lf pct.",
      	      static_cast<double>(cnt) * 100 / nodes.size());
    }
    fprintf(stderr,"\r                              \r");
    {
      double sum=0;
      BOOST_FOREACH(const double& h, hops){sum += h;}
      std::cerr << "average hops:" << std::hex << sum/nodes.size() << std::endl;
    }
  }
private:
  const node& which_node(const int& k)const {
    assert(keys.find(key(k,0)) != keys.end());
    return *(keys.find(key(k,0))->second);
  }
  static char which_level(int i){
    int cnt=0;
    while(i){i>>=1;cnt++;}
    return cnt;
  }
  static int get_width(int i){
    int cnt=1;i/=10;
    while(i){i/=10;cnt++;}
    return cnt;
  }
  static bool line_is_empty(const std::string& l){
    std::string::const_iterator it = std::find(l.begin(),l.end(),':');
    ++it;
    while(it != l.end()){
      if(*it != ' '){ return false;}
      ++it;
    }
    return true;
  }
private: // forbid method
  global_nodes();
  global_nodes(const global_nodes&);
};
