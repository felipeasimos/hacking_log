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

		unsigned int number_of_probes = 1000000;
};

TEST_F(FlushCalibratorTest, calibration_flush_flush){

	hit_miss_map map = calibrator.calibrate(ff, number_of_probes);

	printf("flush_flush threshold: %u\n", ff.threshold);

	ASSERT_GT(ff.threshold, 130);
	ASSERT_LT(ff.threshold, 150);

	ASSERT_FALSE(ff.hit_is_faster);
};

TEST_F(FlushCalibratorTest, calibration_flush_reload){

	hit_miss_map map = calibrator.calibrate(fr, number_of_probes);

	printf("flush_reload threshold: %u\n", fr.threshold);

	ASSERT_GT(fr.threshold, 80);
	ASSERT_LT(fr.threshold, 100);

	ASSERT_TRUE(fr.hit_is_faster);
}

TEST_F(FlushCalibratorTest, visualizer){

	calibrator.histogram(ff, "ff.csv");
	calibrator.histogram(fr, "fr.csv");
}

TEST_F(FlushCalibratorTest, hit_is_faster){

	calibrator.calibrate(ff, number_of_probes);
	calibrator.calibrate(fr, number_of_probes);

	ASSERT_FALSE(ff.hit_is_faster);
	ASSERT_TRUE(fr.hit_is_faster);
}
