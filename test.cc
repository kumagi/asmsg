
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <boost/program_options.hpp>
#include <boost/noncopyable.hpp>
#include <boost/random.hpp>
#include "asmsg.hpp"
#include <gtest/gtest.h>

TEST(construct,do_nothing){
  global_nodes world(0);
  EXPECT_EQ(world.nodes.size(), 0);
  EXPECT_EQ(world.keys.size(), 0);
}
TEST(set_nodes,1){
  for(int i=1;i<512;i++){
    global_nodes world(0);
    world.set_nodes(i);
    EXPECT_EQ(world.nodes.size(),i);
  }
}

TEST(put_key,1){
  boost::mt19937 rand(0);
  global_nodes world(0);
  world.set_nodes(2);
  world.put_key(key(3,rand64(rand)), 0);
  world.refresh_keymap(1);
  EXPECT_EQ(key::not_exist,world.nodes[0].keys_[0].right_[0]);
}
TEST(put_key,many){
  for(int i=1;i<64;i++){
    global_nodes world(0);
    world.set_nodes(1);
    for(int j=0;j<i;j++){
      world.put_key(key(j,0), 0);
    }
    world.refresh_keymap(1);
    EXPECT_EQ(i,world.keys.size());
  }
}
TEST(refresh_keymap,2){
  global_nodes world(0);
  world.set_nodes(1);
  world.put_key(key(3,0), 0);
  world.put_key(key(4,0), 0);
  world.refresh_keymap(1);
  EXPECT_EQ(4,world.nodes[0].keys_[0].right_[0]);
  EXPECT_EQ(3,world.nodes[0].keys_[1].left_[0]);
}
