/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <math.h>

#include "hymn_math.h"

#define PI 3.14159265358979323846

static HymnValue math_abs(Hymn *H, int count, HymnValue *arguments) {
    if (count >= 1) {
        HymnValue value = arguments[0];
        if (hymn_is_int(value)) {
            int64_t number = hymn_as_int(value);
            if (number < 0) {
                return hymn_new_int(-number);
            }
            return value;
        } else if (hymn_is_float(value)) {
            double number = hymn_as_float(value);
            if (number < 0) {
                return hymn_new_float(-number);
            }
            return value;
        }
        return hymn_new_exception(H, "argument must be a number");
    }
    return hymn_new_exception(H, "missing number");
}

static HymnValue math_min(Hymn *H, int count, HymnValue *arguments) {
    if (count >= 2) {
        HymnValue a = arguments[0];
        HymnValue b = arguments[1];
        if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                if (hymn_as_int(a) < hymn_as_int(b)) {
                    return a;
                } else {
                    return b;
                }
            } else if (hymn_is_float(b)) {
                if ((HymnFloat)hymn_as_int(a) < hymn_as_float(b)) {
                    return a;
                } else {
                    return b;
                }
            }
        } else if (hymn_is_float(a)) {
            if (hymn_is_int(b)) {
                if (hymn_as_float(a) < (HymnFloat)hymn_as_int(b)) {
                    return a;
                } else {
                    return b;
                }
            } else if (hymn_is_float(b)) {
                if (hymn_as_float(a) < hymn_as_float(b)) {
                    return a;
                } else {
                    return b;
                }
            }
        }
        return hymn_new_exception(H, "arguments must be numbers");
    }
    return hymn_new_exception(H, "missing numbers");
}

static HymnValue math_max(Hymn *H, int count, HymnValue *arguments) {
    if (count >= 2) {
        HymnValue a = arguments[0];
        HymnValue b = arguments[1];
        if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                if (hymn_as_int(a) > hymn_as_int(b)) {
                    return a;
                } else {
                    return b;
                }
            } else if (hymn_is_float(b)) {
                if ((HymnFloat)hymn_as_int(a) > hymn_as_float(b)) {
                    return a;
                } else {
                    return b;
                }
            }
        } else if (hymn_is_float(a)) {
            if (hymn_is_int(b)) {
                if (hymn_as_float(a) > (HymnFloat)hymn_as_int(b)) {
                    return a;
                } else {
                    return b;
                }
            } else if (hymn_is_float(b)) {
                if (hymn_as_float(a) > hymn_as_float(b)) {
                    return a;
                } else {
                    return b;
                }
            }
        }
        return hymn_new_exception(H, "arguments must be numbers");
    }
    return hymn_new_exception(H, "missing numbers");
}

#define MATH_FUNCTION(fun)                                          \
    if (count >= 1) {                                               \
        HymnValue value = arguments[0];                             \
        if (hymn_is_int(value)) {                                   \
            return hymn_new_float(fun((double)hymn_as_int(value))); \
        } else if (hymn_is_float(value)) {                          \
            return hymn_new_float(fun(hymn_as_float(value)));       \
        }                                                           \
        return hymn_new_exception(H, "argument must be a number");  \
    }                                                               \
    return hymn_new_exception(H, "missing number");

static HymnValue math_floor(Hymn *H, int count, HymnValue *arguments) {
    MATH_FUNCTION(floor)
}

static HymnValue math_ceil(Hymn *H, int count, HymnValue *arguments) {
    MATH_FUNCTION(ceil)
}

static HymnValue math_sin(Hymn *H, int count, HymnValue *arguments) {
    MATH_FUNCTION(sin)
}

static HymnValue math_cos(Hymn *H, int count, HymnValue *arguments) {
    MATH_FUNCTION(cos)
}

static HymnValue math_tan(Hymn *H, int count, HymnValue *arguments) {
    MATH_FUNCTION(tan)
}

static HymnValue math_asin(Hymn *H, int count, HymnValue *arguments) {
    MATH_FUNCTION(asin)
}

static HymnValue math_acos(Hymn *H, int count, HymnValue *arguments) {
    MATH_FUNCTION(acos)
}

static HymnValue math_sinh(Hymn *H, int count, HymnValue *arguments) {
    MATH_FUNCTION(sinh)
}

static HymnValue math_cosh(Hymn *H, int count, HymnValue *arguments) {
    MATH_FUNCTION(cosh)
}

static HymnValue math_atan(Hymn *H, int count, HymnValue *arguments) {
    MATH_FUNCTION(atan)
}

static HymnValue math_atan2(Hymn *H, int count, HymnValue *arguments) {
    if (count >= 2) {
        HymnValue a = arguments[0];
        HymnValue b = arguments[1];
        if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                hymn_new_float(atan2((double)hymn_as_int(a), (double)hymn_as_int(b)));
            } else if (hymn_is_float(b)) {
                hymn_new_float(atan2((double)hymn_as_int(a), hymn_as_float(b)));
            }
        } else if (hymn_is_float(a)) {
            if (hymn_is_int(b)) {
                hymn_new_float(atan2(hymn_as_float(a), (double)hymn_as_int(b)));
            } else if (hymn_is_float(b)) {
                hymn_new_float(atan2(hymn_as_float(a), hymn_as_float(b)));
            }
        }
        return hymn_new_exception(H, "arguments must be numbers");
    }
    return hymn_new_exception(H, "missing numbers");
}

static HymnValue math_sqrt(Hymn *H, int count, HymnValue *arguments) {
    MATH_FUNCTION(sqrt)
}

static HymnValue math_pow(Hymn *H, int count, HymnValue *arguments) {
    if (count >= 2) {
        HymnValue a = arguments[0];
        HymnValue b = arguments[1];
        if (hymn_is_int(a)) {
            if (hymn_is_int(b)) {
                hymn_new_float(pow((double)hymn_as_int(a), (double)hymn_as_int(b)));
            } else if (hymn_is_float(b)) {
                hymn_new_float(pow((double)hymn_as_int(a), hymn_as_float(b)));
            }
        } else if (hymn_is_float(a)) {
            if (hymn_is_int(b)) {
                hymn_new_float(pow(hymn_as_float(a), (double)hymn_as_int(b)));
            } else if (hymn_is_float(b)) {
                hymn_new_float(pow(hymn_as_float(a), hymn_as_float(b)));
            }
        }
        return hymn_new_exception(H, "arguments must be numbers");
    }
    return hymn_new_exception(H, "missing numbers");
}

static HymnValue math_log(Hymn *H, int count, HymnValue *arguments) {
    MATH_FUNCTION(log)
}

static HymnValue math_log2(Hymn *H, int count, HymnValue *arguments) {
    MATH_FUNCTION(log2)
}

static HymnValue math_log10(Hymn *H, int count, HymnValue *arguments) {
    MATH_FUNCTION(log10)
}

void hymn_use_math(Hymn *H) {
    HymnTable *math = hymn_new_table();
    hymn_add_function_to_table(H, math, "abs", math_abs);
    hymn_add_function_to_table(H, math, "min", math_min);
    hymn_add_function_to_table(H, math, "max", math_max);
    hymn_add_function_to_table(H, math, "floor", math_floor);
    hymn_add_function_to_table(H, math, "ceil", math_ceil);
    hymn_add_function_to_table(H, math, "sin", math_sin);
    hymn_add_function_to_table(H, math, "cos", math_cos);
    hymn_add_function_to_table(H, math, "tan", math_tan);
    hymn_add_function_to_table(H, math, "asin", math_asin);
    hymn_add_function_to_table(H, math, "acos", math_acos);
    hymn_add_function_to_table(H, math, "sinh", math_sinh);
    hymn_add_function_to_table(H, math, "cosh", math_cosh);
    hymn_add_function_to_table(H, math, "atan", math_atan);
    hymn_add_function_to_table(H, math, "atan2", math_atan2);
    hymn_add_function_to_table(H, math, "sqrt", math_sqrt);
    hymn_add_function_to_table(H, math, "pow", math_pow);
    hymn_add_function_to_table(H, math, "log", math_log);
    hymn_add_function_to_table(H, math, "log2", math_log2);
    hymn_add_function_to_table(H, math, "log10", math_log10);
    hymn_add_table(H, "math", math);

    hymn_add(H, "PI", hymn_new_float(PI));
}
