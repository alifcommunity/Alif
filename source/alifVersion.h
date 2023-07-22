#pragma once

/*
	نسخة ألف
*/

/* قيم */
#define ALIF_RELEASE_LEVEL_ALIF   0xA
#define ALIF_RELEASE_LEVEL_BAA    0xB
#define ALIF_RELEASE_LEVEL_FINAL  0xF

/* النسخة كقيمة رقمية */
#define ALIF_MAJOR_VERSION        5
#define ALIF_MINOR_VERSION        0
#define ALIF_Extra_VERSION        0
#define ALIF_RELEASE_LEVEL        ALIF_RELEASE_LEVEL_ALIF

/* النسخة كقيمة نصية */
#define ALIF_VERSION              "5.0.0"

/*
	نسخة ألف ك 4-بايت رقم ست عشري, مثال: 5.1.0 أ.
	تستخدم هذه للمقارنات العددية, مثال if ALIF_VERSION_HEX >= ...: 
*/
#define ALIF_VERSION_HEX ((ALIF_MAJOR_VERSION << 16)   |  \
                           (ALIF_MINOR_VERSION << 8)   |  \
                           (ALIF_MICRO_VERSION << 4)   |  \
                           (ALIF_RELEASE_LEVEL << 0))
