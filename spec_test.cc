
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
