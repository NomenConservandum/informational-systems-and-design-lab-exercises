CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O2

SRC_DIR  := src
BUILD_DIR := build

LIB_Target := $(BUILD_DIR)/libatm_state.a
APP_Target := $(BUILD_DIR)/atm_app
TST_Target := $(BUILD_DIR)/test_atm

all: $(LIB_Target) $(APP_Target) $(TST_Target)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/atm_state_lib.o: $(SRC_DIR)/atm_state_lib.cpp $(SRC_DIR)/atm_state_lib.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/atm_state_lib.cpp -o $(BUILD_DIR)/atm_state_lib.o

$(LIB_Target): $(BUILD_DIR)/atm_state_lib.o
	ar rcs $(LIB_Target) $(BUILD_DIR)/atm_state_lib.o

$(APP_Target): $(SRC_DIR)/main.cpp $(LIB_Target)
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/main.cpp $(LIB_Target) -o $(APP_Target)

$(TST_Target): $(SRC_DIR)/test_atm.cpp $(LIB_Target)
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/test_atm.cpp $(LIB_Target) -o $(TST_Target)

test: $(TST_Target)
	./$(TST_Target)

demo: $(APP_Target)
	./$(APP_Target) --demo

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test demo clean

