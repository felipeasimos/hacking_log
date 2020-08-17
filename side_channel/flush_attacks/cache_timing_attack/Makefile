CXX:=g++ -std=c++17
object_files:=$(addsuffix .o, $(basename $(filter-out runner.cpp,$(wildcard *.cpp))))

UNIQUE_DEPS=cache_info/cache_info_final.o

LD_FLAGS:=-lgtest_main -lgtest -pthread

.PHONY: test basic_clean clean done runner $(DEPS)

test: CXXFLAGS := -fPIC -g -O0
test: runner
	$(MAKE) basic_clean
	./runner

done: CXXFLAGS := -O1 -fPIC
done: cache_timing_attack_final.o
	$(MAKE) basic_clean

cache_timing_attack_final.o: $(object_files) $(UNIQUE_DEPS)
	ld -r $^ -o $@

clean:
	rm -f *.o

basic_clean:
	rm -f $(object_files) runner.o

runner: $(object_files) runner.o $(UNIQUE_DEPS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LD_FLAGS) 

$(UNIQUE_DEPS): %: %
	$(MAKE) done -C $(dir $@)

$(object_files): %.o: %.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

runner.o: runner.cpp
	$(CXX) $(CXXFLAGS) $^ -c -o $@
