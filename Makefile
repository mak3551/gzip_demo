CXX = g++
CXXFLAGS = -O2 -Wall -Wextra
LDFLAGS = -ldeflate
TARGET = gunzip_demo
SOURCE = gunzip_demo.cpp gzipdata.cpp
OBJECT = $(SOURCE:.cpp=.o)

.PHONY: all clean rebuild test

$(TARGET): $(OBJECT)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	rm -f $(OBJECT) $(TARGET)
rebuild: clean $(TARGET)
test: $(TARGET)
	./test.sh