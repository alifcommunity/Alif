#pragma once

#include <iostream>

#include "AlifVersion.h"
#include "AlifConfig.h"

#ifdef _WIN64
#include <windows.h>
#endif

#include <algorithm>

#include "AlifPort.h"
#include "AlifMacro.h"
#include "InitConfig.h"
#include "AlifTypeDefs.h"
#include "FileManip.h"
#include "AlifBuffer.h"
#include "AlifObject.h"
#include "ObjImpl.h"
#include "IntegerObject.h"
#include "BoolObject.h"
#include "FloatObject.h"
#include "UStrObject.h"
#include "ListObject.h"
#include "TupleObject.h"
#include "DictObject.h"
#include "MethodObject.h"
#include "FunctionObject.h"
#include "BytesObject.h"
#include "SliceObject.h"
#include "IterObject.h"
#include "DescrObject.h"
#include "SetObject.h"
#include "Code.h"
#include "AlifCapsule.h"
#include "AlifTime.h"
#include "AlifThread.h"
#include "ModSupport.h"
#include "Abstract.h"
#include "AlifCppType.h"
#include "Compile.h"
#include "AlifRun.h"
#include "AlifEval.h"
#include "AlifState.h"
#include "Import.h"
