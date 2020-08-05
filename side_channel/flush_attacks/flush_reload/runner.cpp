#include <gtest/gtest.h>
#include "flush_reload.hpp"

class FlushReloadTest : public ::testing::Test {

	protected:
		virtual void SetUp(){
		
			ff = FlushReload();
		}
		virtual void TearDown(){

		}

		FlushReload ff;
};

TEST_F(FlushReloadTest, hit_miss){

}
