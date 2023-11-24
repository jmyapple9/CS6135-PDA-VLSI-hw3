CXXFLAGS = -O3 -std=c++17 # -Wall -Wextra
OUTPUT_DIR = ../bin/
TESTCASE_DIR = ../testcase/
OUTPUT_DIR_PUBLIC = ../output/
PROGRAM_NAME = hw3

all:
	g++ $(CXXFLAGS) main.cpp -o $(OUTPUT_DIR)$(PROGRAM_NAME)

clean:
	rm $(OUTPUT_DIR)$(PROGRAM_NAME)

run: all
	$(OUTPUT_DIR)$(PROGRAM_NAME) $(TESTCASE_DIR)public$(filter-out $@,$(MAKECMDGOALS)).txt $(OUTPUT_DIR_PUBLIC)public$(filter-out $@,$(MAKECMDGOALS)).floorplan

verify:
	$(OUTPUT_DIR)verify $(TESTCASE_DIR)public$(filter-out $@,$(MAKECMDGOALS)).txt $(OUTPUT_DIR_PUBLIC)public$(filter-out $@,$(MAKECMDGOALS)).floorplan

runall: all
	$(OUTPUT_DIR)$(PROGRAM_NAME) $(TESTCASE_DIR)public$(filter-out $@,$(MAKECMDGOALS)).txt $(OUTPUT_DIR_PUBLIC)public$(filter-out $@,$(MAKECMDGOALS)).floorplan
	$(OUTPUT_DIR)verify $(TESTCASE_DIR)public$(filter-out $@,$(MAKECMDGOALS)).txt $(OUTPUT_DIR_PUBLIC)public$(filter-out $@,$(MAKECMDGOALS)).floorplan

sample:
	g++ $(CXXFLAGS) main.cpp -o $(OUTPUT_DIR)$(PROGRAM_NAME)
	$(OUTPUT_DIR)$(PROGRAM_NAME) $(TESTCASE_DIR)sample.txt $(OUTPUT_DIR_PUBLIC)sample.floorplan
	$(OUTPUT_DIR)verify $(TESTCASE_DIR)sample.txt $(OUTPUT_DIR_PUBLIC)sample.floorplan

%:
	@: