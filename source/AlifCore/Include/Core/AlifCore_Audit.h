#pragma once



/* Runtime audit hook state */

class AlifAuditHookEntry {
public:
	class AlifAuditHookEntry* next{};
	AlifAuditHookFunction hookCFunction{};
	void* userData{};
};


//extern AlifIntT _alifSys_audit(AlifThread*, const char*, const char*, ...);

//extern void _alifSys_clearAuditHooks(AlifThread*);
