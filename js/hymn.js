/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

const HYMN_DEBUG_TRACE = false
const HYMN_DEBUG_STACK = false

const UINT8_MAX = 255
const UINT16_MAX = 65535
const HYMN_UINT8_COUNT = UINT8_MAX + 1
const HYMN_FRAMES_MAX = 64

const HYMN_VALUE_NONE = 1
const HYMN_VALUE_BOOL = 2
const HYMN_VALUE_INTEGER = 3
const HYMN_VALUE_FLOAT = 4
const HYMN_VALUE_STRING = 5
const HYMN_VALUE_ARRAY = 6
const HYMN_VALUE_TABLE = 7
const HYMN_VALUE_FUNC = 8
const HYMN_VALUE_FUNC_NATIVE = 9
const HYMN_VALUE_POINTER = 10

const node = typeof window === 'undefined'
const node_fs = node ? require('fs') : null
const node_path = node ? require('path') : null

const LOAD_FACTOR = 0.8
const INITIAL_BINS = 1 << 3
const MAXIMUM_BINS = 1 << 30

class HymnValue {
  constructor(is, value) {
    this.is = is
    this.value = value
  }
}

function copyValueToFrom(to, from) {
  to.is = from.is
  to.value = from.value
}

function cloneValue(original) {
  const value = new HymnValue()
  copyValueToFrom(value, original)
  return value
}

class HymnTableItem {
  constructor(hash, key, value) {
    this.hash = hash
    this.key = key
    this.value = value
    this.next = null
  }
}

class HymnTable {
  constructor() {
    this.size = 0
    this.bins = INITIAL_BINS
    this.items = new Array(this.bins).fill(null)
  }
}

class HymnNativeFunction {
  constructor(name, func) {
    this.name = name
    this.func = func
  }
}

class HymnByteCode {
  constructor() {
    this.count = 0
    this.instructions = new Uint8Array(128)
    this.lines = new Uint32Array(128)
    this.constants = []
  }
}

class ExceptList {
  constructor() {
    this.start = 0
    this.end = 0
    this.stack = 0
    this.next = null
  }
}

class HymnFunction {
  constructor() {
    this.count = 0
    this.name = null
    this.script = null
    this.arity = 0
    this.code = null
    this.except = null
  }
}

class HymnFrame {
  constructor() {
    this.func = null
    this.ip = 0
    this.stack = 0
  }
}

class Instruction {
  constructor(index, instruction) {
    this.index = index
    this.instruction = instruction
  }
}

function printOut(text) {
  console.log(text)
}

class Hymn {
  constructor() {
    this.stack = []
    this.stackTop = 0
    this.frames = []
    this.frameCount = 0
    this.globals = new HymnTable()
    this.paths = []
    this.imports = new HymnTable()
    this.error = null
    this.print = printOut
  }
}

const STRING_NONE = 'None'
const STRING_BOOL = 'Bool'
const STRING_TRUE = 'True'
const STRING_FALSE = 'False'
const STRING_INTEGER = 'Integer'
const STRING_FLOAT = 'Float'
const STRING_STRING = 'String'
const STRING_ARRAY = 'Array'
const STRING_TABLE = 'Table'
const STRING_FUNC = 'Function'
const STRING_NATIVE = 'Native'
const STRING_POINTER = 'Pointer'

const TOKEN_ADD = 0
const TOKEN_AND = 1
const TOKEN_ASSIGN = 2
const TOKEN_ASSIGN_ADD = 3
const TOKEN_ASSIGN_BIT_AND = 4
const TOKEN_ASSIGN_BIT_LEFT_SHIFT = 5
const TOKEN_ASSIGN_BIT_OR = 6
const TOKEN_ASSIGN_BIT_RIGHT_SHIFT = 7
const TOKEN_ASSIGN_BIT_XOR = 8
const TOKEN_ASSIGN_DIVIDE = 9
const TOKEN_ASSIGN_MODULO = 10
const TOKEN_ASSIGN_MULTIPLY = 11
const TOKEN_ASSIGN_SUBTRACT = 12
const TOKEN_BEGIN = 13
const TOKEN_BIT_AND = 14
const TOKEN_BIT_LEFT_SHIFT = 15
const TOKEN_BIT_NOT = 16
const TOKEN_BIT_OR = 17
const TOKEN_BIT_RIGHT_SHIFT = 18
const TOKEN_BIT_XOR = 19
const TOKEN_BREAK = 20
const TOKEN_CLEAR = 21
const TOKEN_COLON = 22
const TOKEN_COMMA = 23
const TOKEN_CONTINUE = 24
const TOKEN_COPY = 25
const TOKEN_DELETE = 26
const TOKEN_DIVIDE = 27
const TOKEN_DOT = 28
const TOKEN_ELIF = 29
const TOKEN_ELSE = 30
const TOKEN_END = 31
const TOKEN_EOF = 32
const TOKEN_EQUAL = 33
const TOKEN_ERROR = 34
const TOKEN_EXCEPT = 35
const TOKEN_FALSE = 36
const TOKEN_FLOAT = 37
const TOKEN_FOR = 38
const TOKEN_FUNCTION = 39
const TOKEN_GREATER = 40
const TOKEN_GREATER_EQUAL = 41
const TOKEN_IDENT = 42
const TOKEN_IF = 43
const TOKEN_IN = 44
const TOKEN_INDEX = 45
const TOKEN_INSERT = 46
const TOKEN_INTEGER = 47
const TOKEN_KEYS = 48
const TOKEN_LEFT_CURLY = 49
const TOKEN_LEFT_PAREN = 50
const TOKEN_LEFT_SQUARE = 51
const TOKEN_LEN = 52
const TOKEN_LESS = 53
const TOKEN_LESS_EQUAL = 54
const TOKEN_LET = 55
const TOKEN_LINE = 56
const TOKEN_MODULO = 57
const TOKEN_MULTIPLY = 58
const TOKEN_NONE = 59
const TOKEN_NOT = 60
const TOKEN_NOT_EQUAL = 61
const TOKEN_OR = 62
const TOKEN_POP = 63
const TOKEN_PRINT = 64
const TOKEN_PUSH = 65
const TOKEN_RETURN = 66
const TOKEN_RIGHT_CURLY = 67
const TOKEN_RIGHT_PAREN = 68
const TOKEN_RIGHT_SQUARE = 69
const TOKEN_SEMICOLON = 70
const TOKEN_STRING = 71
const TOKEN_SUBTRACT = 72
const TOKEN_THROW = 73
const TOKEN_TO_FLOAT = 74
const TOKEN_TO_INTEGER = 75
const TOKEN_TO_STRING = 76
const TOKEN_TRUE = 77
const TOKEN_TRY = 78
const TOKEN_TYPE_FUNC = 79
const TOKEN_UNDEFINED = 80
const TOKEN_USE = 81
const TOKEN_VALUE = 82
const TOKEN_WHILE = 83

const PRECEDENCE_NONE = 0
const PRECEDENCE_ASSIGN = 1
const PRECEDENCE_BITS = 2
const PRECEDENCE_OR = 3
const PRECEDENCE_AND = 4
const PRECEDENCE_EQUALITY = 5
const PRECEDENCE_COMPARE = 6
const PRECEDENCE_TERM = 7
const PRECEDENCE_FACTOR = 8
const PRECEDENCE_UNARY = 9
const PRECEDENCE_CALL = 10

const OP_ADD = 0
const OP_ADD_TWO_LOCAL = 1
const OP_INCREMENT = 2
const OP_ARRAY_INSERT = 3
const OP_ARRAY_POP = 4
const OP_ARRAY_PUSH = 5
const OP_BIT_AND = 6
const OP_BIT_LEFT_SHIFT = 7
const OP_BIT_NOT = 8
const OP_BIT_OR = 9
const OP_BIT_RIGHT_SHIFT = 10
const OP_BIT_XOR = 11
const OP_CALL = 12
const OP_TAIL_CALL = 13
const OP_CLEAR = 14
const OP_CONSTANT = 15
const OP_COPY = 16
const OP_DEFINE_GLOBAL = 17
const OP_DELETE = 18
const OP_DIVIDE = 19
const OP_DUPLICATE = 20
const OP_EQUAL = 21
const OP_FALSE = 22
const OP_GET_DYNAMIC = 23
const OP_GET_GLOBAL = 24
const OP_GET_LOCAL = 25
const OP_GET_TWO_LOCAL = 26
const OP_GET_PROPERTY = 27
const OP_GREATER = 28
const OP_GREATER_EQUAL = 29
const OP_INDEX = 30
const OP_JUMP = 31
const OP_JUMP_IF_EQUAL = 32
const OP_JUMP_IF_NOT_EQUAL = 33
const OP_JUMP_IF_LESS = 34
const OP_JUMP_IF_GREATER = 35
const OP_JUMP_IF_LESS_EQUAL = 36
const OP_JUMP_IF_GREATER_EQUAL = 37
const OP_JUMP_IF_FALSE = 38
const OP_JUMP_IF_TRUE = 39
const OP_KEYS = 40
const OP_LEN = 41
const OP_LESS = 42
const OP_LESS_EQUAL = 43
const OP_LOOP = 44
const OP_MODULO = 45
const OP_MULTIPLY = 46
const OP_NEGATE = 47
const OP_NONE = 48
const OP_NOT = 49
const OP_NOT_EQUAL = 50
const OP_POP = 51
const OP_POP_TWO = 52
const OP_POP_N = 53
const OP_PRINT = 54
const OP_RETURN = 55
const OP_SET_DYNAMIC = 56
const OP_SET_GLOBAL = 57
const OP_SET_LOCAL = 58
const OP_INCREMENT_LOCAL = 59
const OP_INCREMENT_LOCAL_AND_SET = 60
const OP_SET_PROPERTY = 61
const OP_SLICE = 62
const OP_SUBTRACT = 63
const OP_THROW = 64
const OP_TO_FLOAT = 65
const OP_TO_INTEGER = 66
const OP_TO_STRING = 67
const OP_TRUE = 68
const OP_TYPE = 69
const OP_USE = 70
const OP_FOR = 71
const OP_FOR_LOOP = 72

const TYPE_FUNCTION = 0
const TYPE_SCRIPT = 1

class Token {
  constructor() {
    this.type = TOKEN_UNDEFINED
    this.row = 0
    this.column = 0
    this.start = 0
    this.len = 0
  }
}

function copyToken(dest, src) {
  dest.type = src.type
  dest.row = src.row
  dest.column = src.column
  dest.start = src.start
  dest.len = src.len
}

class Local {
  constructor() {
    this.name = null
    this.depth = 0
  }
}

class Rule {
  constructor(prefix, infix, precedence) {
    this.prefix = prefix
    this.infix = infix
    this.precedence = precedence
  }
}

class Scope {
  constructor() {
    this.enclosing = null
    this.func = null
    this.type = TYPE_FUNCTION
    this.locals = []
    this.localCount = 0
    this.depth = 0
  }
}

class Compiler {
  constructor(script, source, H) {
    this.pos = 0
    this.row = 1
    this.column = 1
    this.script = script
    this.source = source
    this.previous = new Token()
    this.current = new Token()
    this.H = H
    this.scope = null
    this.loop = null
    this.jump = null
    this.jumpOr = null
    this.jumpAnd = null
    this.jumpFor = null
    this.error = null
  }
}

class JumpList {
  constructor() {
    this.jump = 0
    this.depth = 0
    this.next = null
  }
}

class LoopList {
  constructor(start, depth, next, isFor) {
    this.start = start
    this.depth = depth
    this.next = next
    this.isFor = isFor
  }
}

const rules = []
rules[TOKEN_ADD] = new Rule(null, compileBinary, PRECEDENCE_TERM)
rules[TOKEN_AND] = new Rule(null, compileAnd, PRECEDENCE_AND)
rules[TOKEN_ASSIGN] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_ASSIGN_ADD] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_ASSIGN_BIT_AND] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_ASSIGN_BIT_LEFT_SHIFT] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_ASSIGN_BIT_OR] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_ASSIGN_BIT_RIGHT_SHIFT] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_ASSIGN_BIT_XOR] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_ASSIGN_DIVIDE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_ASSIGN_MODULO] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_ASSIGN_MULTIPLY] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_ASSIGN_SUBTRACT] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_BEGIN] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_BIT_AND] = new Rule(null, compileBinary, PRECEDENCE_BITS)
rules[TOKEN_BIT_LEFT_SHIFT] = new Rule(null, compileBinary, PRECEDENCE_BITS)
rules[TOKEN_BIT_NOT] = new Rule(compileUnary, null, PRECEDENCE_NONE)
rules[TOKEN_BIT_OR] = new Rule(null, compileBinary, PRECEDENCE_BITS)
rules[TOKEN_BIT_RIGHT_SHIFT] = new Rule(null, compileBinary, PRECEDENCE_BITS)
rules[TOKEN_BIT_XOR] = new Rule(null, compileBinary, PRECEDENCE_BITS)
rules[TOKEN_BREAK] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_COLON] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_CLEAR] = new Rule(clearExpression, null, PRECEDENCE_NONE)
rules[TOKEN_COMMA] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_CONTINUE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_COPY] = new Rule(copyExpression, null, PRECEDENCE_NONE)
rules[TOKEN_DELETE] = new Rule(deleteExpression, null, PRECEDENCE_NONE)
rules[TOKEN_DIVIDE] = new Rule(null, compileBinary, PRECEDENCE_FACTOR)
rules[TOKEN_DOT] = new Rule(null, compileDot, PRECEDENCE_CALL)
rules[TOKEN_ELIF] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_ELSE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_END] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_EOF] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_EQUAL] = new Rule(null, compileBinary, PRECEDENCE_EQUALITY)
rules[TOKEN_ERROR] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_EXCEPT] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_FALSE] = new Rule(compileFalse, null, PRECEDENCE_NONE)
rules[TOKEN_FLOAT] = new Rule(compileFloat, null, PRECEDENCE_NONE)
rules[TOKEN_FOR] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_FUNCTION] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_GREATER] = new Rule(null, compileBinary, PRECEDENCE_COMPARE)
rules[TOKEN_GREATER_EQUAL] = new Rule(null, compileBinary, PRECEDENCE_COMPARE)
rules[TOKEN_IDENT] = new Rule(compileVariable, null, PRECEDENCE_NONE)
rules[TOKEN_IF] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_IN] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_INDEX] = new Rule(indexExpression, null, PRECEDENCE_NONE)
rules[TOKEN_INSERT] = new Rule(arrayInsertExpression, null, PRECEDENCE_NONE)
rules[TOKEN_INTEGER] = new Rule(compileInteger, null, PRECEDENCE_NONE)
rules[TOKEN_KEYS] = new Rule(keysExpression, null, PRECEDENCE_NONE)
rules[TOKEN_LEFT_CURLY] = new Rule(compileTable, null, PRECEDENCE_NONE)
rules[TOKEN_LEFT_PAREN] = new Rule(compileGroup, compileCall, PRECEDENCE_CALL)
rules[TOKEN_LEFT_SQUARE] = new Rule(compileArray, compileSquare, PRECEDENCE_CALL)
rules[TOKEN_LEN] = new Rule(lenExpression, null, PRECEDENCE_NONE)
rules[TOKEN_LESS] = new Rule(null, compileBinary, PRECEDENCE_COMPARE)
rules[TOKEN_LESS_EQUAL] = new Rule(null, compileBinary, PRECEDENCE_COMPARE)
rules[TOKEN_LET] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_LINE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_MODULO] = new Rule(null, compileBinary, PRECEDENCE_FACTOR)
rules[TOKEN_MULTIPLY] = new Rule(null, compileBinary, PRECEDENCE_FACTOR)
rules[TOKEN_NONE] = new Rule(compileNone, null, PRECEDENCE_NONE)
rules[TOKEN_NOT] = new Rule(compileUnary, null, PRECEDENCE_NONE)
rules[TOKEN_NOT_EQUAL] = new Rule(null, compileBinary, PRECEDENCE_EQUALITY)
rules[TOKEN_OR] = new Rule(null, compileOr, PRECEDENCE_OR)
rules[TOKEN_POP] = new Rule(arrayPopExpression, null, PRECEDENCE_NONE)
rules[TOKEN_PRINT] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_PUSH] = new Rule(arrayPushExpression, null, PRECEDENCE_NONE)
rules[TOKEN_RETURN] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_RIGHT_CURLY] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_RIGHT_PAREN] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_RIGHT_SQUARE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_STRING] = new Rule(compileString, null, PRECEDENCE_NONE)
rules[TOKEN_SUBTRACT] = new Rule(compileUnary, compileBinary, PRECEDENCE_TERM)
rules[TOKEN_TO_FLOAT] = new Rule(castFloatExpression, null, PRECEDENCE_NONE)
rules[TOKEN_TO_INTEGER] = new Rule(castIntegerExpression, null, PRECEDENCE_NONE)
rules[TOKEN_TO_STRING] = new Rule(castStringExpression, null, PRECEDENCE_NONE)
rules[TOKEN_TRUE] = new Rule(compileTrue, null, PRECEDENCE_NONE)
rules[TOKEN_TRY] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_THROW] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_TYPE_FUNC] = new Rule(typeExpression, null, PRECEDENCE_NONE)
rules[TOKEN_UNDEFINED] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_USE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_VALUE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_WHILE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_SEMICOLON] = new Rule(null, null, PRECEDENCE_NONE)

function valueName(type) {
  switch (type) {
    case HYMN_VALUE_NONE:
      return STRING_NONE
    case HYMN_VALUE_BOOL:
      return STRING_BOOL
    case HYMN_VALUE_INTEGER:
      return STRING_INTEGER
    case HYMN_VALUE_FLOAT:
      return STRING_FLOAT
    case HYMN_VALUE_STRING:
      return STRING_STRING
    case HYMN_VALUE_ARRAY:
      return STRING_ARRAY
    case HYMN_VALUE_FUNC:
      return STRING_FUNC
    case HYMN_VALUE_FUNC_NATIVE:
      return STRING_NATIVE
    default:
      return '?'
  }
}

function stringMixHashCode(key) {
  const length = key.length
  let hash = 0
  for (let i = 0; i < length; i++) {
    hash = 31 * hash + key.charCodeAt(i)
    hash |= 0
  }
  return hash ^ (hash >> 16)
}

function tableGetBin(table, hash) {
  return (table.bins - 1) & hash
}

function tableResize(table) {
  const binsOld = table.bins
  const bins = binsOld << 1

  if (bins > MAXIMUM_BINS) return

  const itemsOld = table.items
  const items = new Array(bins).fill(null)

  for (let i = 0; i < binsOld; i++) {
    let item = itemsOld[i]
    if (item === null) continue
    if (item.next === null) {
      items[(bins - 1) & item.hash] = item
    } else {
      let lowHead = null
      let lowTail = null
      let highHead = null
      let highTail = null
      do {
        if ((binsOld & item.hash) === 0) {
          if (lowTail === null) lowHead = item
          else lowTail.next = item
          lowTail = item
        } else {
          if (highTail === null) highHead = item
          else highTail.next = item
          highTail = item
        }
        item = item.next
      } while (item !== null)

      if (lowTail !== null) {
        lowTail.next = null
        items[i] = lowHead
      }

      if (highTail !== null) {
        highTail.next = null
        items[i + binsOld] = highHead
      }
    }
  }

  table.bins = bins
  table.items = items
}

function tablePut(table, key, value) {
  const hash = stringMixHashCode(key)
  const bin = tableGetBin(table, hash)
  let item = table.items[bin]
  let previous = null
  while (item !== null) {
    if (key === item.key) {
      const old = item.value
      item.value = value
      return old
    }
    previous = item
    item = item.next
  }
  item = new HymnTableItem(hash, key, value)
  if (previous === null) table.items[bin] = item
  else previous.next = item
  table.size++
  if (table.size > table.bins * LOAD_FACTOR) tableResize(table)
  return null
}

function tableGet(table, key) {
  const hash = stringMixHashCode(key)
  const bin = tableGetBin(table, hash)
  let item = table.items[bin]
  while (item !== null) {
    if (key === item.key) return item.value
    item = item.next
  }
  return null
}

function tableNext(table, key) {
  const bins = table.bins
  if (key === null) {
    for (let i = 0; i < bins; i++) {
      const item = table.items[i]
      if (item !== null) return item
    }
    return null
  }
  const hash = stringMixHashCode(key)
  const bin = tableGetBin(table, hash)
  {
    let item = table.items[bin]
    while (item !== null) {
      const next = item.next
      if (key === item.key) {
        if (next !== null) return next
      }
      item = next
    }
  }
  for (let i = bin + 1; i < bins; i++) {
    const item = table.items[i]
    if (item !== null) return item
  }
  return null
}

function tableRemove(table, key) {
  const hash = stringMixHashCode(key)
  const bin = tableGetBin(table, hash)
  let item = table.items[bin]
  let previous = null
  while (item !== null) {
    if (key === item.key) {
      if (previous === null) table.items[bin] = item.next
      else previous.next = item.next
      const value = item.value
      table.size--
      return value
    }
    previous = item
    item = item.next
  }
  return null
}

function tableClear(table) {
  table.size = 0
  const bins = table.bins
  for (let i = 0; i < bins; i++) {
    let item = table.items[i]
    while (item !== null) {
      const next = item.next
      item.next = null
      item = next
    }
    table.items[i] = null
  }
}

function newTableCopy(from) {
  const copy = new HymnTable()
  const bins = from.bins
  for (let i = 0; i < bins; i++) {
    let item = from.items[i]
    while (item !== null) {
      tablePut(copy, item.key, cloneValue(item.value))
      item = item.next
    }
  }
  return copy
}

function stringCompare(a, b) {
  return a === b ? 0 : a > b ? 1 : -1
}

function tableKeys(table) {
  const size = table.size
  const keys = new Array(size)
  if (size === 0) return keys
  let total = 0
  const bins = table.bins
  for (let i = 0; i < bins; i++) {
    let item = table.items[i]
    while (item !== null) {
      const key = item.key
      let insert = 0
      while (insert !== total) {
        if (stringCompare(key, keys[insert].value) < 0) {
          for (let swap = total; swap > insert; swap--) {
            keys[swap] = keys[swap - 1]
          }
          break
        }
        insert++
      }
      keys[insert] = newString(item.key)
      total++
      item = item.next
    }
  }
  return keys
}

function tableKeyOf(table, input) {
  let bin = 0
  let item = null

  const bins = table.bins
  for (let i = 0; i < bins; i++) {
    const start = table.items[i]
    if (start) {
      bin = i
      item = start
      break
    }
  }

  if (item === null) return null
  if (matchValues(input, item.value)) return item.key

  while (true) {
    item = item.next
    if (item === null) {
      for (bin = bin + 1; bin < bins; bin++) {
        const start = table.items[bin]
        if (start) {
          item = start
          break
        }
      }
      if (item === null) return null
    }
    if (matchValues(input, item.value)) return item.key
  }
}

function sourceSubstring(C, len, start) {
  return C.source.substring(start, start + len)
}

function newNone() {
  return new HymnValue(HYMN_VALUE_NONE, null)
}

function newBool(boolean) {
  return new HymnValue(HYMN_VALUE_BOOL, boolean)
}

function newInt(number) {
  return new HymnValue(HYMN_VALUE_INTEGER, number)
}

function newFloat(number) {
  return new HymnValue(HYMN_VALUE_FLOAT, number)
}

function newString(string) {
  return new HymnValue(HYMN_VALUE_STRING, string)
}

function newArrayValue(array) {
  return new HymnValue(HYMN_VALUE_ARRAY, array)
}

function newTableValue(table) {
  return new HymnValue(HYMN_VALUE_TABLE, table)
}

function newFuncValue(func) {
  return new HymnValue(HYMN_VALUE_FUNC, func)
}

function newFuncNativeValue(func) {
  return new HymnValue(HYMN_VALUE_FUNC_NATIVE, func)
}

function newPointerValue(pointer) {
  return new HymnValue(HYMN_VALUE_POINTER, pointer)
}

function newFunction(script) {
  const func = new HymnFunction()
  func.code = new HymnByteCode()
  if (script) func.script = script
  return func
}

function newNativeFunction(name, func) {
  return new HymnNativeFunction(name, func)
}

function isNone(value) {
  return value.is === HYMN_VALUE_NONE
}

function isBool(value) {
  return value.is === HYMN_VALUE_BOOL
}

function isInt(value) {
  return value.is === HYMN_VALUE_INTEGER
}

function isFloat(value) {
  return value.is === HYMN_VALUE_FLOAT
}

function isString(value) {
  return value.is === HYMN_VALUE_STRING
}

function isArray(value) {
  return value.is === HYMN_VALUE_ARRAY
}

function isTable(value) {
  return value.is === HYMN_VALUE_TABLE
}

function isFunc(value) {
  return value.is === HYMN_VALUE_FUNC
}

function isFuncNative(value) {
  return value.is === HYMN_VALUE_FUNC_NATIVE
}

function isPointer(value) {
  return value.is === HYMN_VALUE_POINTER
}

function currentFunc(C) {
  return C.scope.func
}

function current(C) {
  return C.scope.func.code
}

function compileError(C, token, format) {
  if (C.error !== null) return

  const error = format

  C.error = error

  C.previous.type = TOKEN_EOF
  C.current.type = TOKEN_EOF
}

function nextChar(C) {
  const pos = C.pos
  if (pos === C.source.length) return '\0'
  const c = C.source[pos]
  C.pos = pos + 1
  if (c === '\n') {
    C.row++
    C.column = 0
  } else {
    C.column++
  }
  return c
}

function peekChar(C) {
  if (C.pos === C.source.length) return '\0'
  return C.source[C.pos]
}

function token(C, type) {
  const token = C.current
  token.type = type
  token.row = C.row
  token.column = C.column
  token.start = C.pos - 1
  token.len = 1
}

function tokenSpecial(C, type, offset, len) {
  const token = C.current
  token.type = type
  token.row = C.row
  token.column = C.column
  token.start = C.pos - offset
  token.len = len
}

function valueToken(C, type, start, end) {
  const token = C.current
  token.type = type
  token.row = C.row
  token.column = C.column
  token.start = start
  token.len = end - start
}

function identTrie(ident, offset, rest, type) {
  for (let i = 0; i < rest.length; i++) {
    if (ident[offset + i] !== rest[i]) {
      return TOKEN_UNDEFINED
    }
  }
  return type
}

function identKey(ident, size) {
  switch (ident[0]) {
    case 'o':
      if (size === 2) return identTrie(ident, 1, 'r', TOKEN_OR)
      break
    case 'u':
      if (size === 3) return identTrie(ident, 1, 'se', TOKEN_USE)
      break
    case 'a':
      if (size === 3) return identTrie(ident, 1, 'nd', TOKEN_AND)
      break
    case 'n':
      if (size === 4) return identTrie(ident, 1, 'one', TOKEN_NONE)
      break
    case 'w':
      if (size === 5) return identTrie(ident, 1, 'hile', TOKEN_WHILE)
      break
    case 'b':
      if (size === 5) {
        if (ident[1] === 'e') return identTrie(ident, 2, 'gin', TOKEN_BEGIN)
        if (ident[1] === 'r') return identTrie(ident, 2, 'eak', TOKEN_BREAK)
      }
      break
    case 'd':
      if (size === 6) return identTrie(ident, 1, 'elete', TOKEN_DELETE)
      break
    case 'r':
      if (size === 6) return identTrie(ident, 1, 'eturn', TOKEN_RETURN)
      break
    case 's':
      if (size === 6) return identTrie(ident, 1, 'tring', TOKEN_TO_STRING)
      break
    case 'k':
      if (size === 4) return identTrie(ident, 1, 'eys', TOKEN_KEYS)
      break
    case 'c':
      if (size === 4) return identTrie(ident, 1, 'opy', TOKEN_COPY)
      if (size === 5) return identTrie(ident, 1, 'lear', TOKEN_CLEAR)
      if (size === 8) return identTrie(ident, 1, 'ontinue', TOKEN_CONTINUE)
      break
    case 'l':
      if (size === 3 && ident[1] === 'e') {
        if (ident[2] === 't') return TOKEN_LET
        if (ident[2] === 'n') return TOKEN_LEN
      }
      break
    case 't':
      if (size === 3) return identTrie(ident, 1, 'ry', TOKEN_TRY)
      if (size === 5) return identTrie(ident, 1, 'hrow', TOKEN_THROW)
      if (size === 4) {
        if (ident[1] === 'r') return identTrie(ident, 2, 'ue', TOKEN_TRUE)
        if (ident[1] === 'y') return identTrie(ident, 2, 'pe', TOKEN_TYPE_FUNC)
      }
      break
    case 'i':
      if (size === 3) return identTrie(ident, 1, 'nt', TOKEN_TO_INTEGER)
      if (size === 5) return identTrie(ident, 1, 'ndex', TOKEN_INDEX)
      if (size === 6) return identTrie(ident, 1, 'nsert', TOKEN_INSERT)
      if (size === 2) {
        if (ident[1] === 'f') return TOKEN_IF
        if (ident[1] === 'n') return TOKEN_IN
      }
      break
    case 'p':
      if (size === 3) return identTrie(ident, 1, 'op', TOKEN_POP)
      if (size === 5) return identTrie(ident, 1, 'rint', TOKEN_PRINT)
      if (size === 4) return identTrie(ident, 1, 'ush', TOKEN_PUSH)
      break
    case 'e':
      if (size === 3) return identTrie(ident, 1, 'nd', TOKEN_END)
      if (size === 6) return identTrie(ident, 1, 'xcept', TOKEN_EXCEPT)
      if (size === 4 && ident[1] === 'l') {
        if (ident[2] === 's') {
          if (ident[3] === 'e') {
            return TOKEN_ELSE
          }
        } else if (ident[2] === 'i' && ident[3] === 'f') {
          return TOKEN_ELIF
        }
      }
      break
    case 'f':
      if (size === 3) return identTrie(ident, 1, 'or', TOKEN_FOR)
      if (size === 8) return identTrie(ident, 1, 'unction', TOKEN_FUNCTION)
      if (size === 5) {
        if (ident[1] === 'a') return identTrie(ident, 2, 'lse', TOKEN_FALSE)
        if (ident[1] === 'l') return identTrie(ident, 2, 'oat', TOKEN_TO_FLOAT)
      }
      break
  }
  return TOKEN_UNDEFINED
}

function pushIdentToken(C, start, end) {
  const ident = C.source.substring(start, end)
  const keyword = identKey(ident, end - start)
  if (keyword !== TOKEN_UNDEFINED) {
    valueToken(C, keyword, start, end)
  } else {
    valueToken(C, TOKEN_IDENT, start, end)
  }
}

function isDigit(c) {
  return c >= '0' && c <= '9'
}

function isIdent(c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c === '_'
}

function advance(C) {
  copyToken(C.previous, C.current)
  if (C.previous.type === TOKEN_EOF) {
    return
  }
  while (true) {
    let c = nextChar(C)
    switch (c) {
      case ' ':
      case '\t':
      case '\r':
      case '\n':
        c = peekChar(C)
        while (c !== '\0' && (c === ' ' || c === '\t' || c === '\r' || c === '\n')) {
          nextChar(C)
          c = peekChar(C)
        }
        continue
      case '!':
        if (peekChar(C) === '=') {
          nextChar(C)
          tokenSpecial(C, TOKEN_NOT_EQUAL, 2, 2)
        } else {
          token(C, TOKEN_NOT)
        }
        return
      case '=':
        if (peekChar(C) === '=') {
          nextChar(C)
          tokenSpecial(C, TOKEN_EQUAL, 2, 2)
        } else {
          token(C, TOKEN_ASSIGN)
        }
        return
      case '-': {
        if (peekChar(C) === '-') {
          nextChar(C)
          c = peekChar(C)
          while (c !== '\n' && c !== '\0') {
            nextChar(C)
            c = peekChar(C)
          }
          continue
        } else if (peekChar(C) === '=') {
          nextChar(C)
          tokenSpecial(C, TOKEN_ASSIGN_SUBTRACT, 2, 2)
          return
        } else {
          token(C, TOKEN_SUBTRACT)
          return
        }
      }
      case '+':
        if (peekChar(C) === '=') {
          nextChar(C)
          tokenSpecial(C, TOKEN_ASSIGN_ADD, 2, 2)
        } else {
          token(C, TOKEN_ADD)
        }
        return
      case '*':
        if (peekChar(C) === '=') {
          nextChar(C)
          tokenSpecial(C, TOKEN_ASSIGN_MULTIPLY, 2, 2)
        } else {
          token(C, TOKEN_MULTIPLY)
        }
        return
      case '/':
        if (peekChar(C) === '=') {
          nextChar(C)
          tokenSpecial(C, TOKEN_ASSIGN_DIVIDE, 2, 2)
        } else {
          token(C, TOKEN_DIVIDE)
        }
        return
      case '%':
        if (peekChar(C) === '=') {
          nextChar(C)
          tokenSpecial(C, TOKEN_ASSIGN_MODULO, 2, 2)
        } else {
          token(C, TOKEN_MODULO)
        }
        return
      case '&':
        if (peekChar(C) === '=') {
          nextChar(C)
          tokenSpecial(C, TOKEN_ASSIGN_BIT_AND, 2, 2)
        } else {
          token(C, TOKEN_BIT_AND)
        }
        return
      case '|':
        if (peekChar(C) === '=') {
          nextChar(C)
          tokenSpecial(C, TOKEN_ASSIGN_BIT_OR, 2, 2)
        } else {
          token(C, TOKEN_BIT_OR)
        }
        return
      case '^':
        if (peekChar(C) === '=') {
          nextChar(C)
          tokenSpecial(C, TOKEN_ASSIGN_BIT_XOR, 2, 2)
        } else {
          token(C, TOKEN_BIT_XOR)
        }
        return
      case '>':
        if (peekChar(C) === '=') {
          nextChar(C)
          tokenSpecial(C, TOKEN_GREATER_EQUAL, 2, 2)
        } else if (peekChar(C) === '>') {
          nextChar(C)
          if (peekChar(C) === '=') {
            nextChar(C)
            tokenSpecial(C, TOKEN_ASSIGN_BIT_RIGHT_SHIFT, 2, 2)
          } else {
            tokenSpecial(C, TOKEN_BIT_RIGHT_SHIFT, 2, 2)
          }
        } else {
          token(C, TOKEN_GREATER)
        }
        return
      case '<':
        if (peekChar(C) === '=') {
          nextChar(C)
          tokenSpecial(C, TOKEN_LESS_EQUAL, 2, 2)
        } else if (peekChar(C) === '<') {
          nextChar(C)
          if (peekChar(C) === '=') {
            nextChar(C)
            tokenSpecial(C, TOKEN_ASSIGN_BIT_LEFT_SHIFT, 2, 2)
          } else {
            tokenSpecial(C, TOKEN_BIT_LEFT_SHIFT, 2, 2)
          }
        } else {
          token(C, TOKEN_LESS)
        }
        return
      case '~':
        token(C, TOKEN_BIT_NOT)
        return
      case ',':
        token(C, TOKEN_COMMA)
        return
      case '.':
        token(C, TOKEN_DOT)
        return
      case '(':
        token(C, TOKEN_LEFT_PAREN)
        return
      case ')':
        token(C, TOKEN_RIGHT_PAREN)
        return
      case '[':
        token(C, TOKEN_LEFT_SQUARE)
        return
      case ']':
        token(C, TOKEN_RIGHT_SQUARE)
        return
      case '{':
        token(C, TOKEN_LEFT_CURLY)
        return
      case '}':
        token(C, TOKEN_RIGHT_CURLY)
        return
      case ':':
        token(C, TOKEN_COLON)
        return
      case ';':
        token(C, TOKEN_SEMICOLON)
        return
      case '\0':
        token(C, TOKEN_EOF)
        return
      case '"': {
        const start = C.pos
        while (true) {
          c = nextChar(C)
          if (c === '\\') {
            nextChar(C)
            continue
          } else if (c === '"' || c === '\0') {
            break
          }
        }
        const end = C.pos - 1
        valueToken(C, TOKEN_STRING, start, end)
        return
      }
      case "'": {
        const start = C.pos
        while (true) {
          c = nextChar(C)
          if (c === '\\') {
            nextChar(C)
            continue
          } else if (c === "'" || c === '\0') {
            break
          }
        }
        const end = C.pos - 1
        valueToken(C, TOKEN_STRING, start, end)
        return
      }
      default: {
        if (isDigit(c)) {
          const start = C.pos - 1
          while (isDigit(peekChar(C))) {
            nextChar(C)
          }
          let discrete = true
          if (peekChar(C) === '.') {
            discrete = false
            nextChar(C)
            while (isDigit(peekChar(C))) {
              nextChar(C)
            }
          }
          const end = C.pos
          if (discrete) {
            valueToken(C, TOKEN_INTEGER, start, end)
          } else {
            valueToken(C, TOKEN_FLOAT, start, end)
          }
          return
        } else if (isIdent(c)) {
          const start = C.pos - 1
          while (isIdent(peekChar(C))) {
            nextChar(C)
          }
          const end = C.pos
          pushIdentToken(C, start, end)
          return
        } else {
          compileError(C, C.current, 'Unknown character: `' + c + '`')
        }
      }
    }
  }
}

function matchValues(a, b) {
  if (a.is !== b.is) {
    return false
  }
  switch (a.is) {
    case HYMN_VALUE_NONE:
      return true
    default:
      return a.value === b.value
  }
}

function scopeGetLocal(scope, index) {
  if (index < scope.locals.length) {
    return scope.locals[index]
  }
  const local = new Local()
  scope.locals.push(local)
  return local
}

function scopeInit(C, scope, type) {
  scope.enclosing = C.scope
  C.scope = scope

  scope.localCount = 0
  scope.depth = 0
  scope.func = newFunction(C.script)
  scope.type = type

  if (type !== TYPE_SCRIPT) {
    scope.func.name = sourceSubstring(C, C.previous.len, C.previous.start)
  }

  const local = scopeGetLocal(scope, scope.localCount++)
  local.depth = 0
  local.name = null
}

function newCompiler(script, source, H, scope) {
  const C = new Compiler(script, source, H)
  scopeInit(C, scope, TYPE_SCRIPT)
  return C
}

function byteCodeNewConstant(C, value) {
  const code = current(C)
  code.constants.push(value)
  let constant = code.constants.length - 1
  if (constant > UINT8_MAX) {
    compileError(C, C.previous, 'Too many constants.')
    constant = 0
  }
  return constant
}

function arrayIndexOf(array, input) {
  for (let i = 0; i < array.length; i++) {
    if (matchValues(input, array[i])) {
      return i
    }
  }
  return -1
}

function writeByte(code, byte, row) {
  const count = code.count
  const size = code.instructions.length
  if (count + 1 > size) {
    const instructions = new Uint8Array(size * 2)
    const lines = new Uint32Array(size * 2)
    for (let i = 0; i < size; i++) instructions[i] = code.instructions[i]
    for (let i = 0; i < size; i++) lines[i] = code.lines[i]
    code.instructions = instructions
    code.lines = lines
  }
  code.instructions[count] = byte
  code.lines[count] = row
  code.count = count + 1
}

function emit(C, i) {
  writeByte(current(C), i, C.previous.row)
}

function emitShort(C, i, b) {
  const row = C.previous.row
  const code = current(C)
  writeByte(code, i, row)
  writeByte(code, b, row)
}

function emitWord(C, i, b, n) {
  const row = C.previous.row
  const code = current(C)
  writeByte(code, i, row)
  writeByte(code, b, row)
  writeByte(code, n, row)
}

function emitConstant(C, value) {
  const constant = byteCodeNewConstant(C, value)
  emitShort(C, OP_CONSTANT, constant)
  return constant
}

function checkAssign(C) {
  switch (C.current.type) {
    case TOKEN_ASSIGN:
    case TOKEN_ASSIGN_ADD:
    case TOKEN_ASSIGN_BIT_AND:
    case TOKEN_ASSIGN_BIT_LEFT_SHIFT:
    case TOKEN_ASSIGN_BIT_OR:
    case TOKEN_ASSIGN_BIT_RIGHT_SHIFT:
    case TOKEN_ASSIGN_BIT_XOR:
    case TOKEN_ASSIGN_DIVIDE:
    case TOKEN_ASSIGN_MODULO:
    case TOKEN_ASSIGN_MULTIPLY:
    case TOKEN_ASSIGN_SUBTRACT:
      return true
    default:
      return false
  }
}

function check(C, type) {
  return C.current.type === type
}

function match(C, type) {
  if (!check(C, type)) {
    return false
  }
  advance(C)
  return true
}

function compileWithPrecedence(C, precedence) {
  advance(C)
  const rule = rules[C.previous.type]
  const prefix = rule.prefix
  if (prefix === null) {
    compileError(C, C.previous, 'Syntax Error: Expected expression following `' + sourceSubstring(C, C.previous.len, C.previous.start) + '`.')
    return
  }
  const assign = precedence <= PRECEDENCE_ASSIGN
  prefix(C, assign)
  while (precedence <= rules[C.current.type].precedence) {
    advance(C)
    const infix = rules[C.previous.type].infix
    if (infix === null) {
      compileError(C, C.previous, 'Expected infix.')
    }
    infix(C, assign)
  }
  if (assign && checkAssign(C)) {
    advance(C)
    compileError(C, C.current, 'Invalid assignment target.')
  }
}

function consume(C, type, error) {
  if (C.current.type === type) {
    advance(C)
    return
  }
  compileError(C, C.current, error)
}

function pushHiddenLocal(C) {
  const scope = C.scope
  if (scope.localCount === HYMN_UINT8_COUNT) {
    compileError(C, C.previous, 'Too many local variables in scope.')
    return 0
  }
  const index = scope.localCount++
  const local = scopeGetLocal(scope, index)
  local.name = null
  local.depth = scope.depth
  return index
}

function args(C) {
  let count = 0
  if (!check(C, TOKEN_RIGHT_PAREN)) {
    do {
      expression(C)
      if (count === UINT8_MAX) {
        compileError(C, C.previous, "Can't have more than 255 function arguments.")
        break
      }
      count++
    } while (match(C, TOKEN_COMMA))
  }
  consume(C, TOKEN_RIGHT_PAREN, "Expected ')' after function arguments.")
  return count
}

function compileCall(C) {
  const count = args(C)
  emitShort(C, OP_CALL, count)
}

function compileGroup(C) {
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, 'Expected right parenthesis.')
}

function compileNone(C) {
  emit(C, OP_NONE)
}

function compileTrue(C) {
  emit(C, OP_TRUE)
}

function compileFalse(C) {
  emit(C, OP_FALSE)
}

function compileInteger(C) {
  const previous = C.previous
  const number = parseInt(sourceSubstring(C, previous.len, previous.start))
  emitConstant(C, newInt(number))
}

function compileFloat(C) {
  const previous = C.previous
  const number = parseFloat(sourceSubstring(C, previous.len, previous.start))
  emitConstant(C, newFloat(number))
}

function escapeSequence(c) {
  switch (c) {
    case 'b':
      return '\b'
    case 'f':
      return '\f'
    case 'n':
      return '\n'
    case 'r':
      return '\r'
    case 't':
      return '\t'
    case 'v':
      return '\v'
    case '\\':
      return '\\'
    case "'":
      return "'"
    case '"':
      return '"'
    default:
      return null
  }
}

function parseStringLiteral(string, start, len) {
  const end = start + len
  let literal = ''
  for (let i = start; i < end; i++) {
    const c = string[i]
    if (c === '\\' && i + 1 < end) {
      const e = escapeSequence(string[i + 1])
      if (e !== null) {
        literal += e
        i++
        continue
      }
    }
    literal += c
  }
  return literal
}

function compileString(C) {
  const previous = C.previous
  const string = parseStringLiteral(C.source, previous.start, previous.len)
  emitConstant(C, newString(string))
}

function identConstant(C, token) {
  const string = sourceSubstring(C, token.len, token.start)
  return byteCodeNewConstant(C, newString(string))
}

function beginScope(C) {
  C.scope.depth++
}

function endScope(C) {
  const scope = C.scope
  scope.depth--
  while (scope.localCount > 0 && scope.locals[scope.localCount - 1].depth > scope.depth) {
    emit(C, OP_POP)
    scope.localCount--
  }
}

function compileArray(C) {
  emitConstant(C, newArrayValue(null))
  if (match(C, TOKEN_RIGHT_SQUARE)) {
    return
  }
  while (!check(C, TOKEN_RIGHT_SQUARE) && !check(C, TOKEN_EOF)) {
    emit(C, OP_DUPLICATE)
    expression(C)
    emitShort(C, OP_ARRAY_PUSH, OP_POP)
    if (!check(C, TOKEN_RIGHT_SQUARE)) {
      consume(C, TOKEN_COMMA, "Expected ','.")
    }
  }
  consume(C, TOKEN_RIGHT_SQUARE, "Expected ']' declaring array.")
}

function compileTable(C) {
  emitConstant(C, newTableValue(null))
  if (match(C, TOKEN_RIGHT_CURLY)) {
    return
  }
  while (!check(C, TOKEN_RIGHT_CURLY) && !check(C, TOKEN_EOF)) {
    emit(C, OP_DUPLICATE)
    consume(C, TOKEN_IDENT, 'Expected property name')
    const name = identConstant(C, C.previous)
    consume(C, TOKEN_COLON, "Expected ':'.")
    expression(C)
    emitShort(C, OP_SET_PROPERTY, name)
    emit(C, OP_POP)
    if (!check(C, TOKEN_RIGHT_CURLY)) {
      consume(C, TOKEN_COMMA, "Expected ','.")
    }
  }
  consume(C, TOKEN_RIGHT_CURLY, "Expected '}' declaring table.")
}

function pushLocal(C, name) {
  const scope = C.scope
  if (scope.localCount === HYMN_UINT8_COUNT) {
    compileError(C, name, 'Too many local variables in scope')
    return
  }
  const local = scopeGetLocal(scope, scope.localCount++)
  local.name = name
  local.depth = -1
}

function variable(C, error) {
  consume(C, TOKEN_IDENT, error)
  const scope = C.scope
  if (scope.depth === 0) {
    return identConstant(C, C.previous)
  }
  const token = C.previous
  const name = sourceSubstring(C, token.len, token.start)
  for (let i = scope.localCount - 1; i >= 0; i--) {
    const local = scope.locals[i]
    if (local.depth !== -1 && local.depth < scope.depth) {
      break
    } else if (name === local.name) {
      compileError(C, name, 'Scope Error: Variable `' + sourceSubstring(C, name.len, name.start) + '` already exists in C scope.')
    }
  }
  pushLocal(C, name)
  return 0
}

function localInitialize(C) {
  const scope = C.scope
  if (scope.depth === 0) {
    return
  }
  scope.locals[scope.localCount - 1].depth = scope.depth
}

function finalizeVariable(C, global) {
  if (C.scope.depth > 0) {
    localInitialize(C)
    return
  }
  emitShort(C, OP_DEFINE_GLOBAL, global)
}

function defineNewVariable(C) {
  const v = variable(C, 'Syntax Error: Expected variable name.')
  consume(C, TOKEN_ASSIGN, "Assignment Error: Expected '=' after variable.")
  expression(C)
  finalizeVariable(C, v)
}

function resolveLocal(C, token) {
  const name = sourceSubstring(C, token.len, token.start)
  const scope = C.scope
  for (let i = scope.localCount - 1; i >= 0; i--) {
    const local = scope.locals[i]
    if (name === local.name) {
      if (local.depth === -1) {
        compileError(C, name, 'Reference Error: Local variable `' + sourceSubstring(name.len, name.start) + '` referenced before assignment.')
      }
      return i
    }
  }
  return -1
}

function namedVariable(C, token, assign) {
  let get
  let set
  let v = resolveLocal(C, token)
  if (v !== -1) {
    get = OP_GET_LOCAL
    set = OP_SET_LOCAL
  } else {
    get = OP_GET_GLOBAL
    set = OP_SET_GLOBAL
    v = identConstant(C, token)
  }
  if (assign && checkAssign(C)) {
    const type = C.current.type
    advance(C)
    if (type !== TOKEN_ASSIGN) {
      emitShort(C, get, v)
    }
    expression(C)
    switch (type) {
      case TOKEN_ASSIGN_ADD:
        emit(C, OP_ADD)
        break
      case TOKEN_ASSIGN_BIT_AND:
        emit(C, OP_BIT_AND)
        break
      case TOKEN_ASSIGN_BIT_LEFT_SHIFT:
        emit(C, OP_BIT_LEFT_SHIFT)
        break
      case TOKEN_ASSIGN_BIT_OR:
        emit(C, OP_BIT_OR)
        break
      case TOKEN_ASSIGN_BIT_RIGHT_SHIFT:
        emit(C, OP_BIT_RIGHT_SHIFT)
        break
      case TOKEN_ASSIGN_BIT_XOR:
        emit(C, OP_BIT_XOR)
        break
      case TOKEN_ASSIGN_DIVIDE:
        emit(C, OP_DIVIDE)
        break
      case TOKEN_ASSIGN_MODULO:
        emit(C, OP_MODULO)
        break
      case TOKEN_ASSIGN_MULTIPLY:
        emit(C, OP_MULTIPLY)
        break
      case TOKEN_ASSIGN_SUBTRACT:
        emit(C, OP_SUBTRACT)
        break
      default:
        break
    }
    emitShort(C, set, v)
  } else {
    emitShort(C, get, v)
  }
}

function compileVariable(C, assign) {
  namedVariable(C, C.previous, assign)
}

function compileUnary(C) {
  const type = C.previous.type
  compileWithPrecedence(C, PRECEDENCE_UNARY)
  switch (type) {
    case TOKEN_NOT:
      emit(C, OP_NOT)
      break
    case TOKEN_SUBTRACT:
      emit(C, OP_NEGATE)
      break
    case TOKEN_BIT_NOT:
      emit(C, OP_BIT_NOT)
      break
    default:
      return
  }
}

function compileBinary(C) {
  const type = C.previous.type
  const rule = rules[type]
  compileWithPrecedence(C, rule.precedence + 1)
  switch (type) {
    case TOKEN_ADD:
      emit(C, OP_ADD)
      break
    case TOKEN_SUBTRACT:
      emit(C, OP_SUBTRACT)
      break
    case TOKEN_MODULO:
      emit(C, OP_MODULO)
      break
    case TOKEN_MULTIPLY:
      emit(C, OP_MULTIPLY)
      break
    case TOKEN_DIVIDE:
      emit(C, OP_DIVIDE)
      break
    case TOKEN_EQUAL:
      emit(C, OP_EQUAL)
      break
    case TOKEN_NOT_EQUAL:
      emit(C, OP_NOT_EQUAL)
      break
    case TOKEN_LESS:
      emit(C, OP_LESS)
      break
    case TOKEN_LESS_EQUAL:
      emit(C, OP_LESS_EQUAL)
      break
    case TOKEN_GREATER:
      emit(C, OP_GREATER)
      break
    case TOKEN_GREATER_EQUAL:
      emit(C, OP_GREATER_EQUAL)
      break
    case TOKEN_BIT_OR:
      emit(C, OP_BIT_OR)
      break
    case TOKEN_BIT_AND:
      emit(C, OP_BIT_AND)
      break
    case TOKEN_BIT_XOR:
      emit(C, OP_BIT_XOR)
      break
    case TOKEN_BIT_LEFT_SHIFT:
      emit(C, OP_BIT_LEFT_SHIFT)
      break
    case TOKEN_BIT_RIGHT_SHIFT:
      emit(C, OP_BIT_RIGHT_SHIFT)
      break
    default:
      return
  }
}

function compileDot(C, assign) {
  consume(C, TOKEN_IDENT, "Expected property name after '.'.")
  const name = identConstant(C, C.previous)
  if (assign && match(C, TOKEN_ASSIGN)) {
    expression(C)
    emitShort(C, OP_SET_PROPERTY, name)
  } else {
    emitShort(C, OP_GET_PROPERTY, name)
  }
}

function compileSquare(C, assign) {
  if (match(C, TOKEN_COLON)) {
    emitConstant(C, newInt(0))
    if (match(C, TOKEN_RIGHT_SQUARE)) {
      emitConstant(C, newNone())
    } else {
      expression(C)
      consume(C, TOKEN_RIGHT_SQUARE, "Expected ']' after expression.")
    }
    emit(C, OP_SLICE)
  } else {
    expression(C)
    if (match(C, TOKEN_COLON)) {
      if (match(C, TOKEN_RIGHT_SQUARE)) {
        emitConstant(C, newNone())
      } else {
        expression(C)
        consume(C, TOKEN_RIGHT_SQUARE, "Expected ']' after expression.")
      }
      emit(C, OP_SLICE)
    } else {
      consume(C, TOKEN_RIGHT_SQUARE, "Expected ']' after expression.")
      if (assign && match(C, TOKEN_ASSIGN)) {
        expression(C)
        emit(C, OP_SET_DYNAMIC)
      } else {
        emit(C, OP_GET_DYNAMIC)
      }
    }
  }
}

function emitJump(C, instruction) {
  emit(C, instruction)
  emitShort(C, UINT8_MAX, UINT8_MAX)
  return current(C).count - 2
}

function patchJump(C, jump) {
  if (jump === -1) {
    return
  }
  const code = current(C)
  const offset = code.count - jump - 2
  if (offset > UINT16_MAX) {
    compileError(C, C.previous, 'Jump offset too large.')
    return
  }
  code.instructions[jump] = (offset >> 8) & UINT8_MAX
  code.instructions[jump + 1] = offset & UINT8_MAX
}

function addJump(C, list, instruction) {
  const jump = new JumpList()
  jump.jump = emitJump(C, instruction)
  jump.depth = C.scope.depth
  jump.code = current(C)
  jump.next = list
  return jump
}

function freeJumpAndList(C) {
  let jump = C.jumpAnd
  const code = current(C)
  const depth = C.scope.depth
  while (jump !== null) {
    if (jump.code !== code || jump.depth < depth) {
      break
    }
    patchJump(C, jump.jump)
    jump = jump.next
  }
  C.jumpAnd = jump
}

function freeJumpOrList(C) {
  let jump = C.jumpOr
  const code = current(C)
  const depth = C.scope.depth
  while (jump !== null) {
    if (jump.code !== code || jump.depth < depth) {
      break
    }
    patchJump(C, jump.jump)
    jump = jump.next
  }
  C.jumpOr = jump
}

function freeJumps(C, jump) {
  while (jump !== null) {
    patchJump(C, jump.jump)
    jump = jump.next
  }
}

function compileAnd(C) {
  C.jumpAnd = addJump(C, C.jumpAnd, OP_JUMP_IF_FALSE)
  compileWithPrecedence(C, PRECEDENCE_AND)
}

function compileOr(C) {
  C.jumpOr = addJump(C, C.jumpOr, OP_JUMP_IF_TRUE)
  freeJumpAndList(C)
  compileWithPrecedence(C, PRECEDENCE_OR)
}

function next(instruction) {
  switch (instruction) {
    case OP_POP_N:
    case OP_SET_GLOBAL:
    case OP_SET_LOCAL:
    case OP_SET_PROPERTY:
    case OP_INCREMENT:
    case OP_CALL:
    case OP_TAIL_CALL:
    case OP_CONSTANT:
    case OP_DEFINE_GLOBAL:
    case OP_GET_GLOBAL:
    case OP_GET_LOCAL:
    case OP_GET_PROPERTY:
      return 2
    case OP_GET_TWO_LOCAL:
    case OP_ADD_TWO_LOCAL:
    case OP_JUMP:
    case OP_JUMP_IF_FALSE:
    case OP_JUMP_IF_TRUE:
    case OP_JUMP_IF_EQUAL:
    case OP_JUMP_IF_NOT_EQUAL:
    case OP_JUMP_IF_LESS:
    case OP_JUMP_IF_GREATER:
    case OP_JUMP_IF_LESS_EQUAL:
    case OP_JUMP_IF_GREATER_EQUAL:
    case OP_LOOP:
    case OP_INCREMENT_LOCAL_AND_SET:
    case OP_INCREMENT_LOCAL:
      return 3
    case OP_FOR:
    case OP_FOR_LOOP:
      return 4
    default:
      return 1
  }
}

function adjustable(important, instructions, count, target) {
  for (let t = 0; t < important.length; t++) {
    const view = important[t]
    const i = view.index
    const instruction = view.instruction
    switch (instruction) {
      case OP_JUMP:
      case OP_JUMP_IF_FALSE:
      case OP_JUMP_IF_TRUE:
      case OP_JUMP_IF_EQUAL:
      case OP_JUMP_IF_NOT_EQUAL:
      case OP_JUMP_IF_LESS:
      case OP_JUMP_IF_GREATER:
      case OP_JUMP_IF_LESS_EQUAL:
      case OP_JUMP_IF_GREATER_EQUAL: {
        if (i < target) {
          const jump = (instructions[i + 1] << 8) | instructions[i + 2]
          if (i + 3 + jump === target) {
            return false
          }
        }
        break
      }
      case OP_FOR: {
        if (i < target) {
          const jump = (instructions[i + 2] << 8) | instructions[i + 3]
          if (i + 3 + jump === target) {
            return false
          }
        }
        break
      }
      case OP_LOOP: {
        if (i >= target) {
          const jump = (instructions[i + 1] << 8) | instructions[i + 2]
          if (i + 3 - jump === target) {
            return false
          }
        }
        break
      }
      case OP_FOR_LOOP: {
        if (i >= target) {
          const jump = (instructions[i + 2] << 8) | instructions[i + 3]
          if (i + 3 - jump === target) {
            return false
          }
        }
        break
      }
    }
  }
  return true
}

function rewrite(important, instructions, lines, count, start, shift) {
  for (let t = 0; t < important.length; t++) {
    const view = important[t]
    const i = view.index
    const instruction = view.instruction
    switch (instruction) {
      case OP_JUMP:
      case OP_JUMP_IF_FALSE:
      case OP_JUMP_IF_TRUE:
      case OP_JUMP_IF_EQUAL:
      case OP_JUMP_IF_NOT_EQUAL:
      case OP_JUMP_IF_LESS:
      case OP_JUMP_IF_GREATER:
      case OP_JUMP_IF_LESS_EQUAL:
      case OP_JUMP_IF_GREATER_EQUAL: {
        if (i < start) {
          let jump = (instructions[i + 1] << 8) | instructions[i + 2]
          if (i + 3 + jump > start) {
            jump -= shift
            instructions[i + 1] = (jump >> 8) & UINT8_MAX
            instructions[i + 2] = jump & UINT8_MAX
          }
        }
        break
      }
      case OP_FOR: {
        if (i < start) {
          let jump = (instructions[i + 2] << 8) | instructions[i + 3]
          if (i + 3 + jump > start) {
            jump -= shift
            instructions[i + 2] = (jump >> 8) & UINT8_MAX
            instructions[i + 3] = jump & UINT8_MAX
          }
        }
        break
      }
      case OP_LOOP: {
        if (i >= start) {
          let jump = (instructions[i + 1] << 8) | instructions[i + 2]
          if (i + 3 - jump < start) {
            jump -= shift
            instructions[i + 1] = (jump >> 8) & UINT8_MAX
            instructions[i + 2] = jump & UINT8_MAX
          }
        }
        break
      }
      case OP_FOR_LOOP: {
        if (i >= start) {
          let jump = (instructions[i + 2] << 8) | instructions[i + 3]
          if (i + 3 - jump < start) {
            jump -= shift
            instructions[i + 2] = (jump >> 8) & UINT8_MAX
            instructions[i + 3] = jump & UINT8_MAX
          }
        }
        break
      }
    }
    if (i >= start) {
      view.index = i - shift
    }
  }
  count -= shift
  for (let c = start; c < count; c++) {
    const n = c + shift
    instructions[c] = instructions[n]
    lines[c] = lines[n]
  }
  return shift
}

function update(important, instructions, i, instruction) {
  instructions[i] = instruction
  for (let t = 0; t < important.length; t++) {
    const view = important[t]
    if (i === view.index) {
      view.instruction = instruction
      return
    }
  }
  throw 'Optimization failed to find instruction to update.'
}

function deleter(important, i) {
  for (let t = 0; t < important.length; t++) {
    const view = important[t]
    if (i === view.index) {
      important.splice(t, 1)
      return
    }
  }
  throw 'Optimization failed to find instruction to delete.'
}

function interest(instructions,count) {
  const important = []
  let i = 0
  while (i < count) {
    const instruction = instructions[i]
    switch (instruction) {
      case OP_JUMP:
      case OP_JUMP_IF_FALSE:
      case OP_JUMP_IF_TRUE:
      case OP_JUMP_IF_EQUAL:
      case OP_JUMP_IF_NOT_EQUAL:
      case OP_JUMP_IF_LESS:
      case OP_JUMP_IF_GREATER:
      case OP_JUMP_IF_LESS_EQUAL:
      case OP_JUMP_IF_GREATER_EQUAL:
      case OP_FOR:
      case OP_LOOP:
      case OP_FOR_LOOP: {
        important.push(new Instruction(i, instruction))
        break
      }
    }
    i += next(instruction)
  }
  return important
}

function optimize(C) {
  const code = current(C)
  const instructions = code.instructions
  const lines = code.lines
  let count = code.count
  const important = interest(code)
  let one = 0
  while (one < count) {
    const first = instructions[one]
    const two = one + next(first)
    if (two >= count) break
    const second = instructions[two]

    if (!adjustable(important, instructions, count, one) || !adjustable(important, instructions, count, two)) {
      one = two
      continue
    }

    switch (first) {
      case OP_CALL: {
        if (second === OP_RETURN) {
          instructions[one] = OP_TAIL_CALL
          continue
        }
        break
      }
      case OP_POP: {
        if (second === OP_POP) {
          count -= rewrite(important, instructions, lines, count, one, 1)
          instructions[one] = OP_POP_TWO
          continue
        }
        break
      }
      case OP_POP_TWO: {
        if (second === OP_POP) {
          instructions[one] = OP_POP_N
          instructions[one + 1] = 3
          continue
        }
        break
      }
      case OP_POP_N: {
        if (second === OP_POP) {
          const pop = instructions[one + 1]
          if (pop < UINT8_MAX - 1) {
            count -= rewrite(important, instructions, lines, count, one + 1, 1)
            instructions[one + 1] = pop + 1
            continue
          }
        }
        break
      }
      case OP_GET_LOCAL: {
        if (second === OP_GET_LOCAL) {
          instructions[one] = OP_GET_TWO_LOCAL
          count -= rewrite(important, instructions, lines, count, one + 2, 1)
          continue
        } else if (second === OP_CONSTANT) {
          const three = two + next(second)
          const third = three < count ? instructions[three] : UINT8_MAX
          if (third === OP_ADD) {
            const value = code.constants[code.instructions[two + 1]]
            if (isInt(value)) {
              const add = value.value
              if (add >= 0 && add <= UINT8_MAX) {
                const local = code.instructions[one + 1]
                count -= rewrite(important, instructions, lines, count, one, 2)
                instructions[one] = OP_INCREMENT_LOCAL
                instructions[one + 1] = local
                instructions[one + 2] = add
                continue
              }
            }
          }
        }
        break
      }
      case OP_GET_TWO_LOCAL: {
        if (second === OP_ADD) {
          instructions[one] = OP_ADD_TWO_LOCAL
          count -= rewrite(important, instructions, lines, count, one + 3, 1)
          continue
        }
        break
      }
      case OP_INCREMENT_LOCAL: {
        if (second === OP_SET_LOCAL) {
          if (instructions[one + 1] === instructions[one + 4]) {
            instructions[one] = OP_INCREMENT_LOCAL_AND_SET
            count -= rewrite(important, instructions, lines, count, one + 3, 2)
            continue
          }
        }
        break
      }
      case OP_INCREMENT_LOCAL_AND_SET: {
        if (second === OP_POP) {
          count -= rewrite(important, instructions, lines, count, one + 3, 1)
          continue
        }
        break
      }
      case OP_CONSTANT: {
        if (second === OP_NEGATE) {
          const value = code.constants[instructions[one + 1]]
          if (isInt(value) || isFloat(value)) {
            value.value = -value.value
          }
          const constant = byteCodeNewConstant(C, value)
          instructions[one + 1] = constant
          count -= rewrite(important, instructions, lines, count, one + 2, 1)
          continue
        } else if (second === OP_ADD) {
          const value = code.constants[code.instructions[one + 1]]
          if (isInt(value)) {
            const add = value.value
            if (add >= 0 && add <= UINT8_MAX) {
              instructions[one] = OP_INCREMENT
              instructions[one + 1] = add
              count -= rewrite(important, instructions, lines, count, one + 2, 1)
              continue
            }
          }
        }
        break
      }
      case OP_EQUAL: {
        if (second === OP_JUMP_IF_TRUE) {
          count -= rewrite(important, instructions, lines, count, one, 1)
          update(important, instructions, one, OP_JUMP_IF_EQUAL)
          continue
        } else if (second === OP_JUMP_IF_FALSE) {
          count -= rewrite(important, instructions, lines, count, one, 1)
          update(important, instructions, one, OP_JUMP_IF_NOT_EQUAL)
          continue
        }
        break
      }
      case OP_NOT_EQUAL: {
        if (second === OP_JUMP_IF_TRUE) {
          count -= rewrite(important, instructions, lines, count, one, 1)
          update(important, instructions, one, OP_JUMP_IF_NOT_EQUAL)
          continue
        } else if (second === OP_JUMP_IF_FALSE) {
          count -= rewrite(important, instructions, lines, count, one, 1)
          update(important, instructions, one, OP_JUMP_IF_EQUAL)
          continue
        }
        break
      }
      case OP_LESS: {
        if (second === OP_JUMP_IF_TRUE) {
          count -= rewrite(important, instructions, lines, count, one, 1)
          update(important, instructions, one, OP_JUMP_IF_LESS)
          continue
        } else if (second === OP_JUMP_IF_FALSE) {
          count -= rewrite(important, instructions, lines, count, one, 1)
          update(important, instructions, one, OP_JUMP_IF_GREATER_EQUAL)
          continue
        }
        break
      }
      case OP_GREATER: {
        if (second === OP_JUMP_IF_TRUE) {
          count -= rewrite(important, instructions, lines, count, one, 1)
          update(important, instructions, one, OP_JUMP_IF_GREATER)
          continue
        } else if (second === OP_JUMP_IF_FALSE) {
          count -= rewrite(important, instructions, lines, count, one, 1)
          update(important, instructions, one, OP_JUMP_IF_LESS_EQUAL)
          continue
        }
        break
      }
      case OP_LESS_EQUAL: {
        if (second === OP_JUMP_IF_TRUE) {
          count -= rewrite(important, instructions, lines, count, one, 1)
          update(important, instructions, one, OP_JUMP_IF_LESS_EQUAL)
          continue
        } else if (second === OP_JUMP_IF_FALSE) {
          count -= rewrite(important, instructions, lines, count, one, 1)
          update(important, instructions, one, OP_JUMP_IF_GREATER)
          continue
        }
        break
      }
      case OP_GREATER_EQUAL: {
        if (second === OP_JUMP_IF_TRUE) {
          count -= rewrite(important, instructions, lines, count, one, 1)
          update(important, instructions, one, OP_JUMP_IF_GREATER_EQUAL)
          continue
        } else if (second === OP_JUMP_IF_FALSE) {
          count -= rewrite(important, instructions, lines, count, one, 1)
          update(important, instructions, one, OP_JUMP_IF_LESS)
          continue
        }
        break
      }
      case OP_TRUE: {
        if (second === OP_JUMP_IF_TRUE) {
          count -= rewrite(important, instructions, lines, count, one, 1)
          update(important, instructions, one, OP_JUMP)
          continue
        } else if (second === OP_JUMP_IF_FALSE) {
          deleter(important, two)
          count -= rewrite(important, instructions, lines, count, one, 4)
          continue
        }
        break
      }
      case OP_FALSE: {
        if (second === OP_JUMP_IF_TRUE) {
          deleter(important, two)
          count -= rewrite(important, instructions, lines, count, one, 4)
          continue
        } else if (second === OP_JUMP_IF_FALSE) {
          count -= rewrite(important, instructions, lines, count, one, 1)
          update(important, instructions, one, OP_JUMP)
          continue
        }
        break
      }
      case OP_NOT: {
        if (second === OP_JUMP_IF_TRUE) {
          count -= rewrite(important, instructions, lines, count, one, 1)
          update(important, instructions, one, OP_JUMP_IF_FALSE)
          continue
        } else if (second === OP_JUMP_IF_FALSE) {
          count -= rewrite(important, instructions, lines, count, one, 1)
          update(important, instructions, one, OP_JUMP_IF_TRUE)
          continue
        }
        break
      }
    }

    one = two
  }

  code.count = count
}

function endFunction(C) {
  emitShort(C, OP_NONE, OP_RETURN)
  optimize(C)
  const func = C.scope.func
  C.scope = C.scope.enclosing
  return func
}

function compileFunction(C, type) {
  const scope = new Scope()
  scopeInit(C, scope, type)

  beginScope(C)

  consume(C, TOKEN_LEFT_PAREN, "Expected '(' after function name.")

  if (!check(C, TOKEN_RIGHT_PAREN)) {
    do {
      C.scope.func.arity++
      if (C.scope.func.arity > 255) {
        compileError(C, C.previous, "Can't have more than 255 function parameters.")
      }
      const parameter = variable(C, 'Expected parameter name.')
      finalizeVariable(C, parameter)
    } while (match(C, TOKEN_COMMA))
  }

  consume(C, TOKEN_RIGHT_PAREN, "Expected ')' after function parameters.")

  while (!check(C, TOKEN_END) && !check(C, TOKEN_EOF)) {
    declaration(C)
  }

  endScope(C)
  consume(C, TOKEN_END, "Expected 'end' after function body.")

  const func = endFunction(C)
  emitConstant(C, newFuncValue(func))
}

function declareFunction(C) {
  const global = variable(C, 'Expected function name.')
  localInitialize(C)
  compileFunction(C, TYPE_FUNCTION)
  finalizeVariable(C, global)
}

function declaration(C) {
  if (match(C, TOKEN_LET)) {
    defineNewVariable(C)
  } else if (match(C, TOKEN_FUNCTION)) {
    declareFunction(C)
  } else {
    statement(C)
  }
}

function block(C) {
  beginScope(C)
  while (!check(C, TOKEN_END) && !check(C, TOKEN_EOF)) {
    declaration(C)
  }
  endScope(C)
}

function ifStatement(C) {
  expression(C)
  let jump = emitJump(C, OP_JUMP_IF_FALSE)

  freeJumpOrList(C)

  beginScope(C)
  while (!check(C, TOKEN_ELIF) && !check(C, TOKEN_ELSE) && !check(C, TOKEN_END) && !check(C, TOKEN_EOF)) {
    declaration(C)
  }
  endScope(C)

  if (check(C, TOKEN_END)) {
    patchJump(C, jump)
    freeJumpAndList(C)
  } else {
    const jumpEnd = new JumpList()
    jumpEnd.jump = emitJump(C, OP_JUMP)
    let tail = jumpEnd

    while (match(C, TOKEN_ELIF)) {
      patchJump(C, jump)
      freeJumpAndList(C)

      expression(C)
      jump = emitJump(C, OP_JUMP_IF_FALSE)

      freeJumpOrList(C)

      beginScope(C)
      while (!check(C, TOKEN_ELIF) && !check(C, TOKEN_ELSE) && !check(C, TOKEN_END) && !check(C, TOKEN_EOF)) {
        declaration(C)
      }
      endScope(C)

      const next = new JumpList()
      next.jump = emitJump(C, OP_JUMP)

      tail.next = next
      tail = next
    }

    patchJump(C, jump)
    freeJumpAndList(C)

    if (match(C, TOKEN_ELSE)) {
      block(C)
    }

    patchJump(C, jumpEnd.jump)
    freeJumps(C, jumpEnd.next)
  }

  consume(C, TOKEN_END, "Expected 'end' after if statement.")
}

function emitLoop(C, start) {
  emit(C, OP_LOOP)
  const offset = current(C).count - start + 2
  if (offset > UINT16_MAX) {
    compileError(C, C.previous, 'Loop is too large.')
  }
  emitShort(C, (offset >> 8) & UINT8_MAX, offset & UINT8_MAX)
}

function patchJumpList(C) {
  while (C.jump !== null) {
    let depth = 1
    if (C.loop !== null) {
      depth = C.loop.depth + 1
    }
    if (C.jump.depth < depth) {
      break
    }
    patchJump(C, C.jump.jump)
    C.jump = C.jump.next
  }
}

function patchJumpForList(C) {
  while (C.jumpFor !== null) {
    let depth = 1
    if (C.loop !== null) {
      depth = C.loop.depth
    }
    if (C.jumpFor.depth < depth) {
      break
    }
    patchJump(C, C.jumpFor.jump)
    C.jumpFor = C.jumpFor.next
  }
}

function iteratorStatement(C, pair) {
  localInitialize(C)

  const index = C.scope.localCount
  const value = index + 1
  const object = index - 1

  pushHiddenLocal(C)

  if (pair) {
    variable(C, 'Missing variable name in for loop')
    localInitialize(C)
    consume(C, TOKEN_IN, "Missing 'in' in for loop")
    C.scope.locals[index].name = C.scope.locals[object].name
  } else {
    pushHiddenLocal(C)
    C.scope.locals[value].name = C.scope.locals[object].name
  }

  C.scope.locals[object].name = null

  // IN

  expression(C)

  emitShort(C, OP_FOR, object)
  emitShort(C, UINT8_MAX, UINT8_MAX)

  const start = current(C).count
  const jump = start - 2

  const loop = new LoopList(start, C.scope.depth + 1, C.loop, true)
  C.loop = loop

  // BODY

  block(C)

  // LOOP

  patchJumpForList(C)
  emitShort(C, OP_FOR_LOOP, object)
  const offset = current(C).count - start + 2
  if (offset > UINT16_MAX) {
    compileError(C, C.previous, 'Loop is too large')
  }
  emitShort(C, (offset >> 8) & UINT8_MAX, offset & UINT8_MAX)

  // END

  C.loop = loop.next

  patchJump(C, jump)
  patchJumpList(C)

  endScope(C)

  consume(C, TOKEN_END, "Expected 'end' after for loop.")
}

function forStatement(C) {
  beginScope(C)

  // ASSIGN

  const index = C.scope.localCount

  variable(C, 'Missing variable name in for loop')

  if (match(C, TOKEN_ASSIGN)) {
    expression(C)
    localInitialize(C)
    consume(C, TOKEN_COMMA, "Missing ',' in for loop")
  } else if (match(C, TOKEN_COMMA)) {
    iteratorStatement(C, true)
    return
  } else if (match(C, TOKEN_IN)) {
    iteratorStatement(C, false)
    return
  } else {
    compileError(C, C.previous, 'Missing either `=`, `in`, or `,` in for loop')
    return
  }

  // COMPARE

  const compare = current(C).count

  expression(C)

  const jump = emitJump(C, OP_JUMP_IF_FALSE)

  // INCREMENT

  const increment = current(C).count

  const loop = new LoopList(increment, C.scope.depth + 1, C.loop, true)
  C.loop = loop

  if (match(C, TOKEN_COMMA)) {
    expression(C)
  } else {
    emitWord(C, OP_INCREMENT_LOCAL_AND_SET, index, 1)
  }

  const code = current(C)

  const count = code.count - increment
  const instructions = new Array(count)
  const lines = new Array(count)
  for (let x = 0; x < count; x++) {
    instructions[x] = code.instructions[increment + x]
    lines[x] = code.lines[increment + x]
  }
  code.count = increment

  // BODY

  block(C)

  // INCREMENT

  patchJumpForList(C)

  const position = code.count
  for (let x = 0; x < count; x++) {
    code.instructions[position + x] = instructions[x]
    code.lines[position + x] = lines[x]
  }
  code.count += count

  emitLoop(C, compare)

  // END

  C.loop = loop.next

  patchJump(C, jump)
  patchJumpList(C)

  endScope(C)

  consume(C, TOKEN_END, "Expected 'end' after for loop.")
}

function whileStatement(C) {
  const start = current(C).count

  const loop = new LoopList(start, C.scope.depth + 1, C.loop, false)
  C.loop = loop

  expression(C)

  const jump = emitJump(C, OP_JUMP_IF_FALSE)

  block(C)
  emitLoop(C, start)

  C.loop = loop.next

  patchJump(C, jump)
  patchJumpList(C)

  consume(C, TOKEN_END, "Expected 'end' after while loop.")
}

function returnStatement(C) {
  if (C.scope.type === TYPE_SCRIPT) {
    compileError(C, C.previous, "Can't return from outside a function.")
  }
  if (check(C, TOKEN_END)) {
    emit(C, OP_NONE)
  } else {
    expression(C)
  }
  emit(C, OP_RETURN)
}

function popStackLoop(C) {
  const depth = C.loop.depth
  const scope = C.scope
  for (let i = scope.localCount; i > 0; i--) {
    if (scope.locals[i - 1].depth < depth) {
      return
    }
    emit(C, OP_POP)
  }
}

function breakStatement(C) {
  if (C.loop === null) {
    compileError(C, C.previous, "Can't use 'break' outside of a loop.")
  }
  popStackLoop(C)
  const jumpNext = C.jump
  const jump = new JumpList()
  jump.jump = emitJump(C, OP_JUMP)
  jump.depth = C.loop.depth
  jump.next = jumpNext
  C.jump = jump
}

function continueStatement(C) {
  if (C.loop === null) {
    compileError(C, C.previous, "Can't use 'continue' outside of a loop.")
  }
  popStackLoop(C)
  if (C.loop.isFor) {
    const jumpNext = C.jumpFor
    const jump = new JumpList()
    jump.jump = emitJump(C, OP_JUMP)
    jump.depth = C.loop.depth
    jump.next = jumpNext
    C.jumpFor = jump
  } else {
    emitLoop(C, C.loop.start)
  }
}

function tryStatement(C) {
  const except = new ExceptList()
  except.stack = C.scope.localCount
  except.start = current(C).count

  const func = currentFunc(C)
  except.next = func.except
  func.except = except

  beginScope(C)
  while (!check(C, TOKEN_EXCEPT) && !check(C, TOKEN_EOF)) {
    declaration(C)
  }
  endScope(C)

  const jump = emitJump(C, OP_JUMP)

  consume(C, TOKEN_EXCEPT, "Expected 'except' after 'try'.")

  except.end = current(C).count

  beginScope(C)
  const message = variable(C, "Expected variable after 'except'.")
  finalizeVariable(C, message)
  while (!check(C, TOKEN_END) && !check(C, TOKEN_EOF)) {
    declaration(C)
  }
  endScope(C)

  consume(C, TOKEN_END, "Expected 'end' after 'except'.")

  patchJump(C, jump)
}

function printStatement(C) {
  expression(C)
  emit(C, OP_PRINT)
}

function useStatement(C) {
  expression(C)
  emit(C, OP_USE)
}

function throwStatement(C) {
  expression(C)
  emit(C, OP_THROW)
}

function statement(C) {
  if (match(C, TOKEN_PRINT)) {
    printStatement(C)
  } else if (match(C, TOKEN_USE)) {
    useStatement(C)
  } else if (match(C, TOKEN_IF)) {
    ifStatement(C)
  } else if (match(C, TOKEN_FOR)) {
    forStatement(C)
  } else if (match(C, TOKEN_WHILE)) {
    whileStatement(C)
  } else if (match(C, TOKEN_RETURN)) {
    returnStatement(C)
  } else if (match(C, TOKEN_BREAK)) {
    breakStatement(C)
  } else if (match(C, TOKEN_CONTINUE)) {
    continueStatement(C)
  } else if (match(C, TOKEN_TRY)) {
    tryStatement(C)
  } else if (match(C, TOKEN_THROW)) {
    throwStatement(C)
  } else if (match(C, TOKEN_BEGIN)) {
    block(C)
    consume(C, TOKEN_END, "Expected 'end' after block.")
  } else {
    expressionStatement(C)
  }
}

function arrayPushExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, "Expected '(' after push.")
  expression(C)
  consume(C, TOKEN_COMMA, "Expected ',' between push arguments.")
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, "Expected ')' after push expression.")
  emit(C, OP_ARRAY_PUSH)
}

function arrayInsertExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, "Expected '(' after insert.")
  expression(C)
  consume(C, TOKEN_COMMA, "Expected ',' between insert arguments.")
  expression(C)
  consume(C, TOKEN_COMMA, "Expected ',' between insert arguments.")
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, "Expected ')' after insert expression.")
  emit(C, OP_ARRAY_INSERT)
}

function arrayPopExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, "Expected '(' after pop.")
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, "Expected ')' after pop expression.")
  emit(C, OP_ARRAY_POP)
}

function deleteExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, "Expected '(' after delete.")
  expression(C)
  consume(C, TOKEN_COMMA, "Expected ',' between delete arguments.")
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, "Expected ')' after delete expression.")
  emit(C, OP_DELETE)
}

function lenExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, "Expected '(' after len.")
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, "Expected ')' after len expression.")
  emit(C, OP_LEN)
}

function castIntegerExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, "Expected '(' after integer.")
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, "Expected ')' after integer expression.")
  emit(C, OP_TO_INTEGER)
}

function castFloatExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, "Expected '(' after float.")
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, "Expected ')' after float expression.")
  emit(C, OP_TO_FLOAT)
}

function castStringExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, "Expected '(' after string.")
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, "Expected ')' after string expression.")
  emit(C, OP_TO_STRING)
}

function typeExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, "Expected '(' after type.")
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, "Expected ')' after type expression.")
  emit(C, OP_TYPE)
}

function clearExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, "Expected '(' after clear.")
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, "Expected ')' after clear expression.")
  emit(C, OP_CLEAR)
}

function copyExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, "Expected '(' after copy.")
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, "Expected ')' after copy expression.")
  emit(C, OP_COPY)
}

function keysExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, "Expected '(' after keys.")
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, "Expected ')' after keys expression.")
  emit(C, OP_KEYS)
}

function indexExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, "Missing '(' for paramters in `index` function.")
  expression(C)
  consume(C, TOKEN_COMMA, 'Expected 2 arguments for `index` function.')
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, "Missing ')' after parameters in `index` function.")
  emit(C, OP_INDEX)
}

function expressionStatement(C) {
  expression(C)
  emit(C, OP_POP)
}

function expression(C) {
  compileWithPrecedence(C, PRECEDENCE_ASSIGN)
}

function parentFrame(H, offset) {
  const frameCount = H.frameCount
  if (offset > frameCount) {
    return null
  }
  return H.frames[frameCount - offset]
}

function currentFrame(H) {
  return H.frames[H.frameCount - 1]
}

function compile(H, script, source) {
  const scope = new Scope()
  const C = newCompiler(script, source, H, scope)

  advance(C)
  while (!match(C, TOKEN_EOF)) {
    declaration(C)
  }

  const func = endFunction(C)
  return { func: func, error: C.error }
}

function valueToStringRecursive(value, set, quote) {
  switch (value.is) {
    case HYMN_VALUE_NONE:
      return STRING_NONE
    case HYMN_VALUE_BOOL:
      return value.value ? STRING_TRUE : STRING_FALSE
    case HYMN_VALUE_INTEGER:
    case HYMN_VALUE_FLOAT:
      return value.value
    case HYMN_VALUE_STRING:
      if (quote) {
        return '"' + value.value + '"'
      }
      return value.value
    case HYMN_VALUE_ARRAY: {
      const array = value.value
      if (!array || array.length === 0) {
        return '[]'
      }
      if (set === null) {
        set = new Set()
      } else if (set.has(array)) {
        return '[..]'
      }
      set.add(array)
      let print = '['
      for (let i = 0; i < array.length; i++) {
        const item = array[i]
        if (i !== 0) {
          print += ', '
        }
        print += valueToStringRecursive(item, set, true)
      }
      print += ']'
      return print
    }
    case HYMN_VALUE_TABLE: {
      const table = value.value
      if (!table || table.size === 0) {
        return '{}'
      }
      if (set === null) {
        set = new Set()
      } else if (set.has(table)) {
        return '{ .. }'
      }
      set.add(table)
      const size = table.size
      const keys = new Array(size)
      let total = 0
      const bins = table.bins
      for (let i = 0; i < bins; i++) {
        let item = table.items[i]
        while (item !== null) {
          const key = item.key
          let insert = 0
          while (insert !== total) {
            if (stringCompare(key, keys[insert]) < 0) {
              for (let swap = total; swap > insert; swap--) {
                keys[swap] = keys[swap - 1]
              }
              break
            }
            insert++
          }
          keys[insert] = item.key
          total++
          item = item.next
        }
      }
      let print = '{ '
      for (let i = 0; i < size; i++) {
        if (i !== 0) {
          print += ', '
        }
        const key = keys[i]
        const item = tableGet(table, key)
        print += key + ': ' + valueToStringRecursive(item, set, true)
      }
      print += ' }'
      return print
    }
    case HYMN_VALUE_FUNC: {
      const func = value.value
      if (func.name) {
        return func.name
      }
      if (func.script) {
        return func.script
      }
      return 'Script'
    }
    case HYMN_VALUE_FUNC_NATIVE:
      return value.value.name
    case HYMN_VALUE_POINTER:
      return '' + value.value
  }
}

function valueToString(value) {
  return valueToStringRecursive(value, null, false)
}

function hymnConcat(a, b) {
  return newString(valueToString(a) + valueToString(b))
}

function debugValueToString(value) {
  return valueName(value.is) + ': ' + valueToString(value)
}

function hymnResetStack(H) {
  H.stackTop = 0
  H.frameCount = 0
}

function hymnStackGet(H, index) {
  if (index < H.stack.length) {
    return H.stack[index]
  }
  const value = new HymnValue(HYMN_VALUE_NONE, null)
  H.stack.push(value)
  return value
}

function hymnPush(H, value) {
  copyValueToFrom(hymnStackGet(H, H.stackTop++), value)
}

function hymnPeek(H, dist) {
  if (dist > H.stackTop) {
    console.error('Nothing on stack to peek')
    return newNone()
  }
  return cloneValue(H.stack[H.stackTop - dist])
}

function hymnPop(H) {
  if (H.stackTop === 0) {
    console.error('Nothing on stack to pop')
    return newNone()
  }
  return cloneValue(H.stack[--H.stackTop])
}

function hymnException(H) {
  let frame = currentFrame(H)
  while (true) {
    let except = null
    let range = frame.func.except
    while (range !== null) {
      if (frame.ip >= range.start && frame.ip <= range.end) {
        except = range
        break
      }
      range = range.next
    }
    const result = hymnPop(H)
    if (except !== null) {
      H.stackTop = frame.stack + except.stack
      frame.ip = except.end
      hymnPush(H, result)
      return frame
    }
    H.stackTop = frame.stack
    H.frameCount--
    if (H.frameCount === 0 || frame.func.name === null) {
      H.error = valueToString(result)
      return null
    }
    hymnPush(H, result)
    frame = currentFrame(H)
  }
}

function hymnStacktrace(H) {
  let trace = ''

  for (let i = H.frameCount - 1; i >= 0; i--) {
    const frame = H.frames[i]
    const func = frame.func
    const ip = frame.ip - 1
    const row = func.code.lines[ip]

    trace += 'at'

    if (func.name !== null) {
      trace += ' ' + func.name
    }

    if (func.script === null) {
      trace += ' script:'
    } else {
      trace += ' ' + func.script + ':'
    }

    trace += row + '\n'
  }

  return trace
}

function hymnPushError(H, error) {
  hymnPush(H, newString(error))
  return hymnException(H)
}

function hymnThrowExistingError(H, error) {
  return hymnPushError(H, error)
}

function hymnThrowError(H, error) {
  error += '\n\n'
  error += hymnStacktrace(H)
  return hymnPushError(H, error)
}

function hymnEqual(a, b) {
  switch (a.is) {
    case HYMN_VALUE_NONE:
      return isNone(b)
    case HYMN_VALUE_BOOL:
      return isBool(b) && a.value === b.value
    case HYMN_VALUE_INTEGER:
      switch (b.is) {
        case HYMN_VALUE_INTEGER:
          return a.value === b.value
        case HYMN_VALUE_FLOAT:
          return a.value === b.value
        default:
          return false
      }
    case HYMN_VALUE_FLOAT:
      switch (b.is) {
        case HYMN_VALUE_INTEGER:
          return a.value === b.value
        case HYMN_VALUE_FLOAT:
          return a.value === b.value
        default:
          return false
      }
    case HYMN_VALUE_STRING:
    case HYMN_VALUE_ARRAY:
    case HYMN_VALUE_TABLE:
    case HYMN_VALUE_FUNC:
    case HYMN_VALUE_FUNC_NATIVE:
      if (b.is === a.is) {
        return a.value === b.value
      }
      return false
    default:
      return false
  }
}

function hymnFalse(value) {
  switch (value.is) {
    case HYMN_VALUE_NONE:
      return true
    case HYMN_VALUE_BOOL:
      return !value.value
    case HYMN_VALUE_INTEGER:
      return value.value === 0
    case HYMN_VALUE_FLOAT:
      return value.value === 0.0
    case HYMN_VALUE_STRING:
    case HYMN_VALUE_ARRAY:
      return value.value.length === 0
    case HYMN_VALUE_TABLE:
      return value.value.size === 0
    default:
      return false
  }
}

function hymnFrameGet(H, index) {
  if (index < H.frames.length) {
    return H.frames[index]
  }
  const frame = new HymnFrame()
  H.frames.push(frame)
  return frame
}

function hymnCall(H, func, count) {
  if (count !== func.arity) {
    return hymnThrowError(H, 'Expected ' + func.arity + ' function arguments but found ' + count + '.')
  } else if (H.frameCount === HYMN_FRAMES_MAX) {
    return hymnThrowError(H, 'Stack overflow.')
  }

  const frame = hymnFrameGet(H, H.frameCount++)
  frame.func = func
  frame.ip = 0
  frame.stack = H.stackTop - count - 1

  return frame
}

function hymnCallValue(H, value, count) {
  switch (value.is) {
    case HYMN_VALUE_FUNC:
      return hymnCall(H, value.value, count)
    case HYMN_VALUE_FUNC_NATIVE: {
      const func = value.value.func
      const result = func(H, count, H.stack[H.stackTop - count])
      const top = H.stackTop - (count + 1)
      H.stackTop = top
      hymnPush(H, result)
      return currentFrame(H)
    }
    default: {
      const is = valueName(value.is)
      return hymnThrowError(H, 'Call: Requires `Function`, but was `' + is + '`.')
    }
  }
}

function readByte(frame) {
  return frame.func.code.instructions[frame.ip++]
}

function readShort(frame) {
  frame.ip += 2
  return (frame.func.code.instructions[frame.ip - 2] << 8) | frame.func.code.instructions[frame.ip - 1]
}

function readConstant(frame) {
  return frame.func.code.constants[readByte(frame)]
}

function httpPathParent(path) {
  if (path.length < 2) {
    return path
  }
  let i = path.length - 2
  while (true) {
    if (i === 0) break
    if (path[i] === '/') break
    i--
  }
  return path.substring(0, i)
}

async function hymnImport(H, file) {
  const imports = H.imports

  let script = null
  let p = 1
  while (true) {
    const frame = parentFrame(H, p)
    if (frame === null) {
      break
    }
    script = frame.func.script
    if (script !== null) {
      break
    }
    p++
  }

  const paths = H.paths
  const size = paths.length

  let module = null
  let source = null

  if (node) {
    const parent = script ? node_path.dirname(script) : null

    for (let i = 0; i < size; i++) {
      const value = paths[i]
      if (!isString(value)) {
        continue
      }
      const question = value.value

      const replace = question.replace(/<path>/g, file)
      const path = parent ? replace.replace(/<parent>/g, parent) : replace
      const use = node_path.resolve(path)

      if (tableGet(imports, use) !== null) {
        return currentFrame(H)
      }

      if (node_fs.existsSync(use)) {
        module = use
        break
      }
    }

    if (module === null) {
      let missing = 'Import not found: ' + file + '\n'

      for (let i = 0; i < size; i++) {
        const value = paths[i]
        if (!isString(value)) {
          continue
        }
        const question = value.value

        const replace = question.replace(/<path>/g, file)
        const path = parent ? replace.replace(/<parent>/g, parent) : replace
        const use = node_path.resolve(path)

        missing += '\nno file ' + use
      }

      return hymnPushError(H, missing)
    }

    tablePut(imports, module, newBool(true))

    source = node_fs.readFileSync(module, { encoding: 'utf-8' })
  } else {
    const parent = script ? httpPathParent(script) : null

    for (let i = 0; i < size; i++) {
      const value = paths[i]
      if (!isString(value)) {
        continue
      }
      const question = value.value

      const replace = question.replace(/<path>/g, file)
      const use = parent ? replace.replace(/<parent>/g, parent) : replace

      if (tableGet(imports, use) !== null) {
        return currentFrame(H)
      }

      const response = await fetch(use).catch((exception) => {
        return { ok: false, status: 404, exception: exception }
      })

      if (response.ok) {
        source = await response.text().catch((exception) => {
          return exception
        })
        module = use
        break
      }
    }

    if (module === null) {
      let missing = 'Import not found: ' + file + '\n'

      for (let i = 0; i < size; i++) {
        const value = paths[i]
        if (!isString(value)) {
          continue
        }
        const question = value.value

        const replace = question.replace(/<path>/g, file)
        const use = parent ? replace.replace(/<parent>/g, parent) : replace

        missing += '\nno file ' + use
      }

      return hymnPushError(H, missing)
    }

    tablePut(imports, module, newBool(true))
  }

  const result = compile(H, module, source)

  const func = result.func
  let error = result.error

  if (error) {
    return hymnThrowExistingError(H, error)
  }

  const funcValue = newFuncValue(func)

  hymnPush(H, funcValue)
  hymnCall(H, func, 0)

  error = await hymnRun(H)
  if (error) {
    return hymnThrowExistingError(H, error)
  }

  return currentFrame(H)
}

function debugConstantInstruction(debug, name, code, index) {
  const constant = code.instructions[index + 1]
  debug[0] += `${name}: [${debugValueToString(code.constants[constant])}]`
  return index + 2
}

function debugByteInstruction(debug, name, code, index) {
  const byte = code.instructions[index + 1]
  debug[0] += `${name}: [${byte}]`
  return index + 2
}

function debugJumpInstruction(debug, name, sign, code, index) {
  const jump = (code.instructions[index + 1] << 8) | code.instructions[index + 2]
  debug[0] += `${name}: [${index}] -> [${index + 3 + sign * jump}]`
  return index + 3
}

function debugThreeByteInstruction(debug, name, code, index) {
  const byte = code.instructions[index + 1]
  const next = code.instructions[index + 2]
  debug[0] += `${name}: [${byte}] [${next}]`
  return index + 3
}

function debugForLoopInstruction(debug, name, sign, code, index) {
  const slot = code.instructions[index + 1]
  const jump = (code.instructions[index + 2] << 8) | code.instructions[index + 3]
  debug[0] += `${name}: [${slot}] [${index}] -> [${index + 4 + sign * jump}]`
  return index + 4
}

function debugInstruction(debug, name, index) {
  debug[0] += name
  return index + 1
}

function disassembleInstruction(debug, code, index) {
  debug[0] += String(index).padStart(4, '0') + ' '
  if (index > 0 && code.lines[index] === code.lines[index - 1]) {
    debug[0] += '   | '
  } else {
    debug[0] += String(code.lines[index]).padStart(4, ' ') + ' '
  }
  const instruction = code.instructions[index]
  switch (instruction) {
    case OP_ADD:
      return debugInstruction(debug, 'OP_ADD', index)
    case OP_ADD_TWO_LOCAL:
      return debugThreeByteInstruction(debug, 'OP_ADD_TWO_LOCAL', code, index)
    case OP_ARRAY_INSERT:
      return debugInstruction(debug, 'OP_ARRAY_INSERT', index)
    case OP_ARRAY_POP:
      return debugInstruction(debug, 'OP_ARRAY_POP', index)
    case OP_ARRAY_PUSH:
      return debugInstruction(debug, 'OP_ARRAY_PUSH', index)
    case OP_BIT_AND:
      return debugInstruction(debug, 'OP_BIT_AND', index)
    case OP_BIT_LEFT_SHIFT:
      return debugInstruction(debug, 'OP_BIT_LEFT_SHIFT', index)
    case OP_BIT_NOT:
      return debugInstruction(debug, 'OP_BIT_NOT', index)
    case OP_BIT_OR:
      return debugInstruction(debug, 'OP_BIT_OR', index)
    case OP_BIT_RIGHT_SHIFT:
      return debugInstruction(debug, 'OP_BIT_RIGHT_SHIFT', index)
    case OP_BIT_XOR:
      return debugInstruction(debug, 'OP_BIT_XOR', index)
    case OP_CALL:
      return debugByteInstruction(debug, 'OP_CALL', code, index)
    case OP_CLEAR:
      return debugInstruction(debug, 'OP_CLEAR', index)
    case OP_CONSTANT:
      return debugConstantInstruction(debug, 'OP_CONSTANT', code, index)
    case OP_COPY:
      return debugInstruction(debug, 'OP_COPY', index)
    case OP_DEFINE_GLOBAL:
      return debugConstantInstruction(debug, 'OP_DEFINE_GLOBAL', code, index)
    case OP_DELETE:
      return debugInstruction(debug, 'OP_DELETE', index)
    case OP_DIVIDE:
      return debugInstruction(debug, 'OP_DIVIDE', index)
    case OP_DUPLICATE:
      return debugInstruction(debug, 'OP_DUPLICATE', index)
    case OP_EQUAL:
      return debugInstruction(debug, 'OP_EQUAL', index)
    case OP_FALSE:
      return debugInstruction(debug, 'OP_FALSE', index)
    case OP_FOR:
      return debugForLoopInstruction(debug, 'OP_FOR', 1, code, index)
    case OP_FOR_LOOP:
      return debugForLoopInstruction(debug, 'OP_FOR_LOOP', -1, code, index)
    case OP_GET_DYNAMIC:
      return debugInstruction(debug, 'OP_GET_DYNAMIC', index)
    case OP_GET_GLOBAL:
      return debugConstantInstruction(debug, 'OP_GET_GLOBAL', code, index)
    case OP_GET_LOCAL:
      return debugByteInstruction(debug, 'OP_GET_LOCAL', code, index)
    case OP_GET_PROPERTY:
      return debugConstantInstruction(debug, 'OP_GET_PROPERTY', code, index)
    case OP_GET_TWO_LOCAL:
      return debugThreeByteInstruction(debug, 'OP_GET_TWO_LOCAL', code, index)
    case OP_GREATER:
      return debugInstruction(debug, 'OP_GREATER', index)
    case OP_GREATER_EQUAL:
      return debugInstruction(debug, 'OP_GREATER_EQUAL', index)
    case OP_INCREMENT:
      return debugByteInstruction(debug, 'OP_INCREMENT', code, index)
    case OP_INCREMENT_LOCAL:
      return debugThreeByteInstruction(debug, 'OP_INCREMENT_LOCAL', code, index)
    case OP_INCREMENT_LOCAL_AND_SET:
      return debugThreeByteInstruction(debug, 'OP_INCREMENT_LOCAL_AND_SET', code, index)
    case OP_INDEX:
      return debugInstruction(debug, 'OP_INDEX', index)
    case OP_JUMP:
      return debugJumpInstruction(debug, 'OP_JUMP', 1, code, index)
    case OP_JUMP_IF_EQUAL:
      return debugJumpInstruction(debug, 'OP_JUMP_IF_EQUAL', 1, code, index)
    case OP_JUMP_IF_FALSE:
      return debugJumpInstruction(debug, 'OP_JUMP_IF_FALSE', 1, code, index)
    case OP_JUMP_IF_GREATER:
      return debugJumpInstruction(debug, 'OP_JUMP_IF_GREATER', 1, code, index)
    case OP_JUMP_IF_GREATER_EQUAL:
      return debugJumpInstruction(debug, 'OP_JUMP_IF_GREATER_EQUAL', 1, code, index)
    case OP_JUMP_IF_LESS:
      return debugJumpInstruction(debug, 'OP_JUMP_IF_LESS', 1, code, index)
    case OP_JUMP_IF_LESS_EQUAL:
      return debugJumpInstruction(debug, 'OP_JUMP_IF_LESS_EQUAL', 1, code, index)
    case OP_JUMP_IF_NOT_EQUAL:
      return debugJumpInstruction(debug, 'OP_JUMP_IF_NOT_EQUAL', 1, code, index)
    case OP_JUMP_IF_TRUE:
      return debugJumpInstruction(debug, 'OP_JUMP_IF_TRUE', 1, code, index)
    case OP_KEYS:
      return debugInstruction(debug, 'OP_KEYS', index)
    case OP_LEN:
      return debugInstruction(debug, 'OP_LEN', index)
    case OP_LESS:
      return debugInstruction(debug, 'OP_LESS', index)
    case OP_LESS_EQUAL:
      return debugInstruction(debug, 'OP_LESS_EQUAL', index)
    case OP_LOOP:
      return debugJumpInstruction(debug, 'OP_LOOP', -1, code, index)
    case OP_MODULO:
      return debugInstruction(debug, 'OP_MODULO', index)
    case OP_MULTIPLY:
      return debugInstruction(debug, 'OP_MULTIPLY', index)
    case OP_NEGATE:
      return debugInstruction(debug, 'OP_NEGATE', index)
    case OP_NONE:
      return debugInstruction(debug, 'OP_NONE', index)
    case OP_NOT:
      return debugInstruction(debug, 'OP_NOT', index)
    case OP_NOT_EQUAL:
      return debugInstruction(debug, 'OP_NOT_EQUAL', index)
    case OP_POP:
      return debugInstruction(debug, 'OP_POP', index)
    case OP_POP_N:
      return debugByteInstruction(debug, 'OP_POP_N', code, index)
    case OP_POP_TWO:
      return debugInstruction(debug, 'OP_POP_TWO', index)
    case OP_PRINT:
      return debugInstruction(debug, 'OP_PRINT', index)
    case OP_RETURN:
      return debugInstruction(debug, 'OP_RETURN', index)
    case OP_SET_DYNAMIC:
      return debugInstruction(debug, 'OP_SET_DYNAMIC', index)
    case OP_SET_GLOBAL:
      return debugConstantInstruction(debug, 'OP_SET_GLOBAL', code, index)
    case OP_SET_LOCAL:
      return debugByteInstruction(debug, 'OP_SET_LOCAL', code, index)
    case OP_SET_PROPERTY:
      return debugConstantInstruction(debug, 'OP_SET_PROPERTY', code, index)
    case OP_SLICE:
      return debugInstruction(debug, 'OP_SLICE', index)
    case OP_SUBTRACT:
      return debugInstruction(debug, 'OP_SUBTRACT', index)
    case OP_TAIL_CALL:
      return debugByteInstruction(debug, 'OP_TAIL_CALL', code, index)
    case OP_THROW:
      return debugInstruction(debug, 'OP_THROW', index)
    case OP_TO_FLOAT:
      return debugInstruction(debug, 'OP_TO_FLOAT', index)
    case OP_TO_INTEGER:
      return debugInstruction(debug, 'OP_TO_INTEGER', index)
    case OP_TO_STRING:
      return debugInstruction(debug, 'OP_TO_STRING', index)
    case OP_TRUE:
      return debugInstruction(debug, 'OP_TRUE', index)
    case OP_TYPE:
      return debugInstruction(debug, 'OP_TYPE', index)
    case OP_USE:
      return debugInstruction(debug, 'OP_USE', index)
    default:
      return (debug[0] += 'UNKNOWN OPCODE ' + instruction)
  }
}

function disassembleByteCode(code, name) {
  console.debug(`\n-- ${name !== null ? name : 'NULL'} --`)
  const debug = ['']
  let index = 0
  while (index < code.count) {
    index = disassembleInstruction(debug, code, index)
    console.debug(debug[0])
    debug[0] = ''
  }
}

function debugStack(H) {
  if (H.stackTop === 0) {
    return
  }
  let debug = 'STACK   | '
  for (let i = 0; i < H.stackTop; i++) {
    debug += '[' + debugValueToString(H.stack[i]) + '] '
  }
  console.debug(debug)
}

function debugTrace(code, index) {
  const debug = ['']
  disassembleInstruction(debug, code, index)
  console.debug(debug[0])
}

async function hymnRun(H) {
  let frame = currentFrame(H)
  while (true) {
    if (HYMN_DEBUG_STACK) debugStack(H)
    if (HYMN_DEBUG_TRACE) debugTrace(frame.func.code, frame.ip)
    switch (readByte(frame)) {
      case OP_RETURN: {
        const result = hymnPop(H)
        H.frameCount--
        if (H.frameCount === 0 || frame.func.name === null) {
          hymnPop(H)
          return
        }
        H.stackTop = frame.stack
        hymnPush(H, result)
        frame = currentFrame(H)
        break
      }
      case OP_POP:
        hymnPop(H)
        break
      case OP_POP_TWO:
        hymnPop(H)
        hymnPop(H)
        break
      case OP_POP_N: {
        let count = readByte(frame)
        while (count--) {
          hymnPop(H)
        }
        break
      }
      case OP_TRUE:
        hymnPush(H, newBool(true))
        break
      case OP_FALSE:
        hymnPush(H, newBool(false))
        break
      case OP_NONE:
        hymnPush(H, newNone())
        break
      case OP_CALL: {
        const count = readByte(frame)
        const call = hymnPeek(H, count + 1)
        frame = hymnCallValue(H, call, count)
        if (frame === null) return
        break
      }
      case OP_TAIL_CALL: {
        // TODO
        const count = readByte(frame)
        const call = hymnPeek(H, count + 1)
        frame = hymnCallValue(H, call, count)
        if (frame === null) return
        break
      }
      case OP_JUMP: {
        const jump = readShort(frame)
        frame.ip += jump
        break
      }
      case OP_JUMP_IF_FALSE: {
        const value = hymnPop(H)
        const jump = readShort(frame)
        if (hymnFalse(value)) {
          frame.ip += jump
        }
        break
      }
      case OP_JUMP_IF_TRUE: {
        const value = hymnPop(H)
        const jump = readShort(frame)
        if (!hymnFalse(value)) {
          frame.ip += jump
        }
        break
      }
      case OP_JUMP_IF_EQUAL: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        const jump = readShort(frame)
        if (hymnEqual(a, b)) {
          frame.ip += jump
        }
        break
      }
      case OP_JUMP_IF_NOT_EQUAL: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        const jump = readShort(frame)
        if (!hymnEqual(a, b)) {
          frame.ip += jump
        }
        break
      }
      case OP_JUMP_IF_LESS: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        let answer = false
        if ((isInt(a) || isFloat(a)) && (isInt(b) || isFloat(b))) {
          answer = a.value < b.value
        } else {
          frame = hymnThrowError(H, 'Operands must be numbers.')
          if (frame === null) return
          else break
        }
        const jump = readShort(frame)
        if (answer) {
          frame.ip += jump
        }
        break
      }
      case OP_JUMP_IF_LESS_EQUAL: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        let answer = false
        if ((isInt(a) || isFloat(a)) && (isInt(b) || isFloat(b))) {
          answer = a.value <= b.value
        } else {
          frame = hymnThrowError(H, 'Operands must be numbers.')
          if (frame === null) return
          else break
        }
        const jump = readShort(frame)
        if (answer) {
          frame.ip += jump
        }
        break
      }
      case OP_JUMP_IF_GREATER: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        let answer = false
        if ((isInt(a) || isFloat(a)) && (isInt(b) || isFloat(b))) {
          answer = a.value > b.value
        } else {
          frame = hymnThrowError(H, 'Operands must be numbers.')
          if (frame === null) return
          else break
        }
        const jump = readShort(frame)
        if (answer) {
          frame.ip += jump
        }
        break
      }
      case OP_JUMP_IF_GREATER_EQUAL: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        let answer = false
        if ((isInt(a) || isFloat(a)) && (isInt(b) || isFloat(b))) {
          answer = a.value >= b.value
        } else {
          frame = hymnThrowError(H, 'Operands must be numbers.')
          if (frame === null) return
          else break
        }
        const jump = readShort(frame)
        if (answer) {
          frame.ip += jump
        }
        break
      }
      case OP_LOOP: {
        const jump = readShort(frame)
        frame.ip -= jump
        break
      }
      case OP_FOR: {
        const slot = readByte(frame)
        const object = H.stack[frame.stack + slot]
        H.stackTop += 2
        if (isTable(object)) {
          const table = object.value
          const next = tableNext(table, null)
          if (next === null) {
            H.stack[frame.stack + slot + 1] = newNone()
            H.stack[frame.stack + slot + 2] = newNone()
            const jump = readShort(frame)
            frame.ip += jump
          } else {
            H.stack[frame.stack + slot + 1] = newString(next.key)
            H.stack[frame.stack + slot + 2] = cloneValue(next.value)
            frame.ip += 2
          }
        } else if (isArray(object)) {
          const array = object.value
          if (array.length === 0) {
            H.stack[frame.stack + slot + 1] = newNone()
            H.stack[frame.stack + slot + 2] = newNone()
            const jump = readShort(frame)
            frame.ip += jump
          } else {
            const item = array[0]
            H.stack[frame.stack + slot + 1] = newInt(0)
            H.stack[frame.stack + slot + 2] = cloneValue(item)
            frame.ip += 2
          }
        } else {
          H.stack[frame.stack + slot + 1] = newNone()
          H.stack[frame.stack + slot + 2] = newNone()
          frame = hymnThrowError(H, 'Loop: Expected table or array')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_FOR_LOOP: {
        const slot = readByte(frame)
        const object = H.stack[frame.stack + slot]
        const index = slot + 1
        const value = slot + 2
        if (isTable(object)) {
          const table = object.value
          const key = H.stack[frame.stack + index].value
          const next = tableNext(table, key)
          if (next === null) {
            frame.ip += 2
          } else {
            H.stack[frame.stack + index] = newString(next.key)
            H.stack[frame.stack + value] = cloneValue(next.value)
            const jump = readShort(frame)
            frame.ip -= jump
          }
        } else {
          const array = object.value
          const key = H.stack[frame.stack + index].value + 1
          if (key >= array.length) {
            frame.ip += 2
          } else {
            const item = array[key]
            H.stack[frame.stack + index].value++
            H.stack[frame.stack + value] = cloneValue(item)
            const jump = readShort(frame)
            frame.ip -= jump
          }
        }
        break
      }
      case OP_EQUAL: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        hymnPush(H, newBool(hymnEqual(a, b)))
        break
      }
      case OP_NOT_EQUAL: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        hymnPush(H, newBool(!hymnEqual(a, b)))
        break
      }
      case OP_LESS: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        if ((isInt(a) || isFloat(a)) && (isInt(b) || isFloat(b))) {
          hymnPush(H, newBool(a.value < b.value))
        } else {
          frame = hymnThrowError(H, 'Operands must be numbers.')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_LESS_EQUAL: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        if ((isInt(a) || isFloat(a)) && (isInt(b) || isFloat(b))) {
          hymnPush(H, newBool(a.value <= b.value))
        } else {
          frame = hymnThrowError(H, 'Operands must be numbers.')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_GREATER: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        if ((isInt(a) || isFloat(a)) && (isInt(b) || isFloat(b))) {
          hymnPush(H, newBool(a.value > b.value))
        } else {
          frame = hymnThrowError(H, 'Operands must be numbers.')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_GREATER_EQUAL: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        if ((isInt(a) || isFloat(a)) && (isInt(b) || isFloat(b))) {
          hymnPush(H, newBool(a.value >= b.value))
        } else {
          frame = hymnThrowError(H, 'Operands must be numbers.')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_ADD: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        if (isNone(a)) {
          if (isString(b)) {
            hymnPush(H, hymnConcat(a, b))
          } else {
            frame = hymnThrowError(H, "Add: 1st and 2nd values can't be added.")
            if (frame === null) return
            else break
          }
        } else if (isBool(a)) {
          if (isString(b)) {
            hymnPush(H, hymnConcat(a, b))
          } else {
            frame = hymnThrowError(H, "Add: 1st and 2nd values can't be added.")
            if (frame === null) return
            else break
          }
        } else if (isInt(a)) {
          if (isInt(b)) {
            a.value += b.value
            hymnPush(H, a)
          } else if (isFloat(b)) {
            b.value += a.value
            hymnPush(H, a)
          } else if (isString(b)) {
            hymnPush(H, hymnConcat(a, b))
          } else {
            frame = hymnThrowError(H, "Add: 1st and 2nd values can't be added.")
            if (frame === null) return
            else break
          }
        } else if (isFloat(a)) {
          if (isInt(b)) {
            a.value += b.value
            hymnPush(H, a)
          } else if (isFloat(b)) {
            a.value += b.value
            hymnPush(H, a)
          } else if (isString(b)) {
            hymnPush(H, hymnConcat(a, b))
          } else {
            frame = hymnThrowError(H, "Add: 1st and 2nd values can't be added.")
            if (frame === null) return
            else break
          }
        } else if (isString(a)) {
          hymnPush(H, hymnConcat(a, b))
        } else {
          frame = hymnThrowError(H, "Add: 1st and 2nd values can't be added.")
          if (frame === null) return
          else break
        }
        break
      }
      case OP_ADD_TWO_LOCAL: {
        const a = cloneValue(H.stack[frame.stack + readByte(frame)])
        const b = cloneValue(H.stack[frame.stack + readByte(frame)])
        if (isNone(a)) {
          if (isString(b)) {
            hymnPush(H, hymnConcat(a, b))
          } else {
            frame = hymnThrowError(H, "Add: 1st and 2nd values can't be added.")
            if (frame === null) return
            else break
          }
        } else if (isBool(a)) {
          if (isString(b)) {
            hymnPush(H, hymnConcat(a, b))
          } else {
            frame = hymnThrowError(H, "Add: 1st and 2nd values can't be added.")
            if (frame === null) return
            else break
          }
        } else if (isInt(a)) {
          if (isInt(b)) {
            a.value += b.value
            hymnPush(H, a)
          } else if (isFloat(b)) {
            b.value += a.value
            hymnPush(H, a)
          } else if (isString(b)) {
            hymnPush(H, hymnConcat(a, b))
          } else {
            frame = hymnThrowError(H, "Add: 1st and 2nd values can't be added.")
            if (frame === null) return
            else break
          }
        } else if (isFloat(a)) {
          if (isInt(b)) {
            a.value += b.value
            hymnPush(H, a)
          } else if (isFloat(b)) {
            a.value += b.value
            hymnPush(H, a)
          } else if (isString(b)) {
            hymnPush(H, hymnConcat(a, b))
          } else {
            frame = hymnThrowError(H, "Add: 1st and 2nd values can't be added.")
            if (frame === null) return
            else break
          }
        } else if (isString(a)) {
          hymnPush(H, hymnConcat(a, b))
        } else {
          frame = hymnThrowError(H, "Add: 1st and 2nd values can't be added.")
          if (frame === null) return
          else break
        }
        break
      }
      case OP_INCREMENT: {
        const a = hymnPop(H)
        const increment = readByte(frame)
        if (isNone(a)) {
          frame = hymnThrowError(H, "Increment: 1st and 2nd values can't be added")
          if (frame === null) return
          else break
        } else if (isBool(a)) {
          frame = hymnThrowError(H, "Increment: 1st and 2nd values can't be added")
          if (frame === null) return
          else break
        } else if (isInt(a) || isFloat(a)) {
          a.value += increment
          hymnPush(H, a)
        } else if (isString(a)) {
          hymnPush(H, hymnConcat(a, newInt(increment)))
        } else {
          frame = hymnThrowError(H, "Increment: 1st and 2nd values can't be added")
          if (frame === null) return
          else break
        }
        break
      }
      case OP_SUBTRACT: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        if (isInt(a)) {
          if (isInt(b)) {
            a.value -= b.value
            hymnPush(H, a)
          } else if (isFloat(b)) {
            a.value -= b.value
            hymnPush(H, a)
          } else {
            frame = hymnThrowError(H, 'Subtract: 2nd value must be `Integer` or `Float`.')
            if (frame === null) return
            else break
          }
        } else if (isFloat(a)) {
          if (isInt(b)) {
            a.value -= b.value
            hymnPush(H, a)
          } else if (isFloat(b)) {
            a.value -= b.value
            hymnPush(H, a)
          } else {
            frame = hymnThrowError(H, 'Subtract: 1st and 2nd values must be `Integer` or `Float`.')
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, 'Subtract: 1st and 2nd values must be `Integer` or `Float`.')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_MULTIPLY: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        if (isInt(a)) {
          if (isInt(b)) {
            a.value *= b.value
            hymnPush(H, a)
          } else if (isFloat(b)) {
            a.value *= b.value
            hymnPush(H, a)
          } else {
            frame = hymnThrowError(H, 'Multiply: 2nd value must be `Integer` or `Float`.')
            if (frame === null) return
            else break
          }
        } else if (isFloat(a)) {
          if (isInt(b)) {
            a.value *= b.value
            hymnPush(H, a)
          } else if (isFloat(b)) {
            a.value *= b.value
            hymnPush(H, a)
          } else {
            frame = hymnThrowError(H, 'Multiply: 1st and 2nd values must be `Integer` or `Float`.')
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, 'Multiply: 1st and 2nd values must be `Integer` or `Float`.')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_DIVIDE: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        if (isInt(a)) {
          if (isInt(b)) {
            a.value /= b.value
            hymnPush(H, a)
          } else if (isFloat(b)) {
            a.value /= b.value
            hymnPush(H, a)
          } else {
            frame = hymnThrowError(H, 'Divide: 2nd value must be `Integer` or `Float`.')
            if (frame === null) return
            else break
          }
        } else if (isFloat(a)) {
          if (isInt(b)) {
            a.value /= b.value
            hymnPush(H, a)
          } else if (isFloat(b)) {
            a.value /= b.value
            hymnPush(H, a)
          } else {
            frame = hymnThrowError(H, 'Divide: 1st and 2nd values must be `Integer` or `Float`.')
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, 'Divide: 1st and 2nd values must be `Integer` or `Float`.')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_MODULO: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        if (isInt(a)) {
          if (isInt(b)) {
            a.value %= b.value
            hymnPush(H, a)
          } else {
            frame = hymnThrowError(H, 'Modulo: 2nd value must be `Integer`.')
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, 'Modulo: 1st and 2nd values must be `Integer`.')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_BIT_NOT: {
        const value = hymnPop(H)
        if (isInt(value)) {
          value.value = ~value.value
          hymnPush(H, value)
        } else {
          frame = hymnThrowError(H, 'Bitwise Not: Operand must integer.')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_BIT_OR: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        if (isInt(a)) {
          if (isInt(b)) {
            a.value |= b.value
            hymnPush(H, a)
          } else {
            frame = hymnThrowError(H, 'Bitwise Or: 2nd value must be `Integer`.')
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, 'Bitwise Or: 1st and 2nd values must be `Integer`.')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_BIT_AND: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        if (isInt(a)) {
          if (isInt(b)) {
            a.value &= b.value
            hymnPush(H, a)
          } else {
            frame = hymnThrowError(H, 'Bitwise And: 2nd value must be `Integer`.')
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, 'Bitwise And: 1st and 2nd values must be `Integer`.')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_BIT_XOR: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        if (isInt(a)) {
          if (isInt(b)) {
            a.value ^= b.value
            hymnPush(H, a)
          } else {
            frame = hymnThrowError(H, 'Bitwise Xor: 2nd value must be `Integer`.')
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, 'Bitwise Xor: 1st and 2nd values must be `Integer`.')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_BIT_LEFT_SHIFT: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        if (isInt(a)) {
          if (isInt(b)) {
            a.value <<= b.value
            hymnPush(H, a)
          } else {
            frame = hymnThrowError(H, 'Bitwise Left Shift: 2nd value must be `Integer`.')
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, 'Bitwise Left Shift: 1st and 2nd values must be `Integer`.')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_BIT_RIGHT_SHIFT: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        if (isInt(a)) {
          if (isInt(b)) {
            a.value >>= b.value
            hymnPush(H, a)
          } else {
            frame = hymnThrowError(H, 'Bitwise Right Shift: 2nd value must be `Integer`.')
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, 'Bitwise Right Shift: 1st and 2nd values must be `Integer`.')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_NEGATE: {
        const value = hymnPop(H)
        if (isInt(value)) {
          value.value = -value.value
        } else if (isFloat(value)) {
          value.value = -value.value
        } else {
          frame = hymnThrowError(H, 'Negate: Operand must be a number.')
          if (frame === null) return
          else break
        }
        hymnPush(H, value)
        break
      }
      case OP_NOT: {
        const value = hymnPop(H)
        if (isBool(value)) {
          value.value = !value.value
        } else {
          frame = hymnThrowError(H, 'Not: Operand must be a boolean.')
          if (frame === null) return
          else break
        }
        hymnPush(H, value)
        break
      }
      case OP_CONSTANT: {
        let value = readConstant(frame)
        switch (value.is) {
          case HYMN_VALUE_ARRAY: {
            value = newArrayValue([])
            break
          }
          case HYMN_VALUE_TABLE: {
            value = newTableValue(new HymnTable())
            break
          }
          default:
            break
        }
        hymnPush(H, value)
        break
      }
      case OP_DEFINE_GLOBAL: {
        const name = readConstant(frame).value
        const value = hymnPop(H)
        const previous = tablePut(H.globals, name, value)
        if (previous !== null) {
          frame = hymnThrowError(H, `Global '${name}' was previously defined.`)
          if (frame === null) return
          else break
        }
        break
      }
      case OP_SET_GLOBAL: {
        const name = readConstant(frame).value
        const value = hymnPeek(H, 1)
        const previous = tablePut(H.globals, name, value)
        if (previous === null) {
          frame = hymnThrowError(H, 'Undefined variable `' + name + '`.')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_GET_GLOBAL: {
        const name = readConstant(frame).value
        const get = tableGet(H.globals, name)
        if (get === null) {
          frame = hymnThrowError(H, 'Undefined variable `' + name + '`.')
          if (frame === null) return
          else break
        }
        hymnPush(H, get)
        break
      }
      case OP_SET_LOCAL: {
        const slot = readByte(frame)
        const value = hymnPeek(H, 1)
        H.stack[frame.stack + slot] = value
        break
      }
      case OP_GET_LOCAL: {
        const slot = readByte(frame)
        const value = H.stack[frame.stack + slot]
        hymnPush(H, value)
        break
      }
      case OP_GET_TWO_LOCAL: {
        const slotA = readByte(frame)
        const slotB = readByte(frame)
        const valueA = H.stack[frame.stack + slotA]
        const valueB = H.stack[frame.stack + slotB]
        hymnPush(H, valueA)
        hymnPush(H, valueB)
        break
      }
      case OP_INCREMENT_LOCAL: {
        const slot = readByte(frame)
        const increment = readByte(frame)
        const value = cloneValue(H.stack[frame.stack + slot])
        if (isInt(value) || isFloat(value)) {
          value.value += increment
        } else {
          frame = hymnThrowError(H, 'Increment Local: Expected `Number` but was `' + valueName(value.is) + '`')
          if (frame === null) return
          else break
        }
        hymnPush(H, value)
        break
      }
      case OP_INCREMENT_LOCAL_AND_SET: {
        const slot = readByte(frame)
        const increment = readByte(frame)
        const value = cloneValue(H.stack[frame.stack + slot])
        if (isInt(value) || isFloat(value)) {
          value.value += increment
        } else {
          frame = hymnThrowError(H, 'Get and Set Local: Expected `Number` but was `' + valueName(value.is) + '`')
          if (frame === null) return
          else break
        }
        H.stack[frame.stack + slot] = value
        break
      }
      case OP_SET_PROPERTY: {
        const value = hymnPop(H)
        const tableValue = hymnPop(H)
        if (!isTable(tableValue)) {
          frame = hymnThrowError(H, 'Set Property: Only tables can set properties.')
          if (frame === null) return
          else break
        }
        const table = tableValue.value
        const name = readConstant(frame).value
        tablePut(table, name, value)
        hymnPush(H, value)
        break
      }
      case OP_GET_PROPERTY: {
        const v = hymnPop(H)
        if (!isTable(v)) {
          frame = hymnThrowError(H, 'Only tables can get properties.')
          if (frame === null) return
          else break
        }
        const table = v.value
        const name = readConstant(frame).value
        const g = tableGet(table, name)
        if (g === null) hymnPush(H, newNone())
        else hymnPush(H, g)
        break
      }
      case OP_SET_DYNAMIC: {
        const s = hymnPop(H)
        const i = hymnPop(H)
        const v = hymnPop(H)
        if (isArray(v)) {
          if (!isInt(i)) {
            frame = hymnThrowError(H, 'Integer required to set array index.')
            if (frame === null) return
            else break
          }
          const array = v.value
          const size = array.length
          let index = i.value
          if (index > size) {
            frame = hymnThrowError(H, 'Array index out of bounds ' + index + ' > ' + size + '.')
            if (frame === null) return
            else break
          }
          if (index < 0) {
            index = size + index
            if (index < 0) {
              frame = hymnThrowError(H, 'Array index out of bounds ' + index + '.')
              if (frame === null) return
              else break
            }
          }
          if (index === size) {
            array.push(s)
          } else {
            array[index] = s
          }
        } else if (isTable(v)) {
          if (!isString(i)) {
            frame = hymnThrowError(H, 'String required to set table property.')
            if (frame === null) return
            else break
          }
          const table = v.value
          const name = i.value
          tablePut(table, name, s)
        } else {
          frame = hymnThrowError(H, 'Dynamic Set: 1st argument requires `Array` or `Table`, but was `' + valueName(v.is) + '`.')
          if (frame === null) return
          else break
        }
        hymnPush(H, s)
        break
      }
      case OP_GET_DYNAMIC: {
        const i = hymnPop(H)
        const v = hymnPop(H)
        switch (v.is) {
          case HYMN_VALUE_STRING: {
            if (!isInt(i)) {
              frame = hymnThrowError(H, 'Integer required to get string character from index.')
              if (frame === null) return
              else break
            }
            const string = v.value
            const size = string.length
            let index = i.value
            if (index >= size) {
              frame = hymnThrowError(H, 'String index out of bounds %d >= %d.', index, size)
              if (frame === null) return
              else break
            }
            if (index < 0) {
              index = size + index
              if (index < 0) {
                frame = hymnThrowError(H, 'String index out of bounds %d.', index)
                if (frame === null) return
                else break
              }
            }
            const c = string[index]
            hymnPush(H, newString(String(c)))
            break
          }
          case HYMN_VALUE_ARRAY: {
            if (!isInt(i)) {
              frame = hymnThrowError(H, 'Integer required to get array index.')
              if (frame === null) return
              else break
            }
            const array = v.value
            const size = array.length
            let index = i.value
            if (index >= size) {
              frame = hymnThrowError(H, 'Array index out of bounds %d >= %d.', index, size)
              if (frame === null) return
              else break
            }
            if (index < 0) {
              index = size + index
              if (index < 0) {
                frame = hymnThrowError(H, 'Array index out of bounds %d.', index)
                if (frame === null) return
                else break
              }
            }
            const g = array[index]
            hymnPush(H, g)
            break
          }
          case HYMN_VALUE_TABLE: {
            if (!isString(i)) {
              const is = valueName(i.is)
              frame = hymnThrowError(H, 'Dynamic Get: Expected 2nd argument to be `String`, but was `' + is + '`.')
              if (frame === null) return
              else break
            }
            const table = v.value
            const name = i.value
            const g = tableGet(table, name)
            if (g === null) hymnPush(H, newNone())
            else hymnPush(H, g)
            break
          }
          default: {
            const is = valueName(v.is)
            frame = hymnThrowError(H, 'Dynamic Get: 1st argument requires `Array` or `Table`, but was `%s`.', is)
            if (frame === null) return
            else break
          }
        }
        break
      }
      case OP_LEN: {
        const value = hymnPop(H)
        switch (value.is) {
          case HYMN_VALUE_STRING: {
            const len = value.value.length
            hymnPush(H, newInt(len))
            break
          }
          case HYMN_VALUE_ARRAY: {
            const len = value.value.length
            hymnPush(H, newInt(len))
            break
          }
          case HYMN_VALUE_TABLE: {
            const len = value.value.size
            hymnPush(H, newInt(len))
            break
          }
          default:
            frame = hymnThrowError(H, 'Expected array or table for `len` function.')
            if (frame === null) return
            else break
        }

        break
      }
      case OP_ARRAY_POP: {
        const a = hymnPop(H)
        if (!isArray(a)) {
          const is = valueName(a.is)
          frame = hymnThrowError(H, 'Pop Function: Expected `Array` for 1st argument, but was `%s`.', is)
          if (frame === null) return
          else break
        } else {
          const value = a.value.pop()
          hymnPush(H, value)
        }
        break
      }
      case OP_ARRAY_PUSH: {
        const v = hymnPop(H)
        const a = hymnPop(H)
        if (!isArray(a)) {
          const is = valueName(v.is)
          frame = hymnThrowError(H, 'Push Function: Expected `Array` for 1st argument, but was `%s`.', is)
          if (frame === null) return
          else break
        } else {
          a.value.push(v)
          hymnPush(H, v)
        }
        break
      }
      case OP_ARRAY_INSERT: {
        const p = hymnPop(H)
        const i = hymnPop(H)
        const v = hymnPop(H)
        if (isArray(v)) {
          if (!isInt(i)) {
            const is = valueName(i.is)
            frame = hymnThrowError(H, 'Insert Function: Expected `Integer` for 2nd argument, but was `%s`.', is)
            if (frame === null) return
            else break
          }
          const array = v.value
          const size = array.length
          let index = i.value
          if (index > size) {
            frame = hymnThrowError(H, `Insert Function: Array index out of bounds: ${index} > ${size}`)
            if (frame === null) return
            else break
          }
          if (index < 0) {
            index = size + index
            if (index < 0) {
              frame = hymnThrowError(H, 'Insert Function: Array index less than zero: %d', index)
              if (frame === null) return
              else break
            }
          }
          if (index === size) {
            array.push(p)
          } else {
            array.splice(index, 0, p)
          }
          hymnPush(H, p)
        } else {
          const is = valueName(v.is)
          frame = hymnThrowError(H, 'Insert Function: Expected `Array` for 1st argument, but was `%s`.', is)
          if (frame === null) return
          else break
        }
        break
      }
      case OP_DELETE: {
        const i = hymnPop(H)
        const v = hymnPop(H)
        if (isArray(v)) {
          if (!isInt(i)) {
            frame = hymnThrowError(H, 'Integer required to delete from array.')
            if (frame === null) return
            else break
          }
          const array = v.value
          const size = array.length
          let index = i.value
          if (index >= size) {
            frame = hymnThrowError(H, `Delete Function: Array index out of bounds ${index} > ${size}.`)
            if (frame === null) return
            else break
          }
          if (index < 0) {
            index = size + index
            if (index < 0) {
              frame = hymnThrowError(H, 'Array index out of bounds %d.', index)
              if (frame === null) return
              else break
            }
          }
          const value = array.splice(index, 1)[0]
          hymnPush(H, value)
        } else if (isTable(v)) {
          if (!isString(i)) {
            frame = hymnThrowError(H, 'String required to delete from table.')
            if (frame === null) return
            else break
          }
          const table = v.value
          const name = i.value
          const value = tableGet(table, name)
          if (value !== null) {
            tableRemove(table, name)
            hymnPush(H, value)
          } else {
            hymnPush(H, newNone())
          }
        } else {
          frame = hymnThrowError(H, 'Expected array or table for `delete` function.')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_COPY: {
        const value = hymnPop(H)
        switch (value.is) {
          case HYMN_VALUE_NONE:
          case HYMN_VALUE_BOOL:
          case HYMN_VALUE_INTEGER:
          case HYMN_VALUE_FLOAT:
          case HYMN_VALUE_STRING:
          case HYMN_VALUE_FUNC:
          case HYMN_VALUE_FUNC_NATIVE:
            hymnPush(H, value)
            break
          case HYMN_VALUE_ARRAY: {
            const copy = value.value.slice()
            hymnPush(H, newArrayValue(copy))
            break
          }
          case HYMN_VALUE_TABLE: {
            const copy = newTableCopy(value.value)
            hymnPush(H, newTableValue(copy))
            break
          }
          default:
            hymnPush(H, newNone())
        }
        break
      }
      case OP_SLICE: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        const v = hymnPop(H)
        if (!isInt(a)) {
          frame = hymnThrowError(H, 'Integer required for slice expression.')
          if (frame === null) return
          else break
        }
        const start = a.value
        if (isString(v)) {
          const original = v.value
          const size = original.length
          let end
          if (isInt(b)) {
            end = b.value
          } else if (isNone(b)) {
            end = size
          } else {
            frame = hymnThrowError(H, 'Integer required for slice expression.')
            if (frame === null) return
            else break
          }
          if (end > size) {
            frame = hymnThrowError(H, 'String index out of bounds %d > %d.', end, size)
            if (frame === null) return
            else break
          }
          if (end < 0) {
            end = size + end
            if (end < 0) {
              frame = hymnThrowError(H, 'String index out of bounds %d.', end)
              if (frame === null) return
              else break
            }
          }
          if (start >= end) {
            frame = hymnThrowError(H, 'String start index %d > end index %d.', start, end)
            if (frame === null) return
            else break
          }
          const sub = original.substring(start, end)
          hymnPush(H, newString(sub))
        } else if (isArray(v)) {
          const array = v.value
          const size = array.length
          let end
          if (isInt(b)) {
            end = b.value
          } else if (isNone(b)) {
            end = size
          } else {
            frame = hymnThrowError(H, 'Integer required for slice expression.')
            if (frame === null) return
            else break
          }
          if (end > size) {
            frame = hymnThrowError(H, 'Array index out of bounds %d > %d.', end, size)
            if (frame === null) return
            else break
          }
          if (end < 0) {
            end = size + end
            if (end < 0) {
              frame = hymnThrowError(H, 'Array index out of bounds %d.', end)
              if (frame === null) return
              else break
            }
          }
          if (start >= end) {
            frame = hymnThrowError(H, 'Array start index %d >= end index %d.', start, end)
            if (frame === null) return
            else break
          }
          const copy = array.slice(start, end)
          hymnPush(H, newArrayValue(copy))
        } else {
          frame = hymnThrowError(H, 'Expected string or array for `slice` function.')
          if (frame === null) return
          else break
        }
        break
      }
      case OP_CLEAR: {
        const value = hymnPop(H)
        switch (value.is) {
          case HYMN_VALUE_BOOL:
            hymnPush(H, newBool(false))
            break
          case HYMN_VALUE_INTEGER:
            hymnPush(H, newInt(0))
            break
          case HYMN_VALUE_FLOAT:
            hymnPush(H, newFloat(0.0))
            break
          case HYMN_VALUE_STRING:
            hymnPush(H, newString(''))
            break
          case HYMN_VALUE_ARRAY: {
            const array = value.value
            array.length = 0
            hymnPush(H, value)
            break
          }
          case HYMN_VALUE_TABLE: {
            const table = value.value
            tableClear(table)
            hymnPush(H, value)
            break
          }
          case HYMN_VALUE_NONE:
          case HYMN_VALUE_FUNC:
          case HYMN_VALUE_FUNC_NATIVE:
          case HYMN_VALUE_POINTER:
            hymnPush(H, newNone())
            break
        }
        break
      }
      case OP_KEYS: {
        const value = hymnPop(H)
        if (!isTable(value)) {
          frame = hymnThrowError(H, 'Expected table for `keys` function.')
          if (frame === null) return
          else break
        } else {
          const table = value.value
          const keys = tableKeys(table)
          hymnPush(H, newArrayValue(keys))
        }
        break
      }
      case OP_INDEX: {
        const b = hymnPop(H)
        const a = hymnPop(H)
        switch (a.is) {
          case HYMN_VALUE_STRING: {
            if (!isString(b)) {
              frame = hymnThrowError(H, 'Expected substring for 2nd argument of `index` function.')
              if (frame === null) return
              else break
            }
            const index = a.value.indexOf(b.value)
            hymnPush(H, newInt(index))
            break
          }
          case HYMN_VALUE_ARRAY:
            hymnPush(H, newInt(arrayIndexOf(a.value, b)))
            break
          case HYMN_VALUE_TABLE: {
            const key = tableKeyOf(a.value, b)
            if (key === null) hymnPush(H, newNone())
            else hymnPush(H, newString(key))
            break
          }
          default:
            frame = hymnThrowError(H, 'Index function expects `String`, `Array`, or `Table`')
            if (frame === null) return
            else break
        }
        break
      }
      case OP_TYPE: {
        const value = hymnPop(H)
        switch (value.is) {
          case HYMN_VALUE_NONE:
            hymnPush(H, newString(STRING_NONE))
            break
          case HYMN_VALUE_BOOL:
            hymnPush(H, newString(STRING_BOOL))
            break
          case HYMN_VALUE_INTEGER:
            hymnPush(H, newString(STRING_INTEGER))
            break
          case HYMN_VALUE_FLOAT:
            hymnPush(H, newString(STRING_FLOAT))
            break
          case HYMN_VALUE_STRING:
            hymnPush(H, newString(STRING_STRING))
            break
          case HYMN_VALUE_ARRAY:
            hymnPush(H, newString(STRING_ARRAY))
            break
          case HYMN_VALUE_TABLE:
            hymnPush(H, newString(STRING_TABLE))
            break
          case HYMN_VALUE_FUNC:
            hymnPush(H, newString(STRING_FUNC))
            break
          case HYMN_VALUE_FUNC_NATIVE:
            hymnPush(H, newString(STRING_NATIVE))
            break
          case HYMN_VALUE_POINTER:
            hymnPush(H, newString(STRING_POINTER))
            break
        }
        break
      }
      case OP_TO_INTEGER: {
        const value = hymnPop(H)
        if (isInt(value)) {
          hymnPush(H, value)
        } else if (isFloat(value)) {
          hymnPush(H, newInt(parseInt(value.value)))
        } else if (isString(value)) {
          const number = Number(value.value)
          if (isNaN(number)) {
            hymnPush(H, newNone())
          } else {
            hymnPush(H, newInt(parseInt(number)))
          }
        } else {
          frame = hymnThrowError(H, "Can't cast to an integer.")
          if (frame === null) return
          else break
        }
        break
      }
      case OP_TO_FLOAT: {
        const value = hymnPop(H)
        if (isInt(value)) {
          hymnPush(H, newFloat(parseFloat(value.value)))
        } else if (isFloat(value)) {
          hymnPush(H, value)
        } else if (isString(value)) {
          const number = Number(value.value)
          if (isNaN(number)) {
            hymnPush(H, newNone())
          } else {
            hymnPush(H, newFloat(number))
          }
        } else {
          frame = hymnThrowError(H, "Can't cast to a float.")
          if (frame === null) return
          else break
        }
        break
      }
      case OP_TO_STRING: {
        const value = hymnPop(H)
        hymnPush(H, newString(valueToString(value)))
        break
      }
      case OP_PRINT: {
        const value = hymnPop(H)
        H.print(valueToString(value))
        break
      }
      case OP_THROW: {
        frame = hymnException(H)
        if (frame === null) return
        break
      }
      case OP_DUPLICATE: {
        const top = hymnPeek(H, 1)
        hymnPush(H, top)
        break
      }
      case OP_USE: {
        const file = hymnPop(H)
        if (isString(file)) {
          frame = await hymnImport(H, file.value)
          if (frame === null) return
        } else {
          frame = hymnThrowError(H, "Expected string for 'use' command.")
          if (frame === null) return
          else break
        }
        break
      }
      default:
        console.error('Unknown instruction')
        return
    }
  }
}

function addFunction(H, name, func) {
  const value = newNativeFunction(name, func)
  tablePut(H.globals, name, newFuncNativeValue(value))
}

function addPointer(H, name, pointer) {
  tablePut(H.globals, name, newPointerValue(pointer))
}

async function debugScript(H, script, source) {
  const result = compile(H, script, source)

  const func = result.func
  if (result.error !== null) {
    return result.error
  }

  disassembleByteCode(func.code, script)

  const constants = func.code.constants
  for (let i = 0; i < constants.length; i++) {
    const constant = constants[i]
    if (isFunc(constant)) {
      const value = constant.value
      disassembleByteCode(value.code, value.name)
    }
  }

  return null
}

async function interpretScript(H, script, source) {
  const result = compile(H, script, source)

  const func = result.func
  if (result.error !== null) {
    return result.error
  }

  const funcVal = newFuncValue(func)
  hymnPush(H, funcVal)
  hymnCall(H, func, 0)

  await hymnRun(H)
  if (H.error !== null) {
    return H.error
  }

  hymnResetStack(H)
  return null
}

async function interpret(H, source) {
  return interpretScript(H, null, source)
}

function newVM() {
  const H = new Hymn()

  if (node) {
    H.paths.push(newString('<parent>/<path>.hm'))
    H.paths.push(newString('./<path>.hm'))
    H.paths.push(newString('./libs/<path>.hm'))
  } else {
    const address = window.location.href
    const url = address.substring(0, address.lastIndexOf('/') + 1)
    H.paths.push(newString(url + '<path>.hm'))
    H.paths.push(newString(url + 'libs/<path>.hm'))
    H.paths.push(newString('/<path>.hm'))
    H.paths.push(newString('/libs/<path>.hm'))
  }

  tablePut(H.globals, '__paths', newArrayValue(H.paths))
  tablePut(H.globals, '__imports', newTableValue(H.imports))

  return H
}

if (node) {
  module.exports = {
    isFloat: isFloat,
    isFuncNative: isFuncNative,
    isPointer: isPointer,
    newNone: newNone,
    newFloat: newFloat,
    newFuncNativeValue: newFuncNativeValue,
    newPointerValue: newPointerValue,
    newFunction: newFunction,
    newNativeFunction: newNativeFunction,
    addFunction: addFunction,
    addPointer: addPointer,
    newVM: newVM,
    interpret: interpret,
    interpretScript: interpretScript,
    debug: debugScript,
  }
}
