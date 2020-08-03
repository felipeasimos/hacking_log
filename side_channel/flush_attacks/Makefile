CXX:=g++
object_files=$(addsuffix .o, $(basename $(filter-out runner.cpp,$(wildcard *.cpp))))

.PHONY: test clean done runner

test: CXXFLAGS := -fPIC -g -O0
test: runner
	@valgrind -q --leak-check=full --show-leak-kinds=all ./runner

done: CXXFLAGS := -O3 -fPIC
done: libflushreload.so
libflushreload.so: flush_reload.o
	$(CXX) $(CXXFLAGS) -shared $^ -o $@

clean:
	rm -f *.o *.so

flush_reload.o: $(object_files)
	ld -r $^ -o $@

runner: $(object_files) runner.o
	$(CXX) $^ -lgtest_main -o $@

$(object_files): %.o: %.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

runner.o: runner.cpp
	$(CXX) $(CXXFLAGS) $^ -c -o $@
