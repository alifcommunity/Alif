CPP = c++
CPPFLAGS = -Wall -std=c++20
# CPPFLAGS = -w -std=c++20 -Wno-c++11-narrowing # بدون تحذيرات، مع التشغيل على نظام aarch64a

SOURCE_DIR = ../source
BUILD_DIR = build

SOURCE_FILES = $(wildcard $(SOURCE_DIR)/Alif/*.cpp) $(wildcard $(SOURCE_DIR)/AlifCore/Objects/*.cpp)

INCLUDES =  -I$(SOURCE_DIR)/Alif							\
			-I$(SOURCE_DIR)/AlifCore/Include/Core			\
			-I$(SOURCE_DIR)/AlifCore/Include/Main			\
			-I$(SOURCE_DIR)/AlifCore/Include/Main/Atomics	\
			-I$(SOURCE_DIR)/AlifCore/Include/Main/mimalloc	\
			-I$(SOURCE_DIR)/AlifCore/Include/Main/StringLib	\

OBJECTS = $(SOURCE_FILES:$(SOURCE_DIR)/AlifCore/Objects/%/%.cpp=$(BUILD_DIR)/%.o)

TARGET = $(BUILD_DIR)/alif

ifeq ($(wildcard $(BUILD_DIR)),)
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	@echo "تم إنشاء المجلد - قم بإعادة البناء"
endif

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CPP) $(CPPFLAGS) $(INCLUDES) $(OBJECTS) -o $@

clean:
	rm -rf $(BUILD_DIR)