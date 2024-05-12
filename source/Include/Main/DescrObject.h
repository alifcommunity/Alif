#pragma once

typedef AlifObject* (*Getter)(AlifObject*, void*);
typedef int (*Setter)(AlifObject*, AlifObject*, void*);


class AlifGetSetDef {
public:
	const wchar_t* name_{};
	Getter get_{};
	Setter set_{};
	const wchar_t* doc_{};
	void* closure_{};
};

class AlifMemberDef {
	const wchar_t* name_{};
	int type_{};
	int64_t offset_{};
	int flags_{};
	const wchar_t* doc_{};
};