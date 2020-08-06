#include <gtest/gtest.h>

#include "flush_calibrator.hpp"
#include "../flush_flush/flush_flush.hpp"
#include "../flush_reload/flush_reload.hpp"

class FlushCalibratorTest : public ::testing::Test {

	protected:
		virtual void SetUp(){

			ff = FlushFlush();
			calibrator = FlushCalibrator();
			fr = FlushReload();
		}

		virtual void TearDown(){
		
		}

		FlushFlush ff;
		FlushReload fr;
		FlushCalibrator calibrator;

};

TEST_F(FlushCalibratorTest, calibration_flush_flush){

	calibrator.histogram(ff, "ff.csv");

	printf("flush_flush hit_begin: %u\n", ff.hit_begin);
	printf("flush_flush hit_end: %u\n", ff.hit_end);
};

TEST_F(FlushCalibratorTest, calibration_flush_reload){

	calibrator.histogram(fr, "fr.csv");

	printf("flush_reload hit_begin: %u\n", fr.hit_begin);
	printf("flush_reload hit_end: %u\n", fr.hit_end);
}
