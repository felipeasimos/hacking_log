#include <gtest/gtest.h>
#include "cache_timing_attack.hpp"
#include <unistd.h>

using CTA = CacheTimingAttack;

class CacheTimingAttackTest : public ::testing::Test {

	protected:
		virtual void SetUp(){

			attack = std::make_unique<CTA>(CTA_TEST_EXEC, CTA_TEST_OFFSET);
			data = new int;
		}

		virtual void TearDown(){

			delete data;
		}

		std::unique_ptr<CTA> attack;
		int* data;
};

TEST_F(CacheTimingAttackTest, basic_time){

	unsigned int begin = attack->time();

	ASSERT_GT(attack->time()-begin, 0);
}

TEST_F(CacheTimingAttackTest, basic_flush){

	attack->flush();
}

TEST_F(CacheTimingAttackTest, flush_time_test){

	*data=0; //put data in cache

	//time how long it takes to access it
	unsigned int begin = attack->time();
	*data=1;
	unsigned int hit = attack->time()-begin;

	//time how long it takes to access it after flush
	attack->flush();
	begin = attack->time();
	*data=1;
	unsigned int miss = attack->time()-begin;

	//miss time should be greater than hit time
	ASSERT_GT(miss, hit);
}
