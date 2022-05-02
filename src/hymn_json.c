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
    if (count == 0) {
        return hymn_new_none();
    }

    HymnValue argument = arguments[0];
    if (!hymn_is_string(argument)) {
        return hymn_new_none();
    }
    HymnString *input = hymn_as_string(argument);

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

    HymnValue json = hymn_new_none();
    HymnArray *stack = hymn_new_array(0);

    bool parsing_key = false;

    while (i < len) {
        char c = input[i];
        if (c == ':') {
            if (key == NULL) {
                fprintf(stderr, "Exception parsing JSON: Missing key before colon\n");
                goto error;
            }
            parsing_key = false;
        } else if (c == '{') {
            HymnValue table = hymn_new_table_value(hymn_new_table());
            if (stack->length == 0) {
                json = table;
            } else {
                HymnValue head = stack->items[0];
                if (head.is == HYMN_VALUE_ARRAY) {
                    hymn_array_push(hymn_as_array(head), table);
                    hymn_reference(table);
                } else if (head.is == HYMN_VALUE_TABLE) {
                    if (key == NULL) {
                        fprintf(stderr, "Exception parsing JSON: No key\n");
                        goto error;
                    }
                    hymn_set_property_const(H, hymn_as_table(head), key, table);
                    hymn_string_delete(key);
                    key = NULL;
                } else {
                    fprintf(stderr, "Exception parsing JSON: Need object or array\n");
                    goto error;
                }
            }
            hymn_array_insert(stack, 0, table);
            parsing_key = true;
        } else if (c == '[') {
            HymnValue array = hymn_new_array_value(hymn_new_array(0));
            if (stack->length == 0) {
                json = array;
            } else {
                HymnValue head = stack->items[0];
                if (head.is == HYMN_VALUE_ARRAY) {
                    hymn_array_push(hymn_as_array(head), array);
                    hymn_reference(array);
                } else if (head.is == HYMN_VALUE_TABLE) {
                    if (key == NULL) {
                        fprintf(stderr, "Exception parsing JSON: No key\n");
                        goto error;
                    }
                    hymn_set_property_const(H, hymn_as_table(head), key, array);
                    hymn_string_delete(key);
                    key = NULL;
                } else {
                    fprintf(stderr, "Exception parsing JSON: Need object or array\n");
                    goto error;
                }
            }
            hymn_array_insert(stack, 0, array);
            parsing_key = false;
        } else if (c == '}') {
            if (stack->length == 0) {
                fprintf(stderr, "Exception parsing JSON: Not a JSON object\n");
                goto error;
            }
            HymnValue head = stack->items[0];
            if (head.is != HYMN_VALUE_TABLE) {
                fprintf(stderr, "Exception parsing JSON: Expected JSON object but was: %d\n", head.is);
                goto error;
            }
            hymn_array_remove_index(stack, 0);
            parsing_key = stack->length == 0 || stack->items[0].is != HYMN_VALUE_ARRAY;
        } else if (c == ']') {
            if (stack->length == 0) {
                fprintf(stderr, "Exception parsing JSON: Not a JSON array\n");
                goto error;
            }
            HymnValue head = stack->items[0];
            if (head.is != HYMN_VALUE_ARRAY) {
                fprintf(stderr, "Exception parsing JSON: Not a JSON object\n");
                goto error;
            }
            hymn_array_remove_index(stack, 0);
            parsing_key = stack->length == 0 || stack->items[0].is != HYMN_VALUE_ARRAY;
        } else if (c == ',') {
            if (key != NULL) {
                fprintf(stderr, "Exception parsing JSON: Comma after key\n");
                goto error;
            }
            parsing_key = stack->length == 0 || stack->items[0].is != HYMN_VALUE_ARRAY;
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
                HymnObjectString *object = hymn_intern_string(H, string);
                HymnValue value = hymn_new_string_value(object);
                if (stack->length == 0) {
                    json = value;
                    goto done;
                } else {
                    HymnValue head = stack->items[0];
                    if (head.is == HYMN_VALUE_ARRAY) {
                        hymn_array_push(hymn_as_array(head), value);
                        hymn_reference_string(object);
                    } else if (head.is == HYMN_VALUE_TABLE) {
                        if (key == NULL) {
                            fprintf(stderr, "Exception parsing JSON: No key\n");
                            goto error;
                        }
                        hymn_set_property_const(H, hymn_as_table(head), key, value);
                        hymn_string_delete(key);
                        key = NULL;
                    } else {
                        fprintf(stderr, "Exception parsing JSON: Need object or array\n");
                        goto error;
                    }
                }
            }
        } else if ('0' <= c && c <= '9') {
            size_t start = i;
            bool discrete = true;
            while (true) {
                i++;
                if (i >= len) {
                    break;
                }
                char e = input[i];
                if ('0' <= e && e <= '9') {
                } else if (e == '.' || e == 'e') {
                    discrete = false;
                } else {
                    i--;
                    break;
                }
            }
            HymnValue number;
            if (discrete) {
                number = hymn_new_int(strtoll(&input[start], NULL, 10));
            } else {
                number = hymn_new_float(strtod(&input[start], NULL));
            }
            if (stack->length == 0) {
                json = number;
                goto done;
            } else {
                HymnValue head = stack->items[0];
                if (head.is == HYMN_VALUE_ARRAY) {
                    hymn_array_push(hymn_as_array(head), number);
                } else if (head.is == HYMN_VALUE_TABLE) {
                    if (key == NULL) {
                        fprintf(stderr, "Exception parsing JSON: No key\n");
                        goto error;
                    }
                    hymn_set_property_const(H, hymn_as_table(head), key, number);
                    hymn_string_delete(key);
                    key = NULL;
                } else {
                    fprintf(stderr, "Exception parsing JSON: Need object or array\n");
                    goto error;
                }
            }
        } else if (c == 'n') {
            if (i + 3 < len) {
                if (input[i + 1] == 'u' && input[i + 2] == 'l' && input[i + 3] == 'l') {
                    i += 3;
                    if (stack->length == 0) {
                        json = hymn_new_none();
                        goto done;
                    } else {
                        HymnValue head = stack->items[0];
                        if (head.is == HYMN_VALUE_ARRAY) {
                            hymn_array_push(hymn_as_array(head), hymn_new_none());
                        } else if (head.is == HYMN_VALUE_TABLE) {
                            if (key == NULL) {
                                fprintf(stderr, "Exception parsing JSON: No key\n");
                                goto error;
                            }
                            hymn_set_property_const(H, hymn_as_table(head), key, hymn_new_none());
                            hymn_string_delete(key);
                            key = NULL;
                        } else {
                            fprintf(stderr, "Exception parsing JSON: Need object or array\n");
                            goto error;
                        }
                        goto next;
                    }
                }
            }
            fprintf(stderr, "Exception parsing JSON: Expected null\n");
            goto error;
        } else {
            fprintf(stderr, "Exception parsing JSON: Unknown input\n");
            goto error;
        }
    next:
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

    if (stack->length != 0) {
        fprintf(stderr, "Exception parsing JSON: Incomplete\n");
        goto error;
    }

done:
    hymn_string_delete(key);
    free(stack->items);
    free(stack);
    return json;

error:
    fprintf(stderr, "error with JSON\n");
    hymn_string_delete(key);
    hymn_reference(hymn_new_array_value(stack));
    for (HymnInt s = 0; s < stack->length; s++) {
        hymn_reference(stack->items[s]);
    }
    hymn_array_delete(H, stack);
    return hymn_new_none();
}

void hymn_use_json(Hymn *H) {
    HymnTable *json = hymn_new_table();
    hymn_add_function_to_table(H, json, "save", json_save);
    hymn_add_function_to_table(H, json, "parse", json_parse);
    hymn_add_table(H, "json", json);
}
