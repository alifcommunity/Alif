CPP = c++
CPPFLAGS = -Wall -std=c++20

SOURCE_DIR = ../source/Objects
INCLUDE_DIR = ../source/Include
BUILD_DIR = build

SOURCE_FILES = $(wildcard $(SOURCE_DIR)/Alif/*.cpp) $(wildcard $(SOURCE_DIR)/Core/*.cpp) $(wildcard $(SOURCE_DIR)/Main/*.cpp)

INCLUDES = -I$(INCLUDE_DIR)/Alif -I$(INCLUDE_DIR)/Core -I$(INCLUDE_DIR)/Main

OBJECTS = $(SOURCE_FILES:$(SOURCE_DIR)/%/%.cpp=$(BUILD_DIR)/%.o)

TARGET = $(BUILD_DIR)/alif

ifeq ($(wildcard $(BUILD_DIR)),)
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	@echo "تم إنشاء المجلد"
endif

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CPP) $(CPPFLAGS) $(INCLUDES) $(OBJECTS) -o $@

clean:
	em -rf $(BUILD_DIR)