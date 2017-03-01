TARGETS=test
CFLAGS = -std=c++11 -g
LIBS = -L../../build/lib -lgraphparser -lboost_unit_test_framework

all: $(TARGETS)

%: %.cpp
	$(CXX) $(CFLAGS) -o$@ $^  $(LIBS)
	
clean:
	rm $(TARGETS)
