#pragma once



#define MAX_PREALLOC 12
#define PREALLOC_SIZE(_maxSplit) \
    (_maxSplit >= MAX_PREALLOC ? MAX_PREALLOC : _maxSplit+1)


// هذا الفانكشت اصله ماكرو ولكن بسبب اهي لا يمكنني استخدام template في الماكرو فتم تحيوله للفانكشن
template <typename STRINGLIB_CHAR>
static void split_add(STRINGLIB_CHAR* data, int64_t left, int64_t right,
    int64_t& count, AlifObject* list) {

    AlifObject* sub{};
    if (sizeof(STRINGLIB_CHAR) > sizeof(uint32_t))
    {
        sub = alifUnicode_fromUint32(((uint32_t*)data)+(left), (right)-(left));
    }
    else {
        sub = alifUnicode_fromUint16(((uint16_t*)data)+(left), (right)-(left));
    }

    if (count < MAX_PREALLOC) {
            alifList_setItem(list, count, sub);      
    }
    else {
        alifList_append(list, sub);
                                                          
    }                                           
    count++;

}

#define FIX_PREALLOC_SIZE(list) ((AlifVarObject*)list)->size_ = count


//template <typename STRINGLIB_CHAR>
//AlifObject* split_whiteSpace(AlifObject * str_obj,
//    const STRINGLIB_CHAR * str, int64_t str_len,
//    int64_t maxcount)
//{
//    int64_t i, j, count = 0;
//    AlifObject* list = alifNew_list(PREALLOC_SIZE(maxcount));
//    AlifObject* sub;
//
//    if (list == NULL)
//        return NULL;
//
//    i = j = 0;
//    while (maxcount-- > 0) {
//        while (i < str_len && STRINGLIB_ISSPACE(str[i]))
//            i++;
//        if (i == str_len) break;
//        j = i; i++;
//        while (i < str_len && !STRINGLIB_ISSPACE(str[i]))
//            i++;
//#if !STRINGLIB_MUTABLE
//        if (j == 0 && i == str_len && (str_obj->type_ == &_typeUnicode_)) {
//            /* No whitespace in str_obj, so just use it as list[0] */
//            //Py_INCREF(str_obj);
//            alifList_setItem(list, 0, (AlifObject*)str_obj);
//            count++;
//            break;
//        }
//#endif
//        split_add(str, j, i);
//    }
//
//    if (i < str_len) {
//        /* Only occurs when maxcount was reached */
//        /* Skip any remaining whitespace and copy to end of string */
//        while (i < str_len && STRINGLIB_ISSPACE(str[i]))
//            i++;
//        if (i != str_len)
//            split_add(str, i, str_len);
//    }
//    FIX_PREALLOC_SIZE(list);
//    return list;
//
//onError:
//    //Py_DECREF(list);
//    return NULL;
//}

template <typename STRINGLIB_CHAR>
AlifObject* split_char(AlifObject* str_obj,
    const STRINGLIB_CHAR* str, int64_t str_len,
    const STRINGLIB_CHAR ch,
    int64_t maxcount)
{
    int64_t i, j, count = 0;
    AlifObject* list = alifNew_list(PREALLOC_SIZE(maxcount));
    AlifObject* sub;

    if (list == NULL)
        return NULL;

    i = j = 0;
    while ((j < str_len) && (maxcount-- > 0)) {
        for (; j < str_len; j++) {
            /* I found that using memchr makes no difference */
            if (str[j] == ch) {
                split_add(str, i, j, count, list);
                i = j = j + 1;
                break;
            }
        }
    }
#if !STRINGLIB_MUTABLE
    if (count == 0 && (str_obj->type_ == &_alifUStrType_)) {
        /* ch not in str_obj, so just use str_obj as list[0] */
        //_INCREF(str_obj);
        alifList_setItem(list, 0, (AlifObject*)str_obj);
        count++;
    }
    else
#endif
        if (i <= str_len) {
            split_add(str, i, str_len, count, list);
        }
    FIX_PREALLOC_SIZE(list);
    return list;

onError:
    list;
    return NULL;
}

template <typename STRINGLIB_CHAR>
AlifObject* split(AlifObject* str_obj,
    const STRINGLIB_CHAR* str, int64_t str_len,
    const STRINGLIB_CHAR* sep, int64_t sep_len,
    int64_t maxcount)
{
    int64_t i, j, pos, count = 0;
    AlifObject* list, * sub;

    if (sep_len == 0) {
        //Err_SetString(Exc_ValueError, "empty separator");
        return NULL;
    }
    else if (sep_len == 1)
        return split_char(str_obj, str, str_len, sep[0], maxcount);

    list = alifNew_list(PREALLOC_SIZE(maxcount));
    if (list == NULL)
        return NULL;

    i = j = 0;
    while (maxcount-- > 0) {
        pos = fastSearch(str + i, str_len - i, sep, sep_len, -1, FAST_SEARCH);
        if (pos < 0)
            break;
        j = i + pos;
        split_add(str, i, j, count, list);
        i = j + sep_len;
    }
#if !STRINGLIB_MUTABLE
    if (count == 0 && (str_obj->type_ == &_alifUStrType_)) {
        /* No match in str_obj, so just use it as list[0] */
        //_INCREF(str_obj);
        alifList_setItem(list, 0, (AlifObject*)str_obj);
        count++;
    }
    else
#endif
    {
        split_add(str, i, str_len, count , list);
    }
    FIX_PREALLOC_SIZE(list);
    return list;

onError:
    list;
    return NULL;
}