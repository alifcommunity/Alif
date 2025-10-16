#pragma once




extern AlifTypeObject _alifFrameType_; // 5


#define ALIFFRAME_CHECK(op) ALIF_IS_TYPE((op), &_alifFrameType_) // 8

AlifCodeObject* alifFrame_getCode(AlifFrameObject*);





AlifIntT alifUnstable_interpreterFrameGetLine(class AlifInterpreterFrame*); // 37
