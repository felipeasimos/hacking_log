#include <gtest/gtest.h>
#include "flush_flush.hpp"

class FlushFlushTest : public ::testing::Test {

	protected:
		virtual void SetUp(){
		
			ff = FlushFlush();
		}
		virtual void TearDown(){

		}

		FlushFlush ff;
};

TEST_F(FlushFlushTest, hit_miss){

}
