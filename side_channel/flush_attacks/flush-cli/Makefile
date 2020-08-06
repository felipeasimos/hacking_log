CXX:=g++ -std=c++17
object_files:=$(addsuffix .o, $(basename $(filter-out runner.cpp,$(wildcard *.cpp))))

DEPS:=../flush_flush/flush_flush_final.o ../flush_reload/flush_reload_final.o \
../cache_timing_attack/cache_timing_attack_final.o ../visualizer/visualizer_final.o ../flush_calibrator/flush_calibrator_final.o

LD_FLAGS:=-lgtest_main -lgtest -lpthread

.PHONY: test basic_clean clean done runner plot $(DEPS)

all: CXXFLAGS := -O3 -fPIC
all: flush-cli
	$(MAKE) clean

clean:
	rm -f *.o

flush-cli: $(object_files) $(DEPS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LD_FLAGS)

$(DEPS): %: %
	$(MAKE) done -C $(dir $@)

$(object_files): %.o: %.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@
