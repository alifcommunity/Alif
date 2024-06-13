#include "alif.h"

#include "AlifCore_Tuple.h"
#include "AlifCore_Memory.h"

static const wchar_t* skipItem(const wchar_t** , va_list* , int );

// in file alifMem.h in line 63
#define ALIFMEM_NEW(type_, n) \
  ( ((size_t)(n) > LLONG_MAX / sizeof(type_)) ? nullptr :      \
        ( (type_ *) alifMem_dataAlloc((n) * sizeof(type_)) ) )

typedef int (*DestrT)(AlifObject*, void*);


/* Keep track of "objects" that have been allocated or initialized and
   which will need to be deallocated or cleaned up somehow if overall
   parsing fails.
*/
typedef struct {
    void* item;
    DestrT destructor;
} FreeListEntryT;

typedef struct {
    FreeListEntryT* entries;
    int firstAvailable;
    int entriesMalloced;
} FreeListT;

static int vGetArgs1(AlifObject*, const wchar_t* , va_list*, int );

int alifArg_parseTuple(AlifObject* args, const wchar_t* format, ...)
{
    int retval;
    va_list va;

    va_start(va, format);
    retval = vGetArgs1(args, format, &va, 1);
    va_end(va);
    return retval;
}

static int cleanReturn(int retval, FreeListT* freeList)
{
    int index;

    if (retval == 0) {
        for (index = 0; index < freeList->firstAvailable; ++index) {
            freeList->entries[index].destructor(nullptr,
                freeList->entries[index].item);
        }
    }
    if (freeList->entriesMalloced)
        alifMem_dataFree(freeList->entries);
    return retval;
}

static int vGetArgs1_impl(AlifObject* compatArgs, AlifObject* const* stack, int64_t nArgs, const wchar_t* format,
    va_list* pVa, int flags)
{
    wchar_t msgBuf[256];
    int levels[32];
    const wchar_t* fName = nullptr;
    const wchar_t* message = nullptr;
    int min = -1;
    int max = 0;
    int level = 0;
    int endfmt = 0;
    const wchar_t* formatSave = format;
    int64_t i;
    const wchar_t* msg;
    int compat = flags & 1;
    FreeListEntryT staticEntries[8];
    FreeListT freeList;


    freeList.entries = staticEntries;
    freeList.firstAvailable = 0;
    freeList.entriesMalloced = 0;

    flags = flags & ~1;

    while (endfmt == 0) {
        int c = *format++;
        switch (c) {
        case '(':
            if (level == 0)
                max++;
            level++;
            if (level >= 30)
                //_FatalError("too many tuple nesting levels "
                    //"in argument format string");
            break;
        case ')':
            if (level == 0)
                return 0;
                //_FatalError("excess ')' in getargs format");
            else
                level--;
            break;
        case '\0':
            endfmt = 1;
            break;
        case ':':
            fName = format;
            endfmt = 1;
            break;
        case ';':
            message = format;
            endfmt = 1;
            break;
        case '|':
            if (level == 0)
                min = max;
            break;
        default:
            if (level == 0) {
                if (ALIF_ISALPHA(c))
                    if (c != 'e') /* skip encoded */
                        max++;
            }
            break;
        }
    }

    if (level != 0)
        //_FatalError(/* '(' */ "missing ')' in getargs format");

    if (min < 0)
        min = max;

    format = formatSave;

    if (max > 8) {
        freeList.entries = ALIFMEM_NEW(FreeListEntryT, max);
        if (freeList.entries == nullptr) {
            //Err_NoMemory();
            return 0;
        }
        freeList.entriesMalloced = 1;
    }

    if (compat) {
        if (max == 0) {
            if (compatArgs == nullptr)
                return 1;
            //Err_Format(Exc_TypeError,
                //"%.200s%s takes no arguments",
                //fName == nullptrptr ? "function" : fName,
                //fName == nullptrptr ? "" : "()");
            return cleanReturn(0, &freeList);
        }
        else if (min == 1 && max == 1) {
            if (compatArgs == nullptr) {
                //Err_Format(Exc_TypeError,
                    //"%.200s%s takes at least one argument",
                    //fName == nullptrptr ? "function" : fName,
                    //fName == nullptrptr ? "" : "()");
                return cleanReturn(0, &freeList);
            }
            //msg = convertitem(compatArgs, &format, pVa, flags, levels,
                //msgBuf, sizeof(msgBuf), &freeList);
            //if (msg == nullptr)
                //return cleanReturn(1, &freeList);
            //seterror(levels[0], msg, levels + 1, fName, message);
            return cleanReturn(0, &freeList);
        }
        else {
            //Err_SetString(Exc_SystemError,
                //"old style getargs format uses new features");
            return cleanReturn(0, &freeList);
        }
    }

    if (nArgs < min || max < nArgs) {
        //if (message == nullptrptr)
            //Err_Format(Exc_TypeError,
            //    "%.150s%s takes %s %d argument%s (%zd given)",
            //    fName == nullptrptr ? "function" : fName,
            //    fName == nullptrptr ? "" : "()",
            //    min == max ? "exactly"
            //    : nArgs < min ? "at least" : "at most",
            //    nArgs < min ? min : max,
            //    (nArgs < min ? min : max) == 1 ? "" : "s",
            //    nArgs);
        //else
            //Err_SetString(Exc_TypeError, message);
        return cleanReturn(0, &freeList);
    }

    for (i = 0; i < nArgs; i++) {
        if (*format == '|')
            format++;
        //msg = convertitem(stack[i], &format, pVa,
            //flags, levels, msgBuf,
            //sizeof(msgBuf), &freeList);
        //if (msg) {
            //seterror(i + 1, msg, levels, fName, message);
            //return cleanReturn(0, &freeList);
        //}
    }

    if (*format != '\0' && !ALIF_ISALPHA(*format) &&
        *format != '(' &&
        *format != '|' && *format != ':' && *format != ';') {
        //Err_Format(Exc_SystemError,
            //"bad format string: %.200s", formatSave);
        return cleanReturn(0, &freeList);
    }

    return cleanReturn(1, &freeList);
}

int _alifArg_checkPositional(const wchar_t* name, int64_t nargs, int64_t min, int64_t max)
{

    if (nargs < min) {
        if (name != NULL)
            //Err_Format(
                //Exc_TypeError,
                //"%.200s expected %s%zd argument%s, got %zd",
                //name, (min == max ? "" : "at least "), min, min == 1 ? "" : "s", nargs);
        //else
            //Err_Format(
                //Exc_TypeError,
                //"unpacked tuple should have %s%zd element%s,"
                //" but has %zd",
                //(min == max ? "" : "at least "), min, min == 1 ? "" : "s", nargs);
        return 0;
    }

    if (nargs == 0) {
        return 1;
    }

    if (nargs > max) {
        if (name != NULL)
            //Err_Format(
                //Exc_TypeError,
                //"%.200s expected %s%zd argument%s, got %zd",
                //name, (min == max ? "" : "at most "), max, max == 1 ? "" : "s", nargs);
        //else
            //Err_Format(
                //Exc_TypeError,
                //"unpacked tuple should have %s%zd element%s,"
                //" but has %zd",
                //(min == max ? "" : "at most "), max, max == 1 ? "" : "s", nargs);
        return 0;
    }

    return 1;
}

static int vGetArgs1(AlifObject* args, const wchar_t* format, va_list* pVa, int flags)
{
    AlifObject** stack;
    int64_t nArgs;

    if (!(flags & 1)) {
        

        if (!(args->type_ == &_alifTupleType_)) {
            //Err_SetString(Exc_SystemError,
                //"new style getargs format but argument is not a tuple");
            return 0;
        }

        stack = ALIFTUPLE_ITEMS(args);
        nArgs = ((AlifVarObject*)args)->size_;
    }
    else {
        stack = nullptr;
        nArgs = 0;
    }

    return vGetArgs1_impl(args, stack, nArgs, format, pVa, flags);
}

//static void error_unexpected_keyword_arg(AlifObject* kwargs, AlifObject* kwnames, AlifObject* kwTuple, const wchar_t* fname)
//{
//    /* make sure there are no extraneous keyword arguments */
//    int64_t j = 0;
//    while (1) {
//        AlifObject* keyword;
//        if (kwargs != nullptr) {
//            if (!dict_next(kwargs, &j, &keyword, nullptr, nullptr))
//                break;
//        }
//        else {
//            if (j >= Tuple_GET_SIZE(kwnames))
//                break;
//            keyword = Tuple_GET_ITEM(kwnames, j);
//            j++;
//        }
//        if (!Unicode_Check(keyword)) {
//            Err_SetString(Exc_TypeError,
//                "keywords must be strings");
//            return;
//        }
//
//        int match = Sequence_Contains(kwTuple, keyword);
//        if (match <= 0) {
//            if (!match) {
//                Err_Format(Exc_TypeError,
//                    "'%S' is an invalid keyword "
//                    "argument for %.200s%s",
//                    keyword,
//                    (fname == nullptr) ? "this function" : fname,
//                    (fname == nullptr) ? "" : "()");
//            }
//            return;
//        }
//    }
//    /* Something wrong happened. There are extraneous keyword arguments,
//     * but we don't know what. And we don't bother. */
//    Err_Format(Exc_TypeError,
//        "invalid keyword argument for %.200s%s",
//        (fname == nullptr) ? "this function" : fname,
//        (fname == nullptr) ? "" : "()");
//}

#define IS_END_OF_FORMAT(c) (c == '\0' || c == ';' || c == ':')

static int scan_keywords(const wchar_t* const* keywords, int* ptotal, int* pposonly)
{
    /* scan keywords and count the number of positional-only parameters */
    int i;
    for (i = 0; keywords[i] && !*keywords[i]; i++) {
    }
    *pposonly = i;

    /* scan keywords and get greatest possible nbr of args */
    for (; keywords[i]; i++) {
        if (!*keywords[i]) {
            std::wcout << L"اسم معامل الكلمة الدالة فارغ\n" << std::endl;
            exit(-1);
        }
    }
    *ptotal = i;
    return 0;
}

static int parse_format(const wchar_t* format, int total, int npos,
    const wchar_t** pfname, const wchar_t** pCustomMsg,
    int* pMin, int* pMax)
{
    /* grab the function name or custom error msg first (mutually exclusive) */
    const wchar_t* customMsg;
    const wchar_t* fname = wcschr(format, ':');
    if (fname) {
        fname++;
        customMsg = nullptr;
    }
    else {
        customMsg = wcschr(format, ';');
        if (customMsg) {
            customMsg++;
        }
    }

    int min = INT_MAX;
    int max = INT_MAX;
    for (int i = 0; i < total; i++) {
        if (*format == '|') {
            if (min != INT_MAX) {

                //Err_SetString(Exc_SystemError,
                //    "Invalid format string (| specified twice)");
                return -1;
            }
            if (max != INT_MAX) {
                //Err_SetString(Exc_SystemError,
                //    "Invalid format string ($ before |)");
                return -1;
            }
            min = i;
            format++;
        }
        if (*format == '$') {
            if (max != INT_MAX) {
                //Err_SetString(Exc_SystemError,
                //    "Invalid format string ($ specified twice)");
                return -1;
            }
            if (i < npos) {
                //Err_SetString(Exc_SystemError,
                //    "Empty parameter name after $");
                return -1;
            }
            max = i;
            format++;
        }
        if (IS_END_OF_FORMAT(*format)) {
            //Err_Format(Exc_SystemError,
            //    "More keyword list entries (%d) than "
            //    "format specifiers (%d)", total, i);
            return -1;
        }

        const wchar_t* msg = skipItem(&format, nullptr, 0);
        if (msg) {
            //Err_Format(Exc_SystemError, "%s: '%s'", msg,
            //    format);
            return -1;
        }
    }
    min = min(min, total);
    max = min(max, total);

    if (!IS_END_OF_FORMAT(*format) && (*format != '|') && (*format != '$')) {
        //Err_Format(Exc_SystemError,
        //    "more argument specifiers than keyword list entries "
        //    "(remaining format:'%s')", format);
        return -1;
    }

    *pfname = fname;
    *pCustomMsg = customMsg;
    *pMin = min;
    *pMax = max;
    return 0;
}

static AlifObject* new_kWTuple(const wchar_t* const* keywords, int total, int pos)
{
    int nKW = total - pos;
    AlifObject* kwTuple = alifNew_tuple(nKW);
    if (kwTuple == nullptr) {
        return nullptr;
    }
    keywords += pos;
    for (int i = 0; i < nKW; i++) {
        AlifObject* str = alifUStr_decodeStringToUTF8(keywords[i]);
        if (str == nullptr) {
            //ALIF_DECREF(kwTuple);
            return nullptr;
        }
        //Unicode_InternInPlace(&str);
        ((AlifTupleObject*)kwTuple)->items[i] = str;
    }
    return kwTuple;
}

static int _parser_init(AlifArgParser* parser)
{
    const wchar_t* const* keywords = parser->keywords;

    int len, pos;
    if (scan_keywords(keywords, &len, &pos) < 0) {
        return 0;
    }

    const wchar_t* fname, * customMsg = nullptr;
    int min = 0, max = 0;
    if (parser->format) {
        if (parse_format(parser->format, len, pos,
            &fname, &customMsg, &min, &max) < 0) {
            return 0;
        }
    }
    else {
        fname = parser->fname;
    }

    int owned;
    AlifObject* kwTuple = parser->kwTuple;
    if (kwTuple == nullptr) {
        kwTuple = new_kWTuple(keywords, len, pos);
        if (kwTuple == nullptr) {
            return 0;
        }
        owned = 1;
    }
    else {
        owned = 0;
    }

    parser->pos = pos;
    parser->fname = fname;
    parser->customMsg = customMsg;
    parser->min = min;
    parser->max = max;
    parser->kwTuple = kwTuple;
    parser->initialized = owned ? 1 : -1;

    //parser->next = Runtime.getargs.staticParsers;
    //Runtime.getargs.staticParsers = parser;
    return 1;
}

static int parser_init(AlifArgParser* parser)
{
    if (*((volatile int*)&parser->initialized)) {
        return 1;
    }

    int ret = _parser_init(parser);
    return ret;
}

static AlifObject* find_keyword(AlifObject* kwNames, AlifObject* const* kwStack, AlifObject* key)
{
    int64_t i, nKwArgs;

    nKwArgs = ((AlifVarObject*)kwNames)->size_;
    for (i = 0; i < nKwArgs; i++) {
        AlifObject* kwName = ((AlifTupleObject*)kwNames)->items[i];

        /* kwname == key will normally find a match in since keyword keys
           should be interned strings; if not retry below in a new loop. */
        if (kwName == key) {
            return kwStack[i];
        }
    }

    for (i = 0; i < nKwArgs; i++) {
        AlifObject* kwname = ((AlifTupleObject*)kwNames)->items[i];
        if (uStr_eq(kwname, key)) {
            return kwStack[i];
        }
    }
    return nullptr;
}

AlifObject* const* alifArg_unpackKeywords(AlifObject* const* args, int64_t nArgs,
    AlifObject* kwArgs, AlifObject* kwNames,
    AlifArgParser* parser,
    int minPos, int maxPos, int minKw,
    AlifObject** buf)
{
    AlifObject* kwTuple;
    AlifObject* keyword;
    int i, posonly, minPosOnly, maxArgs;
    int reqLimit = minKw ? maxPos + minKw : minPos;
    int64_t nKwArgs;
    AlifObject* const* kwStack = nullptr;

    if (parser == nullptr) {
        //Err_BadInternalCall();
        return nullptr;
    }

    if (kwNames != nullptr && !(kwNames->type_ == &_alifTupleType_)) {
        //Err_BadInternalCall();
        return nullptr;
    }

    if (args == nullptr && nArgs == 0) {
        args = buf;
    }

    if (!parser_init(parser)) {
        return nullptr;
    }

    kwTuple = parser->kwTuple;
    posonly = parser->pos;
    minPosOnly = min(posonly, minPos);
    maxArgs = posonly + (int)((AlifVarObject*)kwTuple)->size_;

    if (kwArgs != nullptr) {
        nKwArgs = ((AlifDictObject*)kwArgs)->size_;
    }
    else if (kwNames != nullptr) {
        nKwArgs = ((AlifVarObject*)kwNames)->size_;
        kwStack = args + nArgs;
    }
    else {
        nKwArgs = 0;
    }
    if (nKwArgs == 0 && minKw == 0 && minPos <= nArgs && nArgs <= maxPos) {
        /* Fast path. */
        return args;
    }
    if (nArgs + nKwArgs > maxArgs) {
        /* Adding "keyword" (when nArgs == 0) prevents producing wrong error
           messages in some special cases (see bpo-31229). */
        //Err_Format(Exc_TypeError,
        //    "%.200s%s takes at most %d %sargument%s (%zd given)",
        //    (parser->fname == nullptr) ? "function" : parser->fname,
        //    (parser->fname == nullptr) ? "" : "()",
        //    maxArgs,
        //    (nArgs == 0) ? "keyword " : "",
        //    (maxArgs == 1) ? "" : "s",
        //    nArgs + nKwArgs);
        return nullptr;
    }
    if (nArgs > maxPos) {
        if (maxPos == 0) {
            //Err_Format(Exc_TypeError,
            //    "%.200s%s takes no positional arguments",
            //    (parser->fname == nullptr) ? "function" : parser->fname,
            //    (parser->fname == nullptr) ? "" : "()");
        }
        else {
            //Err_Format(Exc_TypeError,
            //    "%.200s%s takes %s %d positional argument%s (%zd given)",
            //    (parser->fname == nullptr) ? "function" : parser->fname,
            //    (parser->fname == nullptr) ? "" : "()",
            //    (minPos < maxPos) ? "at most" : "exactly",
            //    maxPos,
            //    (maxPos == 1) ? "" : "s",
            //    nArgs);
        }
        return nullptr;
    }
    if (nArgs < minPosOnly) {
        //Err_Format(Exc_TypeError,
        //    "%.200s%s takes %s %d positional argument%s"
        //    " (%zd given)",
        //    (parser->fname == nullptr) ? "function" : parser->fname,
        //    (parser->fname == nullptr) ? "" : "()",
        //    minPosOnly < maxPos ? "at least" : "exactly",
        //    minPosOnly,
        //    minPosOnly == 1 ? "" : "s",
        //    nArgs);
        return nullptr;
    }

    /* copy tuple args */
    for (i = 0; i < nArgs; i++) {
        buf[i] = args[i];
    }

    /* copy keyword args using kwTuple to drive process */
    for (i = max((int)nArgs, posonly); i < maxArgs; i++) {
        AlifObject* currentArg;
        if (nKwArgs) {
            keyword = ((AlifTupleObject*)kwTuple)->items[i - posonly];
            if (kwArgs != nullptr) {
                currentArg = dict_getItem(kwArgs, keyword);
                if (currentArg == nullptr) {
                    return nullptr;
                }
            }
            else {
                currentArg = find_keyword(kwNames, kwStack, keyword);
            }
        }
        else if (i >= reqLimit) {
            break;
        }
        else {
            currentArg = nullptr;
        }

        buf[i] = currentArg;

        if (currentArg) {
            currentArg;
            --nKwArgs;
        }
        else if (i < minPos || (maxPos <= i && i < reqLimit)) {
            /* Less arguments than required */
            keyword = ((AlifTupleObject*)kwTuple)->items[i - posonly];
            //Err_Format(Exc_TypeError, "%.200s%s missing required "
            //    "argument '%U' (pos %d)",
            //    (parser->fname == nullptr) ? "function" : parser->fname,
            //    (parser->fname == nullptr) ? "" : "()",
            //    keyword, i + 1);
            return nullptr;
        }
    }

    if (nKwArgs > 0) {
        /* make sure there are no arguments given by name and position */
        for (i = posonly; i < nArgs; i++) {
            AlifObject* currentArg;
            keyword = ((AlifTupleObject*)kwTuple)->items[i - posonly];
            if (kwArgs != nullptr) {
                currentArg = dict_getItem(kwArgs, keyword);
                if (currentArg == nullptr) {
                    return nullptr;
                }
            }
            else {
                currentArg = find_keyword(kwNames, kwStack, keyword);
            }
            if (currentArg) {
                currentArg;
                /* arg present in tuple and in dict */
                //Err_Format(Exc_TypeError,
                //    "argument for %.200s%s given by name ('%U') "
                //    "and position (%d)",
                //    (parser->fname == nullptr) ? "function" : parser->fname,
                //    (parser->fname == nullptr) ? "" : "()",
                //    keyword, i + 1);
                return nullptr;
            }
        }

        //error_unexpected_keyword_arg(kwArgs, kwNames, kwTuple, parser->fname);
        return nullptr;
    }

    return buf;
}

static const wchar_t* skipItem(const wchar_t** pFormat, va_list* pVa, int flags)
{
    const wchar_t* format = *pFormat;
    wchar_t c = *format++;

    switch (c) {

        /*
         * codes that take a single data pointer as an argument
         * (the type_ of the pointer is irrelevant)
         */

    case 'b': /* byte -- very short int */
    case 'B': /* byte as bitfield */
    case 'h': /* short int */
    case 'H': /* short int as bitfield */
    case 'i': /* int */
    case 'I': /* int sized bitfield */
    case 'l': /* long int */
    case 'k': /* long int sized bitfield */
    case 'L': /* long long */
    case 'K': /* long long sized bitfield */
    case 'n': /* int64_t */
    case 'f': /* float */
    case 'd': /* double */
    case 'D': /* complex double */
    case 'c': /* wchar_t */
    case 'C': /* unicode wchar_t */
    case 'p': /* boolean predicate */
    case 'S': /* string object */
    case 'Y': /* string object */
    case 'U': /* unicode string object */
    {
        if (pVa != nullptr) {
            (void)va_arg(*pVa, void*);
        }
        break;
    }

    /* string codes */

    case 'e': /* string with encoding */
    {
        if (pVa != nullptr) {
            (void)va_arg(*pVa, const wchar_t*);
        }
        if (!(*format == 's' || *format == 't'))
            /* after 'e', only 's' and 't' is allowed */
            goto err;
        format++;
    }
    /* fall through */

    case 's': /* string */
    case 'z': /* string or None */
    case 'y': /* bytes */
    case 'w': /* buffer, read-write */
    {
        if (pVa != nullptr) {
            (void)va_arg(*pVa, wchar_t**);
        }
        if (*format == '#') {
            if (pVa != nullptr) {
                (void)va_arg(*pVa, int64_t*);
            }
            format++;
        }
        else if ((c == 's' || c == 'z' || c == 'y' || c == 'w')
            && *format == '*')
        {
            format++;
        }
        break;
    }

    case 'O': /* object */
    {
        if (*format == '!') {
            format++;
            if (pVa != nullptr) {
                (void)va_arg(*pVa, AlifInitObject*);
                (void)va_arg(*pVa, AlifObject**);
            }
        }
        else if (*format == '&') {
            typedef int (*converter)(AlifObject*, void*);
            if (pVa != nullptr) {
                (void)va_arg(*pVa, converter);
                (void)va_arg(*pVa, void*);
            }
            format++;
        }
        else {
            if (pVa != nullptr) {
                (void)va_arg(*pVa, AlifObject**);
            }
        }
        break;
    }

    case '(':           /* bypass tuple, not handled at all previously */
    {
        const wchar_t* msg;
        for (;;) {
            if (*format == ')')
                break;
            if (IS_END_OF_FORMAT(*format))
                return L"Unmatched left paren in format "
                "string";
            msg = skipItem(&format, pVa, flags);
            if (msg)
                return msg;
        }
        format++;
        break;
    }

    case ')':
        return L"Unmatched right paren in format string";

    default:
    err:
        return L"impossible<bad format wchar_t>";

    }

    *pFormat = format;
    return nullptr;
}

int alifSubArg_checkPositional(const wchar_t* _name, int64_t _nArgs,
    int64_t _min, int64_t _max)
{
    if (_nArgs < _min) {
        //if (_name != NULL)
        //    Err_Format(
        //        Exc_TypeError,
        //        "%.200s expected %s%zd argument%s, got %zd",
        //        _name, (_min == _max ? "" : "at least "), _min, _min == 1 ? "" : "s", _nArgs);
        //else
        //    Err_Format(
        //        Exc_TypeError,
        //        "unpacked tuple should have %s%zd element%s,"
        //        " but has %zd",
        //        (_min == _max ? "" : "at least "), _min, _min == 1 ? "" : "s", _nArgs);
        return 0;
    }

    if (_nArgs == 0) {
        return 1;
    }

    if (_nArgs > _max) {
        //if (_name != NULL)
        //    Err_Format(
        //        Exc_TypeError,
        //        "%.200s expected %s%zd argument%s, got %zd",
        //        _name, (_min == _max ? "" : "at most "), _max, _max == 1 ? "" : "s", _nArgs);
        //else
        //    Err_Format(
        //        Exc_TypeError,
        //        "unpacked tuple should have %s%zd element%s,"
        //        " but has %zd",
        //        (_min == _max ? "" : "at most "), _max, _max == 1 ? "" : "s", _nArgs);
        return 0;
    }

    return 1;
}

int alifSubArg_noKwnames(const wchar_t* _funcname, AlifObject* _kwnames)
{
    if (_kwnames == NULL) {
        return 1;
    }


    if (((AlifVarObject*)_kwnames)->size_ == 0) {
        return 1;
    }

    return 0;
}
