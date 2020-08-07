#include <gtest/gtest.h>
#include "flush_flush.hpp"
#include "../cache_timing_attack/cache_timing_attack.hpp"

class FlushFlushTest : public ::testing::Test {

	protected:
		virtual void SetUp(){
		
			ff = std::make_unique<FlushFlush>(CTA_TEST_EXEC, CTA_TEST_OFFSET);
		}
		virtual void TearDown(){

		}

		std::unique_ptr<FlushFlush> ff;
};

TEST_F(FlushFlushTest, hit_miss){

}
