CXX:=g++
object_files=$(addsuffix .o, $(basename $(filter-out runner.cpp,$(wildcard *.cpp))))

.PHONY: test clean done runner

test: CXXFLAGS := -fPIC -g -O0 -lgtest_main -lgtest -lpthread
test: runner
	$(MAKE) clean
	./runner

done: CXXFLAGS := -O3 -fPIC
done: libflushreload.so
libflushreload.so: flush_reload.o
	$(CXX) $(CXXFLAGS) -shared $^ -o $@

clean:
	rm -f *.o

flush_reload.o: $(object_files)
	ld -r $^ -o $@

runner: $(object_files) runner.o
	$(CXX) $^ -o $@ $(CXXFLAGS)

$(object_files): %.o: %.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

runner.o: runner.cpp
	$(CXX) $(CXXFLAGS) $^ -c -o $@
