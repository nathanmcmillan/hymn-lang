/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn.h"

static HymnValue json_save(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    (void)count;
    (void)arguments;
    // none -> null
    return hymn_new_none();
}

// static int skip_space(HymnString *s, int i) {
//     size_t len = hymn_string_len(s);
//     if (i + 1 >= len) {
//         fprintf(stderr, "Wad error at index %d, for: %s", i, s);
//         exit(1);
//     }
//     i++;
//     char c = s[i];
//     if (c != '\n' && c != ' ') {
//         return i - 1;
//     }
//     do {
//         i++;
//         if (i == len) {
//             return i;
//         }
//         c = s[i];
//     } while (c == '\n' || c == ' ');
//     return i - 1;
// }

static HymnValue json_parse(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    (void)count;
    (void)arguments;
    return hymn_new_none();

    //     (void)H;

    //     if (count == 0) {
    //         return hymn_new_none();
    //     }
    //     HymnValue string = arguments[0];
    //     if (!hymn_is_string(string)) {
    //         return hymn_new_none();
    //     }
    //     HymnString *input = hymn_as_string(string);

    //     printf("parsing JSON...\n");

    //     HymnTable *table = hymn_new_table();

    //     HymnArray *stack = hymn_new_array(1);
    //     stack->items[0] = hymn_new_table_value(table);

    //     HymnString *key = hymn_new_string("");
    //     HymnString *value = hymn_new_string("");

    //     char pc = '\0';
    //     bool parsing_key = true;

    //     int len = hymn_string_len(input);

    //     for (int i = 0; i < len; i++) {
    //         char c = input[i];
    //         if (c == '\n' || c == ' ') {
    //             if (!parsing_key && pc != '}' && pc != ']') {
    //                 if (stack->length == 0) {
    //                     printf("Empty stack\n");
    //                     return hymn_new_none();
    //                 }
    //                 Wad *head = stack->items[0];
    //                 Wad *child = NewWadString(value);
    //                 if (head->type == WAD_ARRAY) {
    //                     ArrayPush(WadAsArray(head), child);
    //                 } else {
    //                     WadAddToTable(head, key, child);
    //                     string_zero(key);
    //                     parsing_key = true;
    //                 }
    //                 string_zero(value);
    //             }
    //             pc = c;
    //             i = skip_space(input, i);
    //         } else if (c == ':') {
    //             parsing_key = false;
    //             pc = c;
    //             i = skip_space(input, i);
    //         } else if (c == '{') {
    //             if (ArrayIsEmpty(stack)) {
    //                 ERROR("Wad: Empty stack");
    //                 exit(1);
    //             }
    //             Wad *map = NewWadTable();
    //             Wad *head = stack->items[0];
    //             if (head->type == WAD_ARRAY) {
    //                 ArrayPush(WadAsArray(head), map);
    //             } else {
    //                 WadAddToTable(head, key, map);
    //                 string_zero(key);
    //             }
    //             ArrayInsert(stack, 0, map);
    //             parsing_key = true;
    //             pc = c;
    //             i = skip_space(input, i);
    //         } else if (c == '[') {
    //             if (ArrayIsEmpty(stack)) {
    //                 ERROR("Wad: Empty stack");
    //                 exit(1);
    //             }
    //             Wad *ls = NewWadArray();
    //             Wad *head = stack->items[0];
    //             if (head->type == WAD_ARRAY) {
    //                 ArrayPush(WadAsArray(head), ls);
    //             } else {
    //                 WadAddToTable(head, key, ls);
    //                 string_zero(key);
    //             }
    //             ArrayInsert(stack, 0, ls);
    //             parsing_key = false;
    //             pc = c;
    //             i = skip_space(input, i);
    //         } else if (c == '}') {
    //             if (pc != ' ' && pc != ']' && pc != '{' && pc != '}' && pc != '\n') {
    //                 if (ArrayIsEmpty(stack)) {
    //                     ERROR("Wad: Empty stack");
    //                     exit(1);
    //                 }
    //                 Wad *head = stack->items[0];
    //                 WadAddToTable(head, key, NewWadString(value));
    //                 string_zero(key);
    //                 string_zero(value);
    //             }
    //             ArrayRemoveAt(stack, 0);
    //             if (ArrayIsEmpty(stack)) {
    //                 ERROR("Wad: Empty stack");
    //                 exit(1);
    //             }
    //             Wad *head = stack->items[0];
    //             if (head->type == WAD_ARRAY) {
    //                 parsing_key = false;
    //             } else {
    //                 parsing_key = true;
    //             }
    //             pc = c;
    //             i = skip_space(input, i);
    //         } else if (c == ']') {
    //             if (pc != ' ' && pc != '}' && pc != '[' && pc != ']' && pc != '\n') {
    //                 if (ArrayIsEmpty(stack)) {
    //                     ERROR("Wad: Empty stack");
    //                     exit(1);
    //                 }
    //                 Wad *head = stack->items[0];
    //                 ArrayPush(WadAsArray(head), NewWadString(value));
    //                 string_zero(value);
    //             }
    //             ArrayRemoveAt(stack, 0);
    //             if (ArrayIsEmpty(stack)) {
    //                 ERROR("Wad: Empty stack");
    //                 exit(1);
    //             }
    //             Wad *head = stack->items[0];
    //             if (head->type == WAD_ARRAY) {
    //                 parsing_key = false;
    //             } else {
    //                 parsing_key = true;
    //             }
    //             pc = c;
    //             i = skip_space(input, i);
    //         } else if (c == '"') {
    //             i++;
    //             if (i == len) {
    //                 break;
    //             }
    //             char e = input[i];
    //             while (i < len) {
    //                 if (e == '"' || e == '\n') {
    //                     break;
    //                 } else if (e == '\\' && i + 1 < len && input[i + 1] == '"') {
    //                     value = StringAppendChar(value, '"');
    //                     i += 2;
    //                     e = input[i];
    //                 } else {
    //                     value = StringAppendChar(value, e);
    //                     i++;
    //                     e = input[i];
    //                 }
    //             }
    //             pc = c;
    //         } else if (parsing_key) {
    //             pc = c;
    //             key = StringAppendChar(key, c);
    //         } else {
    //             pc = c;
    //             value = StringAppendChar(value, c);
    //         }
    //     }

    //     if (ArrayIsEmpty(stack)) {
    //         ERROR("Wad: Empty stack");
    //         exit(1);
    //     }

    //     if (pc != ' ' && pc != ']' && pc != '}' && pc != '\n') {
    //         Wad *head = stack->items[0];
    //         WadAddToTable(head, key, NewWadString(value));
    //     }

    //     StringFree(key);
    //     StringFree(value);

    //     printf("done parsing wad...\n");
    //     return wad;

    //     return hymn_new_none();
}

void hymn_use_json(Hymn *H) {
    HymnTable *json = hymn_new_table();
    hymn_add_function_to_table(H, json, "save", json_save);
    hymn_add_function_to_table(H, json, "parse", json_parse);
    hymn_add_table(H, "json", json);
}
