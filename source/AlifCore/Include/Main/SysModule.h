#pragma once

AlifObject* alifSys_getObject(const char*); // 7
AlifIntT alifSys_setObject(const char*, AlifObject*); // 8


/* --------------------------------------------------------------------------------------- */

typedef AlifIntT(*AlifAuditHookFunction)(const char*, AlifObject*, void*); // 5
