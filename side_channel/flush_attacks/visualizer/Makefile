CXX:=g++ -std=c++17
object_files:=$(addsuffix .o, $(basename $(filter-out runner.cpp,$(wildcard *.cpp))))

LD_FLAGS:=-lgtest_main -lgtest -lpthread

.PHONY: test basic_clean clean done runner

test: CXXFLAGS := -fPIC -g -O0
test: runner
	$(MAKE) basic_clean
	./runner

done: CXXFLAGS := -O2 -fPIC
done: visualizer_final.o

visualizer_final.o: $(object_files)
	ld -r $^ -o $@
	$(MAKE) basic_clean

clean:
	rm -f *.o

basic_clean:
	rm -f $(object_files) runner.o

runner: $(object_files) runner.o $(DEPS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LD_FLAGS)

$(object_files): %.o: %.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

runner.o: runner.cpp
	$(CXX) $(CXXFLAGS) $^ -c -o $@
