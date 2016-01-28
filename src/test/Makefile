TARGETS=utility

all: $(TARGETS)

test: $(TARGETS) $(addprefix run-,$(TARGETS))

%: %.cpp
	$(CXX) $(CFLAGS) -o$@ $^ -lboost_unit_test_framework

run-%: %
	-./$^ --output_format=XML --log_level=test_suite > $(^)-report.xml

clean:
	rm $(TARGETS) *-report.xml