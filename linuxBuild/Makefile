CPP = c++ # المترجم
CPPFLAGS = -std=c++20
RELEASE_FLAGS = -O2 -w
DEBUG_FLAGS = -O0 -g
# -g تخزين معلومات debug
# -Wall طباعة معظم التنبيهات
# -w عدم طباعة التنبيهات

SOURCE_DIR = ../source
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
TARGET_FREEZE =  $(BUILD_DIR)/freezemodule
TARGET_ALIF = $(BUILD_DIR)/alif


SOURCES = 	$(wildcard $(SOURCE_DIR)/Alif/*.cpp)				\
			$(filter-out $(SOURCE_DIR)/AlifCore/Objects/GetPath.cpp, $(wildcard $(SOURCE_DIR)/AlifCore/Objects/*.cpp))	\
			$(wildcard $(SOURCE_DIR)/AlifCore/Objects/dtoa/*.cpp)	\
			$(wildcard $(SOURCE_DIR)/Modules/*.cpp)			\
			$(wildcard $(SOURCE_DIR)/Modules/_io/*.cpp)		\
			$(wildcard $(SOURCE_DIR)/FreezeModule/*.cpp)		\


INCLUDES =  -I$(SOURCE_DIR)/Alif							\
			-I$(SOURCE_DIR)/AlifCore/Include/Core				\
			-I$(SOURCE_DIR)/AlifCore/Include/Main				\
			-I$(SOURCE_DIR)/AlifCore/Include/Main/Atomics			\
			-I$(SOURCE_DIR)/AlifCore/Include/Main/mimalloc			\
			-I$(SOURCE_DIR)/AlifCore/Include/Main/StringLib			\
			-I$(SOURCE_DIR)/AlifCore/Include/Main/FrozenModules		\
			-I$(SOURCE_DIR)/AlifCore/Objects/clinic				\
			-I$(SOURCE_DIR)/Modules/_io					\
			-I$(SOURCE_DIR)/Modules/_io/clinic			\


all: release

# تحديد اعلام بناء البرنامج
freeze_release: CPPFLAGS += $(RELEASE_FLAGS)
alif_release:  CPPFLAGS += $(RELEASE_FLAGS)
freeze_debug: CPPFLAGS += $(DEBUG_FLAGS)
alif_debug: CPPFLAGS += $(DEBUG_FLAGS)

# تقوم هذه التعليمة بإضافة أمر الى make
# حيث يصبح make -j(nproc)
# وذلك لبناء اللغة بإستخدام ممرات متعددة multithread
release:
	@make -j$(shell nproc) freeze_release
	@make alif_release

debug:
	@make -j2 freeze_debug
	@make alif_debug 

freeze_release: $(TARGET_FREEZE)
alif_release : $(TARGET_ALIF)
freeze_debug: $(TARGET_FREEZE)
alif_debug: $(TARGET_ALIF)


# تقوم هذه التعليمة بأخذ كل ملفات المصدر المنتهية ب .cpp 
# وتستبدلها بالملفات المنتهية ب .o
OBJS = $(SOURCES:.cpp=.o)
# تقوم هذه التعليمة بتحويل مسارات الملفات من المصدر الى مسار ملف الكائنات
OBJECTS = $(OBJS:$(SOURCE_DIR)/%.o=$(OBJ_DIR)/%.o)

# تقوم هذه التعليمة بإضافة أمر جديد الى المتغير CPPFLAGS
CPPFLAGS += $(addprefix ,$(INCLUDES))


# تقوم هذه التعليمة
# بتحديد قاعدة نمطية لتجميع ملفات المصدر (.cpp)
# إلى ملفات الكائنات (.o). يتم تحديد الملفات المصدرية في المسار المصدري (SOURCE_DIR)
# ويتم تحديد الكائنات في المسار (OBJ_DIR).

# القاعدة النمطية تعني أن كل كائن (.o) في المسار (OBJ_DIR)
# يعتمد على مصدر (.cpp) في المسار المصدري (SOURCE_DIR).
# يتم استخدام العلامة النسبية % لتحديد الجزء المتغير من الاسم 
# (على سبيل المثال، file1 في file1.cpp و file1.o).
$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(OBJ_DIR)
	@echo "ترجمة.." $<
	@$(CPP) $(CPPFLAGS) -c $< -o $@

# يقوم بتحديد قاعدة لربط ملفات الكائنات (OBJECTS)
# إلى ملف الهدف النهائي (TARGET). 
# يتم تحديد ملفات الكائنات في مسار الكائنات (BUILD_DIR).

# القاعدة تعني أن ملف الهدف النهائي (TARGET) 
# يعتمد على جميع ملفات الكائنات (OBJECTS).
# يتم استخدام العلامة النسبية | لتأكيد أن مسار الكائنات (BUILD_DIR)
# يجب أن يكون موجودًا قبل بدء عملية الربط.
$(TARGET_FREEZE): $(OBJECTS) | $(BUILD_DIR)
	@echo "جاري ربط مجرد الشيفرة..."
	@$(CPP) $(filter-out build/obj/Alif/alif.o build/obj/AlifCore/Objects/Frozen.o, $^) -o $@
	@echo "تم بناء مجرد الشيفرة بنجاح"
	@echo "جاري بناء ومعالجة البرامج المكتوبة بلغة ألف"
	@$(shell) ./build/freezemodule GetPath ../source/Modules/تحديد_المسارات.alif GetPath.h
	@$(shell) mv -f GetPath.h ../source/AlifCore/Include/Main/FrozenModules


$(TARGET_ALIF): $(OBJECTS) | $(BUILD_DIR)
	@echo "ترجمة.." $(SOURCE_DIR)/AlifCore/Objects/GetPath.cpp
	@$(CPP) $(CPPFLAGS) -c $(SOURCE_DIR)/AlifCore/Objects/GetPath.cpp -o $(OBJ_DIR)/AlifCore/Objects/GetPath.o
	@echo "جاري ربط لغة ألف..."
	@$(CPP) $(filter-out build/obj/FreezeModule/FreezeModule.o build/obj/FreezeModule/GetPathFreeze.o, $^)	\
	 -o $@ $(OBJ_DIR)/AlifCore/Objects/GetPath.o
	@echo "تم بناء لغة ألف بنجاح"


# إنشاء الملفات المطلوبة
ifeq ($(wildcard $(BUILD_DIR)),)
$(BUILD_DIR):
	@mkdir -p $@
$(OBJ_DIR):
	@mkdir -p $@
	@mkdir -p $(OBJ_DIR)/Alif
	@mkdir -p $(OBJ_DIR)/AlifCore/Objects
	@mkdir -p $(OBJ_DIR)/AlifCore/Objects/dtoa
	@mkdir -p $(OBJ_DIR)/Modules
	@mkdir -p $(OBJ_DIR)/Modules/_io
	@mkdir -p $(OBJ_DIR)/FreezeModule
endif


clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean release debug


help:
	@echo "نظام بناء لغة ألف النسخة الخامسة"
	@echo "طريقة البناء:"
	@echo "make          # لبناء اللغة في وضع الإصدار"
	@echo "make release  # لبناء اللغة في وضع الإصدار"
	@echo "make debug    # لبناء اللغة في وضع تتبع الاخطاء"
	@echo "make clean    # لحذف جميع ملفات البناء"

