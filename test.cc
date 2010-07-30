
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <boost/program_options.hpp>
#include <boost/noncopyable.hpp>
#include <boost/random.hpp>
#include "asmsg.hpp"
#include <gtest/gtest.h>

TEST(construct,1){
  global_nodes world;
  EXPECT_EQ(world.nodes.size(), 0);
  EXPECT_EQ(world.keys.size(), 0);
}
TEST(set_nodes,1){
  boost::mt19937 rand(0);
  global_nodes world;
  world.set_nodes(3, rand);
  EXPECT_EQ(world.nodes.size(),3);
}

TEST(set_nodes,2){
  boost::mt19937 rand(0);
  global_nodes world;
  world.set_nodes(3, rand);
  world.put_key(key(3,rand64(rand)), 3);
  world.put_key(key(3,rand64(rand)), 5);
  EXPECT_EQ(5,world.nodes[0].keys_[0].right_[0]);
}
