#include <gtest/gtest.h>

#include "flush_calibrator.hpp"
#include "../flush_flush/flush_flush.hpp"
#include "../flush_reload/flush_reload.hpp"

class FlushCalibratorTest : public ::testing::Test {

	protected:
		virtual void SetUp(){

			ff = std::make_unique<FlushFlush>(CTA_TEST_EXEC, CTA_TEST_OFFSET);
			calibrator = FlushCalibrator();
			fr = std::make_unique<FlushReload>(CTA_TEST_EXEC, CTA_TEST_OFFSET);
		}

		virtual void TearDown(){
		
		}

		std::unique_ptr<FlushFlush> ff;
		std::unique_ptr<FlushReload> fr;
		FlushCalibrator calibrator;

};

TEST_F(FlushCalibratorTest, calibration_flush_flush){

	calibrator.calibrate(*ff, "ff.csv", 100);

	printf("flush_flush hit_begin: %u\n", ff->hit_begin);
	printf("flush_flush hit_end: %u\n", ff->hit_end);
};

TEST_F(FlushCalibratorTest, calibration_flush_reload){

	calibrator.calibrate(*fr, "fr.csv", 100);

	printf("flush_reload hit_begin: %u\n", fr->hit_begin);
	printf("flush_reload hit_end: %u\n", fr->hit_end);
}
