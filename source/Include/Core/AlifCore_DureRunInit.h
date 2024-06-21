#pragma once


#define ALIF_DURERUNSTATE_INIT(dureRun) \
	{		                            \
        0,      \
        0,      \
        0,                   \
	    {               \
            nullptr, nullptr, -1,       \
        },                  \
        0,          \
        nullptr,            \
        ALIFTSS_NEEDS_INIT,      \
        0,             \
        {0, nullptr},        \
        {nullptr},              \
		{0},		\
        ALIF_INTERPRETERSTATE_INIT(_dureRun_.mainInterpreter), \
	} \

#define ALIF_INTERPRETERSTATE_INIT(interpreter) \
    { \
         nullptr,                       \
         0,                             \
         0,                             \
         0, 0,                       \
        {                               \
            0, nullptr, nullptr, 0, 0,   \
        },                               \
        nullptr,                        \
        0,              \
    }
