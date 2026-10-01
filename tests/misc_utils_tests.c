/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unity/examples/unity_config.h"
#include "unity/src/unity.h"
#include "common.h"
#include "../cJSON_Utils.h"

static void cjson_utils_functions_shouldnt_crash_with_null_pointers(void)
{
    cJSON *item = cJSON_CreateString("item");
    TEST_ASSERT_NOT_NULL(item);

    TEST_ASSERT_NULL(cJSONUtils_GetPointer(item, NULL));
    TEST_ASSERT_NULL(cJSONUtils_GetPointer(NULL, "pointer"));
    TEST_ASSERT_NULL(cJSONUtils_GetPointerCaseSensitive(NULL, "pointer"));
    TEST_ASSERT_NULL(cJSONUtils_GetPointerCaseSensitive(item, NULL));
    TEST_ASSERT_NULL(cJSONUtils_GeneratePatches(item, NULL));
    TEST_ASSERT_NULL(cJSONUtils_GeneratePatches(NULL, item));
    TEST_ASSERT_NULL(cJSONUtils_GeneratePatchesCaseSensitive(item, NULL));
    TEST_ASSERT_NULL(cJSONUtils_GeneratePatchesCaseSensitive(NULL, item));
    cJSONUtils_AddPatchToArray(item, "path", "add", NULL);
    cJSONUtils_AddPatchToArray(item, "path", NULL, item);
    cJSONUtils_AddPatchToArray(item, NULL, "add", item);
    cJSONUtils_AddPatchToArray(NULL, "path", "add", item);
    cJSONUtils_ApplyPatches(item, NULL);
    cJSONUtils_ApplyPatches(NULL, item);
    cJSONUtils_ApplyPatchesCaseSensitive(item, NULL);
    cJSONUtils_ApplyPatchesCaseSensitive(NULL, item);
    TEST_ASSERT_NULL(cJSONUtils_MergePatch(item, NULL));
    item = cJSON_CreateString("item");
    TEST_ASSERT_NULL(cJSONUtils_MergePatchCaseSensitive(item, NULL));
    item = cJSON_CreateString("item");
    /* these calls are actually valid */
    /* cJSONUtils_MergePatch(NULL, item); */
    /* cJSONUtils_MergePatchCaseSensitive(NULL, item);*/
    /* cJSONUtils_GenerateMergePatch(item, NULL); */
    /* cJSONUtils_GenerateMergePatch(NULL, item); */
    /* cJSONUtils_GenerateMergePatchCaseSensitive(item, NULL); */
    /* cJSONUtils_GenerateMergePatchCaseSensitive(NULL, item); */

    TEST_ASSERT_NULL(cJSONUtils_FindPointerFromObjectTo(item, NULL));
    TEST_ASSERT_NULL(cJSONUtils_FindPointerFromObjectTo(NULL, item));
    cJSONUtils_SortObject(NULL);
    cJSONUtils_SortObjectCaseSensitive(NULL);

    cJSON_Delete(item);
}

static void cjson_utils_merge_patch_case_sensitive_nested(void)
{
    cJSON *from = cJSON_Parse("{\"sub\":{\"key\":\"val\"}}");
    cJSON *to = cJSON_Parse("{\"sub\":{\"KEY\":\"val\"}}");
    cJSON *patch = NULL;

    TEST_ASSERT_NOT_NULL(from);
    TEST_ASSERT_NOT_NULL(to);

    patch = cJSONUtils_GenerateMergePatchCaseSensitive(from, to);
    TEST_ASSERT_NOT_NULL(patch);
    /* In case-sensitive mode, key and KEY are distinct, so sub must be patched */
    TEST_ASSERT_NOT_NULL(cJSON_GetObjectItemCaseSensitive(patch, "sub"));

    cJSON_Delete(patch);
    cJSON_Delete(from);
    cJSON_Delete(to);
}

static void cjson_utils_array_index_pointer_hardening(void)
{
    cJSON *arr = cJSON_Parse("[\"zero\", \"one\"]");
    TEST_ASSERT_NOT_NULL(arr);

    /* RFC 6901: empty array token is invalid */
    TEST_ASSERT_NULL(cJSONUtils_GetPointer(arr, "/"));
    TEST_ASSERT_NULL(cJSONUtils_GetPointerCaseSensitive(arr, "/"));

    /* Valid index */
    TEST_ASSERT_NOT_NULL(cJSONUtils_GetPointer(arr, "/0"));
    TEST_ASSERT_NOT_NULL(cJSONUtils_GetPointer(arr, "/1"));
    TEST_ASSERT_NULL(cJSONUtils_GetPointer(arr, "/2"));

    /* Integer overflow protection */
    TEST_ASSERT_NULL(cJSONUtils_GetPointer(arr, "/18446744073709551615999999"));

    cJSON_Delete(arr);
}

static void cjson_core_hardening_tests(void)
{
    char buf[16];
    cJSON *num = cJSON_CreateNumber(42);
    cJSON *str = cJSON_CreateString("hello world");
    cJSON *obj = cJSON_CreateObject();
    cJSON *unlinked = cJSON_CreateString("unlinked");
    cJSON *repl = cJSON_CreateString("replacement");

    TEST_ASSERT_NOT_NULL(num);
    TEST_ASSERT_NOT_NULL(str);
    TEST_ASSERT_NOT_NULL(obj);
    TEST_ASSERT_NOT_NULL(unlinked);
    TEST_ASSERT_NOT_NULL(repl);

    /* Zero-length buffer rejection */
    TEST_ASSERT_FALSE(cJSON_PrintPreallocated(num, buf, 0, false));

    /* Unlinked item rejection in ReplaceItemViaPointer */
    cJSON_AddItemToObject(obj, "first", num);
    TEST_ASSERT_FALSE(cJSON_ReplaceItemViaPointer(obj, unlinked, repl));

    /* cJSON_SetValuestring overlapping safety */
    TEST_ASSERT_NOT_NULL(cJSON_SetValuestring(str, str->valuestring + 6));
    TEST_ASSERT_EQUAL_STRING("world", str->valuestring);

    cJSON_Delete(obj);
    cJSON_Delete(str);
    cJSON_Delete(unlinked);
    cJSON_Delete(repl);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(cjson_utils_functions_shouldnt_crash_with_null_pointers);
    RUN_TEST(cjson_utils_merge_patch_case_sensitive_nested);
    RUN_TEST(cjson_utils_array_index_pointer_hardening);
    RUN_TEST(cjson_core_hardening_tests);

    return UNITY_END();
}
