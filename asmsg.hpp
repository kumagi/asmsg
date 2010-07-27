#include <stdint.h>

#include <vector>
#include <map>
#include <stdio.h>
#include <boost/noncopyable.hpp>
#include <boost/random.hpp>
#include <boost/shared_ptr.hpp>


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
  membership_vector(uint64_t v):vector(v){}
  membership_vector():vector(0){}
  membership_vector(const membership_vector& org):vector(org.vector){}

  int match(const membership_vector& o)const{
    const uint64_t matched = ~(vector ^ o.vector);
    uint64_t bit = 1;
    int cnt = 0;
    while((matched & bit) && cnt < 64){
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
  explicit key(const int& k,const membership_vector& m)
    :key_(k),mv(m){}
  bool operator<(const key& rhs)const{
    return key_ < rhs.key_;
  }
  bool operator==(const key& rhs)const{
    return key_ == rhs.key_;
  }
};

struct node{
  std::vector<key> keys_;
  membership_vector vector_range_begin_;
  membership_vector vector_range_end_;
  explicit node(const membership_vector& r):vector_range_begin_(r){}
  bool operator<(const node& rhs)const{
    return vector_range_begin_ < rhs.vector_range_begin_;
  }
  bool operator==(const node& rhs)const{
    return vector_range_begin_ == rhs.vector_range_begin_;
  }
  key& get_key(const key& k){
    std::vector<key>::iterator it = 
      std::find(keys_.begin(),keys_.end(),k);
    assert(it != keys_.end());
    return *it;
  }
};

struct global_nodes{
  std::vector<node> nodes;
  std::map<key,node> keys;
  void set_nodes(size_t targets, boost::mt19937& rnd){
    while(nodes.size() < targets){
      const int targetsize = targets - nodes.size();
      for(int i=0; i<targetsize ;i++){
	nodes.push_back(node(rnd()));
      }
      sort(nodes.begin(),nodes.end());
      unique(nodes.begin(),nodes.end());
    }
    assert(nodes.size() == targets);
    // assign range
    for(size_t i=0; i<nodes.size()-1; i++){
      nodes[i].vector_range_end_ = nodes[i+1].vector_range_begin_;
    }
    nodes[nodes.size()-1].vector_range_end_ = 0;
  }
  void put_key(const key& newkey, int max_level){
    membership_vector tmpvec = newkey.mv;
    node tmpnode(tmpvec);
    std::vector<node>::iterator it =
      lower_bound(nodes.begin(),nodes.end(),tmpnode);
    it->keys_.push_back(newkey);
    
    // left hand side
    std::map<key,node>::iterator left =
      keys.lower_bound(newkey);
    int targetlevel = 0;
    while(left != keys.end() && targetlevel < max_level){
      key& targetkey = left->second.get_key(left->first);
      int matches = newkey.mv.match(targetkey.mv);
      std::cout << matches << std::endl;
      while(targetlevel < matches){
	targetkey.right_[targetlevel] = newkey.key_;
	//	targetlevel++;
      }
    }
    keys.insert(std::make_pair(newkey,*it));
  }
  void dump(size_t maxlevel)const{
    std::vector<std::string> lines; // 1=level0 / 2,3=level1 / 4,5,6,7=level2
    lines.reserve(1 << maxlevel);
    std::map<key,node>::const_iterator it
      = keys.begin();
    while(it != keys.end()){
      for(size_t i=0; i<lines.size(); i++){
	lines[i] += "===";
      }
    }
    
    for(int i=lines.size()-1; i>=0; i--){
      std::cout << lines[i] << std::endl;
    }
  }
  
  
};

template<typename rnd>
uint64_t rand64(rnd& random){
  return (static_cast<uint64_t>(random()) << 32) ||
    static_cast<uint64_t>(random());
}
