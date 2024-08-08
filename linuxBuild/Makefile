CPP = c++
CPPFLAGS = -Wall -std=c++20

SOURCE_DIR = ../source
BUILD_DIR = build

SOURCE_FILES = $(wildcard $(SOURCE_DIR)/Alif/*.cpp) $(wildcard $(SOURCE_DIR)/Objects/*.cpp)

INCLUDES = -I$(SOURCE_DIR)/Alif -I$(SOURCE_DIR)/Include/Core -I$(SOURCE_DIR)/Include/Main

OBJECTS = $(SOURCE_FILES:$(SOURCE_DIR)/Objects/%.cpp=$(BUILD_DIR)/%.o) $(SOURCE_FILES:$(SOURCE_DIR)/Alif/%.cpp=$(BUILD_DIR)/%.o)

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