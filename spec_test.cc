
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <boost/program_options.hpp>
#include <boost/noncopyable.hpp>
#include <boost/random.hpp>
#include "asmsg.hpp"
#include <gtest/gtest.h>


TEST(nodes,six){
  global_nodes g(0);
  g.set_nodes(6);
  
  EXPECT_EQ(g.nodes[0].vector_range_begin_.vector,0);
  EXPECT_EQ(g.nodes[1].vector_range_begin_.vector,7815051614634499795ULL);
  EXPECT_EQ(g.nodes[2].vector_range_begin_.vector,10051320526890433895ULL);
  EXPECT_EQ(g.nodes[3].vector_range_begin_.vector,10123822223749065263ULL);
  EXPECT_EQ(g.nodes[4].vector_range_begin_.vector,11119021634053362427ULL);
  EXPECT_EQ(g.nodes[5].vector_range_begin_.vector,13192915183495924928ULL);
}

TEST(keys,N){
  enum  {N = 30};
  global_nodes g(0);
  g.set_nodes(1);
  for(int i=0; i<N; i++){
    g.put_key(key(i,0),0);
  }
  g.organize_skipgraph(8);
  for(int i=0; i<N; i++){
    EXPECT_EQ(g.nodes[0].keys_[i].key_, i);
  }
}


TEST(spec,conns){
  enum  {N = 500};
  boost::mt19937 rands(0);
  global_nodes g(0);
  g.set_nodes(100);
  for(int i=0; i<N; i++){
    g.put_key(key(i,rand64(rands)),rand64(rands));
  }
  g.organize_skipgraph(8);
  const std::pair<double,double> r = g.count_neighbor(8);
  
  EXPECT_EQ(23, r.first);
  EXPECT_EQ(282.74000000000001, r.second);
}

TEST(spec,conns2){
  enum  {N = 50};
  boost::mt19937 rands(0);
  global_nodes g(0);
  g.set_nodes(10);
  for(int i=0; i<N; i++){
    g.put_key(key(i,rand64(rands)),rand64(rands));
  }
  g.organize_skipgraph(8);
  const std::pair<double,double> r = g.count_neighbor(8);
  
  EXPECT_EQ(6.4000000000000004, r.first);
  EXPECT_EQ(3.4399999999999999, r.second);
}

TEST(spec,conn_ten){
  double expect_avg[] = {2.3999999999999999,3.2000000000000002
			 ,3.6000000000000001,4,4,4,4,
			 4.2000000000000002,4.2000000000000002,
			 4.5999999999999996};
  double expect_var[] = {4.04,4.7599999999999998,4.4400000000000004,
			 5.2000000000000002,5.2000000000000002,
			 5.2000000000000002,5.2000000000000002,
			 5.5599999999999996,5.5599999999999996,
			 5.04};
  for(int n=10;n<20;n++){
    boost::mt19937 rands(0);
    global_nodes g(0);
    g.set_nodes(10);
    for(int i=0; i<n; i++){
      g.put_key(key(i,rand64(rands)),rand64(rands));
    }
    g.organize_skipgraph(8);
    const std::pair<double,double> r = g.count_neighbor(8);
  
    EXPECT_EQ(expect_avg[n-10], r.first);
    EXPECT_EQ(expect_var[n-10], r.second);
  }
}
TEST(spec,hop_ten){
  double expect_avg[] = {1.2333333333333334,1.2857142857142858,1.4375
			 ,1.4711538461538463,1.4553571428571428
			 ,1.4916666666666667,1.4765625
			 ,1.4852941176470589,1.4930555555555556
			 ,1.5906432748538011};
  double expect_var[] = {0.068888888888888888,0.069488952605835722,0.10026041666666667,
			 0.1012389053254438,0.10578762755102043,
			 0.10215277777777779,0.11126708984375,
			 0.10964532871972318,0.12688078703703703,
			 0.14944769330734242};
  for(int n=10;n<20;n++){
    boost::mt19937 rands(0);
    global_nodes g(0);
    g.set_nodes(10);
    for(int i=0; i<n; i++){
      g.put_key(key(i,rand64(rands)),rand64(rands));
    }
    g.organize_skipgraph(8);
    const std::pair<double,double> r = g.count_average_hop(8);
  
    EXPECT_EQ(expect_avg[n-10], r.first);
    EXPECT_EQ(expect_var[n-10], r.second);
  }
}
