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
#include "AlifMath.h"
#include "AlifMemory.h"
#include "InitConfig.h"
#include "AlifTypeDefs.h"
#include "AlifAtomic.h"
#include "AlifBuffer.h"
#include "Lock.h"
#include "AlifObject.h"
#include "RefCount.h"
#include "TypeSlots.h"
#include "ObjImpl.h"
#include "AlifHash.h"
#include "ByteArrayObject.h"
#include "LongObject.h"
#include "BoolObject.h"
#include "FloatObject.h"
#include "ComplexObject.h"
#include "RangeObject.h"
#include "UStrObject.h"
#include "AlifErrors.h"
#include "LongIntRepr.h"
#include "ListObject.h"
#include "TupleObject.h"
#include "DictObject.h"
#include "MethodObject.h"
#include "ModuleObject.h"
#include "ClassObject.h"
#include "FuncObject.h"
#include "FileObject.h"
#include "BytesObject.h"
#include "SliceObject.h"
#include "CellObject.h"
#include "DescrObject.h"
#include "SetObject.h"
#include "Code.h"
#include "AlifCapsule.h"
#include "AlifFrame.h"
#include "Traceback.h"
#include "AlifTime.h"
#include "IterObject.h"
#include "AlifState.h"
#include "WeakRefObject.h"
#include "StructSeq.h"
#include "GenericAliasObject.h"
#include "Codecs.h"
#include "AlifThread.h"
#include "ModSupport.h"
#include "AlifCompile.h"
#include "Abstract.h"
#include "AlifCppType.h"
#include "SysModule.h"
#include "Import.h"
#include "BltinModule.h"
#include "CriticalSection.h"
#include "AlifStrToDouble.h"
#include "FileUtils.h"
#include "AlifRun.h"
#include "AlifLifeCycle.h"
#include "AlifEval.h"

