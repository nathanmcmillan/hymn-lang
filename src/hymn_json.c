/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "hymn_json.h"

struct PointerSet {
    int count;
    int capacity;
    void **items;
};

static bool pointer_set_has(struct PointerSet *set, void *pointer) {
    void **items = set->items;
    if (items) {
        int count = set->count;
        for (int i = 0; i < count; i++) {
            if (pointer == items[i]) {
                return true;
            }
        }
    }
    return false;
}

static void pointer_set_add(struct PointerSet *set, void *pointer) {
    if (set->items) {
        int count = set->count;
        if (count >= set->capacity) {
            set->capacity *= 2;
            set->items = hymn_realloc_int(set->items, set->capacity, sizeof(void *));
        }
        set->items[count] = pointer;
        set->count = count + 1;
    } else {
        set->count = 1;
        set->capacity = 1;
        set->items = hymn_calloc(1, sizeof(void *));
        set->items[0] = pointer;
    }
}

static HymnString *json_save_recursive(HymnValue value, struct PointerSet *set) {
    switch (value.is) {
    case HYMN_VALUE_UNDEFINED:
    case HYMN_VALUE_NONE: return hymn_new_string("null");
    case HYMN_VALUE_BOOL: return hymn_as_bool(value) ? hymn_new_string("true") : hymn_new_string("false");
    case HYMN_VALUE_INTEGER: return hymn_int_to_string(hymn_as_int(value));
    case HYMN_VALUE_FLOAT: return hymn_float_to_string(hymn_as_float(value));
    case HYMN_VALUE_STRING: return hymn_quote_string(hymn_as_string(value));
    case HYMN_VALUE_ARRAY: {
        HymnArray *array = hymn_as_array(value);
        if (array == NULL || array->length == 0 || pointer_set_has(set, array)) {
            return hymn_new_string("[]");
        } else {
            pointer_set_add(set, array);
        }
        HymnString *string = hymn_new_string("[");
        for (HymnInt i = 0; i < array->length; i++) {
            if (i != 0) {
                string = hymn_string_append(string, ", ");
            }
            HymnString *add = json_save_recursive(array->items[i], set);
            string = hymn_string_append(string, add);
            hymn_string_delete(add);
        }
        string = hymn_string_append_char(string, ']');
        return string;
    }
    case HYMN_VALUE_TABLE: {
        HymnTable *table = hymn_as_table(value);
        if (table == NULL || table->size == 0 || pointer_set_has(set, table)) {
            return hymn_new_string("{}");
        } else {
            pointer_set_add(set, table);
        }
        int size = table->size;
        HymnObjectString **keys = hymn_malloc_int(size, sizeof(HymnObjectString *));
        unsigned int total = 0;
        unsigned int bins = table->bins;
        for (unsigned int i = 0; i < bins; i++) {
            HymnTableItem *item = table->items[i];
            while (item != NULL) {
                HymnString *string = item->key->string;
                unsigned int insert = 0;
                while (insert != total) {
                    if (strcmp(string, keys[insert]->string) < 0) {
                        for (unsigned int swap = total; swap > insert; swap--) {
                            keys[swap] = keys[swap - 1];
                        }
                        break;
                    }
                    insert++;
                }
                keys[insert] = item->key;
                total++;
                item = item->next;
            }
        }
        HymnString *string = hymn_new_string("{ ");
        for (int i = 0; i < size; i++) {
            if (i != 0) {
                string = hymn_string_append(string, ", ");
            }
            HymnObjectString *key = keys[i];
            HymnValue item = hymn_table_get(table, key->string);
            HymnString *add = json_save_recursive(item, set);
            HymnString *quote = hymn_quote_string(key->string);
            string = hymn_string_append(string, quote);
            string = hymn_string_append(string, ": ");
            string = hymn_string_append(string, add);
            hymn_string_delete(quote);
            hymn_string_delete(add);
        }
        string = hymn_string_append(string, " }");
        free(keys);
        return string;
    }
    case HYMN_VALUE_FUNC: {
        HymnFunction *func = hymn_as_func(value);
        if (func->name) return hymn_string_format("\"%s\"", func->name);
        if (func->script) return hymn_string_format("\"%s\"", func->script);
        return hymn_new_string("\"script\"");
    }
    case HYMN_VALUE_FUNC_NATIVE: return hymn_string_copy(hymn_as_native(value)->name->string);
    case HYMN_VALUE_POINTER: return hymn_string_format("\"%p\"", hymn_as_pointer(value));
    default:
        break;
    }
    return hymn_new_string("?");
}

static HymnValue json_save(Hymn *H, int count, HymnValue *arguments) {
    if (count == 0) {
        return hymn_new_exception(H, "missing value");
    }
    struct PointerSet set = {.count = 0, .capacity = 0, .items = NULL};
    HymnString *json = json_save_recursive(arguments[0], &set);
    free(set.items);
    HymnObjectString *string = hymn_intern_string(H, json);
    return hymn_new_string_value(string);
}

static HymnValue json_parse(Hymn *H, int count, HymnValue *arguments) {
    if (count == 0) {
        return hymn_new_exception(H, "missing json");
    }

    HymnValue argument = arguments[0];
    if (!hymn_is_string(argument)) {
        return hymn_new_exception(H, "json must be a string");
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

    const char *error_message = NULL;

    while (i < len) {
        char c = input[i];
        if (c == ':') {
            if (key == NULL) {
                error_message = "parsing JSON: missing key before colon";
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
                        error_message = "parsing JSON: no key";
                        goto error;
                    }
                    hymn_set_property_const(H, hymn_as_table(head), key, table);
                    hymn_string_delete(key);
                    key = NULL;
                } else {
                    error_message = "parsing JSON: need object or array";
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
                        error_message = "parsing JSON: no key";
                        goto error;
                    }
                    hymn_set_property_const(H, hymn_as_table(head), key, array);
                    hymn_string_delete(key);
                    key = NULL;
                } else {
                    error_message = "parsing JSON: need object or array";
                    goto error;
                }
            }
            hymn_array_insert(stack, 0, array);
            parsing_key = false;
        } else if (c == '}') {
            if (stack->length == 0) {
                error_message = "parsing JSON: not a JSON object";
                goto error;
            }
            HymnValue head = stack->items[0];
            if (head.is != HYMN_VALUE_TABLE) {
                error_message = "parsing JSON: expected JSON object";
                goto error;
            }
            hymn_array_remove_index(stack, 0);
            parsing_key = stack->length == 0 || stack->items[0].is != HYMN_VALUE_ARRAY;
        } else if (c == ']') {
            if (stack->length == 0) {
                error_message = "parsing JSON: not a JSON array";
                goto error;
            }
            HymnValue head = stack->items[0];
            if (head.is != HYMN_VALUE_ARRAY) {
                error_message = "parsing JSON: not a JSON object";
                goto error;
            }
            hymn_array_remove_index(stack, 0);
            parsing_key = stack->length == 0 || stack->items[0].is != HYMN_VALUE_ARRAY;
        } else if (c == ',') {
            if (key != NULL) {
                error_message = "parsing JSON: comma after key";
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
                            error_message = "parsing JSON: no key";
                            goto error;
                        }
                        hymn_set_property_const(H, hymn_as_table(head), key, value);
                        hymn_string_delete(key);
                        key = NULL;
                    } else {
                        error_message = "parsing JSON: need object or array";
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
                        error_message = "parsing JSON: no key";
                        goto error;
                    }
                    hymn_set_property_const(H, hymn_as_table(head), key, number);
                    hymn_string_delete(key);
                    key = NULL;
                } else {
                    error_message = "parsing JSON: need object or array";
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
                                error_message = "parsing JSON: no key";
                                goto error;
                            }
                            hymn_set_property_const(H, hymn_as_table(head), key, hymn_new_none());
                            hymn_string_delete(key);
                            key = NULL;
                        } else {
                            error_message = "parsing JSON: need object or array";
                            goto error;
                        }
                        goto next;
                    }
                }
            }
            error_message = "parsing JSON: expected null";
            goto error;
        } else {
            error_message = "parsing JSON: unknown input";
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
        error_message = "parsing JSON: incomplete";
        goto error;
    }

done:
    hymn_string_delete(key);
    free(stack->items);
    free(stack);
    return json;

error:
    hymn_string_delete(key);
    hymn_reference(hymn_new_array_value(stack));
    for (HymnInt s = 0; s < stack->length; s++) {
        hymn_reference(stack->items[s]);
    }
    hymn_array_delete(H, stack);
    return hymn_new_exception(H, error_message);
}

void hymn_use_json(Hymn *H) {
    HymnTable *json = hymn_new_table();
    hymn_add_function_to_table(H, json, "save", json_save);
    hymn_add_function_to_table(H, json, "parse", json_parse);
    hymn_add_table(H, "json", json);
}
