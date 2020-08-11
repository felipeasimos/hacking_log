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
