#include <gtest/gtest.h>
#include "flush_reload.hpp"

class FlushReloadTest : public ::testing::Test {

	protected:
		virtual void SetUp(){
		
			ff = std::make_unique<FlushReload>(CTA_TEST_EXEC, CTA_TEST_OFFSET);
		}
		virtual void TearDown(){

		}

		std::unique_ptr<FlushReload> ff;
};

TEST_F(FlushReloadTest, hit_miss){

}
