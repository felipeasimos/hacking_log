CXX:=g++ -std=c++17
object_files:=$(addsuffix .o, $(basename $(filter-out runner.cpp,$(wildcard *.cpp))))

DEPS:=cache_info/cache_info_final.o ../flush_flush/flush_flush_final.o ../flush_reload/flush_reload_final.o \
	../cache_timing_attack/cache_timing_attack_final.o ../visualizer/visualizer_final.o

UNIQUE_DEPS:=cache_info/cache_info_final.o

LD_FLAGS:=-lgtest_main -lgtest -lpthread

.PHONY: test basic_clean clean done runner $(DEPS)

test: CXXFLAGS := -fPIC -g -O0
test: runner
	$(MAKE) basic_clean
	./runner

done: CXXFLAGS := -O3 -fPIC
done: flush_calibrator_final.o
	$(MAKE) basic_clean

dummy_victim: dummy_victim.c
	$(CXX) $(CXXFLAGS) $^ -o $@

flush_calibrator_final.o: $(object_files) $(UNIQUE_DEPS)
	ld -r $^ -o $@

clean:
	rm -f *.o

basic_clean:
	rm -f $(object_files) runner.o

runner: $(object_files) $(DEPS) runner.o 
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LD_FLAGS)

$(DEPS): %: %
	$(MAKE) done -C $(dir $@)

$(object_files): %.o: %.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

runner.o: runner.cpp
	$(CXX) $(CXXFLAGS) $^ -c -o $@
