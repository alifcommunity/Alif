#pragma once

#include <iostream>
#include <algorithm>
#include <limits.h>
#include <stdint.h> // uint64_t
#include <stdarg.h>
#include <math.h>
#include <string.h>

#include "AlifVersion.h"
#include "AlifConfig.h"

#ifdef _WINDOWS
#include <io.h>
#include <windows.h>
#pragma warning(disable : 4996) // for disable unsafe functions error
#endif

#include "AlifPort.h"
#include "AlifMacro.h"
#include "AlifMemory.h"
#include "InitConfig.h"
//#include "AlifTypeDefs.h"
//#include "AlifBuffer.h"
//#include "AlifObject.h"
//#include "ObjImpl.h"
//#include "IntegerObject.h"
//#include "BoolObject.h"
//#include "FloatObject.h"
#include "UStrObject.h"
//#include "AlifErrors.h"
//#include "ListObject.h"
//#include "TupleObject.h"
//#include "DictObject.h"
//#include "MethodObject.h"
//#include "ModuleObject.h"
//#include "ClassObject.h"
//#include "FunctionObject.h"
//#include "BytesObject.h"
//#include "SliceObject.h"
//#include "IterObject.h"
//#include "DescrObject.h"
//#include "SetObject.h"
//#include "Code.h"
//#include "AlifCapsule.h"
//#include "AlifTime.h"
#include "AlifThread.h"
#include "AlifState.h"
//#include "ModSupport.h"
//#include "Abstract.h"
//#include "AlifCppType.h"
//#include "Compile.h"
#include "Import.h"
#include "FileUtils.h"
//#include "AlifRun.h"
#include "AlifLifeCycle.h"
//#include "AlifEval.h"
//#include "GenericAliasObject.h"
