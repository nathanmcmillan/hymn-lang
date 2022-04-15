/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn.h"

static HymnValue json_save(Hymn *H, int count, HymnValue *arguments) {
    (void)H;
    (void)count;
    (void)arguments;
    return hymn_new_none();
}

static HymnValue json_parse(Hymn *H, int count, HymnValue *arguments) {
    (void)H;

    if (count == 0) {
        return hymn_new_none();
    }
    HymnValue string = arguments[0];
    if (!hymn_is_string(string)) {
        return hymn_new_none();
    }
    HymnString *input = hymn_as_string(string);

    printf("parsing JSON...\n");

    size_t i = 0;
    size_t len = hymn_string_len(input);

    while (true) {
        if (i >= len) {
            return hymn_new_none();
        }
        char c = input[i];
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            break;
        }
    }

    HymnString *key = NULL;
    HymnString *value = NULL;

    HymnValue json = hymn_new_none();
    HymnArray *stack = hymn_new_array(0);

    char pc = '\0';
    bool parsing_key = true;

    while (i < len) {
        char c = input[i];
        printf("<%c>\n", c);
        if (c == ':') {
            parsing_key = false;
            pc = c;
        } else if (c == '{') {
            HymnValue table = hymn_new_table_value(hymn_new_table());
            if (stack->length == 0) {
                json = table;
            } else {
                HymnValue head = stack->items[0];
                if (head.is == HYMN_VALUE_ARRAY) {
                    hymn_array_push(hymn_as_array(head), table);
                } else {
                    hymn_set_property_const(H, hymn_as_table(head), key, table);
                    hymn_string_zero(key);
                }
            }
            hymn_array_insert(stack, 0, table);
            parsing_key = true;
            pc = c;
        } else if (c == '[') {
            HymnValue array = hymn_new_array_value(hymn_new_array(0));
            if (stack->length == 0) {
                json = array;
            } else {
                HymnValue head = stack->items[0];
                if (head.is == HYMN_VALUE_ARRAY) {
                    hymn_array_push(hymn_as_array(head), array);
                } else {
                    hymn_set_property_const(H, hymn_as_table(head), key, array);
                    hymn_string_zero(key);
                }
            }
            hymn_array_insert(stack, 0, array);
            parsing_key = false;
            pc = c;
        } else if (c == '}') {
            if (pc != ' ' && pc != ']' && pc != '{' && pc != '}' && pc != '\n') {
                if (stack->length == 0) {
                    printf("end of JSON object, stack was empty\n");
                    goto error;
                }
                HymnValue head = stack->items[0];
                if (head.is != HYMN_VALUE_TABLE) {
                    printf("expected JSON object\n");
                    goto error;
                }
                HymnString *string = hymn_new_string(value);
                HymnObjectString *object = hymn_new_string_object(string);
                hymn_set_property_const(H, hymn_as_table(head), key, hymn_new_string_value(object));
                hymn_string_zero(key);
                hymn_string_zero(value);
            }
            hymn_array_remove_index(stack, 0);
            parsing_key = stack->length == 0 || stack->items[0].is != HYMN_VALUE_ARRAY;
            pc = c;
        } else if (c == ']') {
            if (pc != ' ' && pc != '}' && pc != '[' && pc != ']' && pc != '\n') {
                if (stack->length == 0) {
                    printf("end of JSON array, stack was empty\n");
                    goto error;
                }
                HymnString *string = hymn_new_string(value);
                HymnObjectString *object = hymn_new_string_object(string);
                HymnValue head = stack->items[0];
                hymn_array_push(hymn_as_array(head), hymn_new_string_value(object));
                hymn_string_zero(value);
            }
            hymn_array_remove_index(stack, 0);
            parsing_key = stack->length == 0 || stack->items[0].is != HYMN_VALUE_ARRAY;
            pc = c;
        } else if (c == '"') {
            i++;
            if (i >= len) {
                break;
            }
            char e = input[i];
            HymnString *string = hymn_new_string("");
            while (i < len) {
                if (e == '"' || e == '\n') {
                    break;
                } else if (e == '\\' && i + 1 < len && input[i + 1] == '"') {
                    string = hymn_string_append_char(string, '"');
                    i += 2;
                    e = input[i];
                } else {
                    string = hymn_string_append_char(string, e);
                    i++;
                    e = input[i];
                }
            }
            if (parsing_key) {
                key = string;
            } else {
                value = string;
            }
            pc = c;
        } else {
            printf("unknown\n");
            goto error;
        }
        while (true) {
            i++;
            if (i >= len) {
                break;
            }
            c = input[i];
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                break;
            }
        }
    }

    hymn_string_delete(key);
    hymn_string_delete(value);
    printf("done parsing JSON\n");
    return json;

error:
    printf("error with JSON\n");
    hymn_string_delete(key);
    hymn_string_delete(value);
    hymn_dereference(H, json);
    return hymn_new_none();
}

void hymn_use_json(Hymn *H) {
    HymnTable *json = hymn_new_table();
    hymn_add_function_to_table(H, json, "save", json_save);
    hymn_add_function_to_table(H, json, "parse", json_parse);
    hymn_add_table(H, "json", json);
}
