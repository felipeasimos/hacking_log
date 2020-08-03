#include <gtest/gtest.h>
#include "cache_info.hpp"

class CacheInfoTest : public ::testing::Test {

	protected:
		virtual void SetUp(){

			cache1 = CacheInfo();
		}
		virtual void TearDown(){
		
		}

	CacheInfo cache1;
};

TEST_F(CacheInfoTest, cache_details){

	ASSERT_EQ(cache1.block_size(), 64);
	ASSERT_EQ(cache1.num_sets(), 4096);
	ASSERT_EQ(cache1.num_ways(), 16);
}
