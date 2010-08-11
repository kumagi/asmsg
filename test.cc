
#include <stdlib.h>
#include <assert.h>
#include <algorithm>
#include <boost/program_options.hpp>
#include <boost/noncopyable.hpp>
#include <boost/random.hpp>
#include "asmsg.hpp"
#include <gtest/gtest.h>

TEST(construct,do_nothing){
  global_nodes world;
  EXPECT_EQ(world.nodes.size(), 0);
  EXPECT_EQ(world.keys.size(), 0);
}
TEST(set_nodes,1){
  for(int i=1;i<512;i++){
    global_nodes world;
    world.set_nodes(i);
    EXPECT_EQ(world.nodes.size(),i);
  }
}

TEST(put_key,1){
  boost::mt19937 rand(0);
  global_nodes world;
  world.set_nodes(2);
  world.put_key(key(3,rand64(rand)), 0);
  world.organize_skipgraph(1);
  EXPECT_EQ(key::not_exist,world.nodes[0].keys_[0].right_[0]);
}

TEST(put_key,many){
  for(int i=1;i<32;i++){
    global_nodes world;
    world.set_nodes(1);
    for(int j=0;j<i;j++){
      world.put_key(key(j,0), 0);
    }
    world.organize_skipgraph(1);
    EXPECT_EQ(i,world.keys.size());
  }
}

TEST(organize_skipgraph,two_keys){
  global_nodes world;
  world.set_nodes(1);
  world.put_key(key(3,0), 0);
  world.put_key(key(4,0), 0);
  world.organize_skipgraph(64);
  for(int i=0;i<64;i++){
    EXPECT_EQ(4,world.nodes[0].keys_[0].right_[i]);
    EXPECT_EQ(3,world.nodes[0].keys_[1].left_[i]);
  }
}

TEST(organize_skipgraph,three_keys){
  global_nodes world;
  world.set_nodes(1);
  world.put_key(key(3,0), 0); // same membership vectors
  world.put_key(key(4,0), 0);
  world.put_key(key(5,0), 0);
  world.organize_skipgraph(64);
  for(int i=0;i<64;i++){// check key4's left and right
    EXPECT_EQ(3,world.nodes[0].keys_[1].left_[i]);
    EXPECT_EQ(5,world.nodes[0].keys_[1].right_[i]);
  }
}


TEST(organize_skipgraph,three_keys_membership_vector){
  global_nodes world(0);
  world.set_nodes(1);
  world.put_key(key(3,1), 0); // 3  3  3  3  3
  world.put_key(key(4,3), 0); // 4  4  |4 |4 |4 ;1bit match
  world.put_key(key(5,1), 0); // 5  5  5  5  5
  world.organize_skipgraph(5);
  for(int i=0;i<2;i++){
    // check key3's right
    EXPECT_EQ(key::not_exist,world.nodes[0].keys_[0].left_[i]);
    EXPECT_EQ(4,world.nodes[0].keys_[0].right_[i]);
    // check key4's left and right
    EXPECT_EQ(3,world.nodes[0].keys_[1].left_[i]);
    EXPECT_EQ(5,world.nodes[0].keys_[1].right_[i]);
    // check key5's left
    EXPECT_EQ(4,world.nodes[0].keys_[2].left_[i]);
    EXPECT_EQ(key::not_exist,world.nodes[0].keys_[2].right_[i]);
  }
  for(int i=2;i<4;i++){
    // check key3's right
    EXPECT_EQ(key::not_exist,world.nodes[0].keys_[0].left_[i]);
    EXPECT_EQ(5,world.nodes[0].keys_[0].right_[i]);
    // check key4's left and right
    EXPECT_EQ(key::not_exist,world.nodes[0].keys_[1].left_[i]);
    EXPECT_EQ(key::not_exist,world.nodes[0].keys_[1].right_[i]);
    // check key5's left
    EXPECT_EQ(3,world.nodes[0].keys_[2].left_[i]);
    EXPECT_EQ(key::not_exist,world.nodes[0].keys_[2].right_[i]);
  }
}

