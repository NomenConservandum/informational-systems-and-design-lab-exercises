CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -O2
AR       := ar
ARFLAGS  := rcs

BUILD_DIR := build
SRC_DIR   := src

LIB_OBJ    := $(BUILD_DIR)/phone_state_lib.o
STATIC_LIB := $(BUILD_DIR)/libphone_state.a
APP_BIN    := $(BUILD_DIR)/phone_app
TEST_BIN   := $(BUILD_DIR)/test_phone

.PHONY: all app test demo clean

all: $(STATIC_LIB) $(APP_BIN) $(TEST_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(LIB_OBJ): $(SRC_DIR)/phone_state_lib.cpp $(SRC_DIR)/phone_state_lib.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/phone_state_lib.cpp -o $(LIB_OBJ)

$(STATIC_LIB): $(LIB_OBJ)
	$(AR) $(ARFLAGS) $(STATIC_LIB) $(LIB_OBJ)

$(APP_BIN): $(SRC_DIR)/main.cpp $(STATIC_LIB)
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/main.cpp $(STATIC_LIB) -o $(APP_BIN)

$(TEST_BIN): $(SRC_DIR)/test_phone.cpp $(STATIC_LIB)
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/test_phone.cpp $(STATIC_LIB) -o $(TEST_BIN)

app: $(APP_BIN)

test: $(TEST_BIN)
	./$(TEST_BIN)

demo: $(APP_BIN)
	./$(APP_BIN) --demo

clean:
	rm -rf $(BUILD_DIR)
