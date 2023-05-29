/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

const HYMN_VERSION = '0.10.0'

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
const nodeFs = node ? require('fs') : null
const nodePath = node ? require('path') : null
const nodeProcess = node ? require('process') : null

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

function copyValue(original) {
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
    this.source = null
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

function printOut(text) {
  if (node) nodeProcess.stdout.write(text)
  else console.log(text)
}

function printError(text) {
  if (node) nodeProcess.stderr.write(text)
  else console.error(text)
}

function printLine(text) {
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
    this.printError = printError
    this.printLine = printLine
  }
}

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
const TOKEN_BIT_AND = 13
const TOKEN_BIT_LEFT_SHIFT = 14
const TOKEN_BIT_NOT = 15
const TOKEN_BIT_OR = 16
const TOKEN_BIT_RIGHT_SHIFT = 17
const TOKEN_BIT_XOR = 18
const TOKEN_BREAK = 19
const TOKEN_CLEAR = 20
const TOKEN_COLON = 21
const TOKEN_COMMA = 22
const TOKEN_CONTINUE = 23
const TOKEN_COPY = 24
const TOKEN_DELETE = 25
const TOKEN_DIVIDE = 26
const TOKEN_DOT = 27
const TOKEN_ECHO = 28
const TOKEN_ELIF = 29
const TOKEN_ELSE = 30
const TOKEN_EOF = 31
const TOKEN_EQUAL = 32
const TOKEN_ERROR = 33
const TOKEN_EXCEPT = 34
const TOKEN_EXISTS = 35
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
const TOKEN_SRC = 47
const TOKEN_INTEGER = 48
const TOKEN_KEYS = 49
const TOKEN_LEFT_CURLY = 50
const TOKEN_LEFT_PAREN = 51
const TOKEN_LEFT_SQUARE = 52
const TOKEN_LEN = 53
const TOKEN_LESS = 54
const TOKEN_LESS_EQUAL = 55
const TOKEN_SET = 56
const TOKEN_MODULO = 57
const TOKEN_MULTIPLY = 58
const TOKEN_NONE = 59
const TOKEN_NOT = 60
const TOKEN_NOT_EQUAL = 61
const TOKEN_OR = 62
const TOKEN_POINTER = 63
const TOKEN_POP = 64
const TOKEN_PRINT = 65
const TOKEN_PUSH = 66
const TOKEN_RETURN = 67
const TOKEN_RIGHT_CURLY = 68
const TOKEN_RIGHT_PAREN = 69
const TOKEN_RIGHT_SQUARE = 70
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
const TOKEN_OPCODES = 84
const TOKEN_STACK = 85
const TOKEN_REFERENCE = 86

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

const STRING_STATUS_NONE = 0
const STRING_STATUS_BEGIN = 1
const STRING_STATUS_ADD = 2
const STRING_STATUS_CLOSE = 3
const STRING_STATUS_CONTINUE = 4

const OP_ADD = 0
const OP_INSERT = 1
const OP_ARRAY_POP = 2
const OP_ARRAY_PUSH = 3
const OP_BIT_AND = 4
const OP_BIT_LEFT_SHIFT = 5
const OP_BIT_NOT = 6
const OP_BIT_OR = 7
const OP_BIT_RIGHT_SHIFT = 8
const OP_BIT_XOR = 9
const OP_CALL = 10
const OP_CLEAR = 11
const OP_CONSTANT = 12
const OP_COPY = 13
const OP_DEFINE_GLOBAL = 14
const OP_DELETE = 15
const OP_DIVIDE = 16
const OP_DUPLICATE = 17
const OP_ECHO = 18
const OP_EQUAL = 19
const OP_EXISTS = 20
const OP_FALSE = 21
const OP_FLOAT = 22
const OP_FOR = 23
const OP_FOR_LOOP = 24
const OP_GET_DYNAMIC = 25
const OP_GET_GLOBAL = 26
const OP_GET_LOCAL = 27
const OP_GET_PROPERTY = 28
const OP_GREATER = 29
const OP_GREATER_EQUAL = 30
const OP_INCREMENT_LOCAL_AND_SET = 31
const OP_INDEX = 32
const OP_SOURCE = 33
const OP_INT = 34
const OP_JUMP = 35
const OP_JUMP_IF_FALSE = 36
const OP_JUMP_IF_TRUE = 37
const OP_KEYS = 38
const OP_LEN = 39
const OP_LESS = 40
const OP_LESS_EQUAL = 41
const OP_LOOP = 42
const OP_MODULO = 43
const OP_MULTIPLY = 44
const OP_NEGATE = 45
const OP_NEW_ARRAY = 46
const OP_NEW_TABLE = 47
const OP_NONE = 48
const OP_NOT = 49
const OP_NOT_EQUAL = 50
const OP_POP = 51
const OP_PRINT = 52
const OP_RETURN = 53
const OP_SELF = 54
const OP_SET_DYNAMIC = 55
const OP_SET_GLOBAL = 56
const OP_SET_LOCAL = 57
const OP_SET_PROPERTY = 58
const OP_SLICE = 59
const OP_STRING = 60
const OP_SUBTRACT = 61
const OP_THROW = 62
const OP_TRUE = 63
const OP_TYPE = 64
const OP_USE = 65
const OP_CODES = 66
const OP_STACK = 67
const OP_REFERENCE = 68

const TYPE_FUNCTION = 0
const TYPE_SCRIPT = 1
const TYPE_DIRECT = 2

class Token {
  constructor() {
    this.type = TOKEN_UNDEFINED
    this.row = 0
    this.column = 0
    this.start = 0
    this.length = 0
    this.number = 0
  }
}

function copyToken(dest, src) {
  dest.type = src.type
  dest.row = src.row
  dest.column = src.column
  dest.start = src.start
  dest.length = src.length
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
    this.begin = 0
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
    this.stringFormat = 0
    this.stringStatus = STRING_STATUS_NONE
    this.H = H
    this.scope = null
    this.pop = -1
    this.barrier = -1
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
rules[TOKEN_BIT_AND] = new Rule(null, compileBinary, PRECEDENCE_BITS)
rules[TOKEN_BIT_LEFT_SHIFT] = new Rule(null, compileBinary, PRECEDENCE_BITS)
rules[TOKEN_BIT_NOT] = new Rule(compileUnary, null, PRECEDENCE_NONE)
rules[TOKEN_BIT_OR] = new Rule(null, compileBinary, PRECEDENCE_BITS)
rules[TOKEN_BIT_RIGHT_SHIFT] = new Rule(null, compileBinary, PRECEDENCE_BITS)
rules[TOKEN_BIT_XOR] = new Rule(null, compileBinary, PRECEDENCE_BITS)
rules[TOKEN_BREAK] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_CLEAR] = new Rule(clearExpression, null, PRECEDENCE_NONE)
rules[TOKEN_COLON] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_COMMA] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_CONTINUE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_COPY] = new Rule(copyExpression, null, PRECEDENCE_NONE)
rules[TOKEN_OPCODES] = new Rule(opCodesExpression, null, PRECEDENCE_NONE)
rules[TOKEN_STACK] = new Rule(stackExpression, null, PRECEDENCE_NONE)
rules[TOKEN_REFERENCE] = new Rule(referenceExpression, null, PRECEDENCE_NONE)
rules[TOKEN_DELETE] = new Rule(deleteExpression, null, PRECEDENCE_NONE)
rules[TOKEN_DIVIDE] = new Rule(null, compileBinary, PRECEDENCE_FACTOR)
rules[TOKEN_DOT] = new Rule(null, compileDot, PRECEDENCE_CALL)
rules[TOKEN_ECHO] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_ELIF] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_ELSE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_EOF] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_EQUAL] = new Rule(null, compileBinary, PRECEDENCE_EQUALITY)
rules[TOKEN_ERROR] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_EXCEPT] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_EXISTS] = new Rule(existsExpression, null, PRECEDENCE_NONE)
rules[TOKEN_FALSE] = new Rule(compileFalse, null, PRECEDENCE_NONE)
rules[TOKEN_FLOAT] = new Rule(compileFloat, null, PRECEDENCE_NONE)
rules[TOKEN_FOR] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_FUNCTION] = new Rule(functionExpression, null, PRECEDENCE_NONE)
rules[TOKEN_GREATER] = new Rule(null, compileBinary, PRECEDENCE_COMPARE)
rules[TOKEN_GREATER_EQUAL] = new Rule(null, compileBinary, PRECEDENCE_COMPARE)
rules[TOKEN_IDENT] = new Rule(compileVariable, null, PRECEDENCE_NONE)
rules[TOKEN_IF] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_IN] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_INDEX] = new Rule(indexExpression, null, PRECEDENCE_NONE)
rules[TOKEN_INSERT] = new Rule(arrayInsertExpression, null, PRECEDENCE_NONE)
rules[TOKEN_SRC] = new Rule(sourceExpression, null, PRECEDENCE_NONE)
rules[TOKEN_INTEGER] = new Rule(compileInteger, null, PRECEDENCE_NONE)
rules[TOKEN_KEYS] = new Rule(keysExpression, null, PRECEDENCE_NONE)
rules[TOKEN_LEFT_CURLY] = new Rule(compileTable, null, PRECEDENCE_NONE)
rules[TOKEN_LEFT_PAREN] = new Rule(compileGroup, compileCall, PRECEDENCE_CALL)
rules[TOKEN_LEFT_SQUARE] = new Rule(compileArray, compileSquare, PRECEDENCE_CALL)
rules[TOKEN_LEN] = new Rule(lenExpression, null, PRECEDENCE_NONE)
rules[TOKEN_LESS] = new Rule(null, compileBinary, PRECEDENCE_COMPARE)
rules[TOKEN_LESS_EQUAL] = new Rule(null, compileBinary, PRECEDENCE_COMPARE)
rules[TOKEN_SET] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_MODULO] = new Rule(null, compileBinary, PRECEDENCE_FACTOR)
rules[TOKEN_MULTIPLY] = new Rule(null, compileBinary, PRECEDENCE_FACTOR)
rules[TOKEN_NONE] = new Rule(compileNone, null, PRECEDENCE_NONE)
rules[TOKEN_NOT] = new Rule(compileUnary, null, PRECEDENCE_NONE)
rules[TOKEN_NOT_EQUAL] = new Rule(null, compileBinary, PRECEDENCE_EQUALITY)
rules[TOKEN_OR] = new Rule(null, compileOr, PRECEDENCE_OR)
rules[TOKEN_POINTER] = new Rule(null, compilePointer, PRECEDENCE_CALL)
rules[TOKEN_POP] = new Rule(arrayPopExpression, null, PRECEDENCE_NONE)
rules[TOKEN_PRINT] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_PUSH] = new Rule(arrayPushExpression, null, PRECEDENCE_NONE)
rules[TOKEN_RETURN] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_RIGHT_CURLY] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_RIGHT_PAREN] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_RIGHT_SQUARE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_STRING] = new Rule(compileString, null, PRECEDENCE_NONE)
rules[TOKEN_SUBTRACT] = new Rule(compileUnary, compileBinary, PRECEDENCE_TERM)
rules[TOKEN_THROW] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_TO_FLOAT] = new Rule(castFloatExpression, null, PRECEDENCE_NONE)
rules[TOKEN_TO_INTEGER] = new Rule(castIntegerExpression, null, PRECEDENCE_NONE)
rules[TOKEN_TO_STRING] = new Rule(castStringExpression, null, PRECEDENCE_NONE)
rules[TOKEN_TRUE] = new Rule(compileTrue, null, PRECEDENCE_NONE)
rules[TOKEN_TRY] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_TYPE_FUNC] = new Rule(typeExpression, null, PRECEDENCE_NONE)
rules[TOKEN_UNDEFINED] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_USE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_VALUE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_WHILE] = new Rule(null, null, PRECEDENCE_NONE)

function valueType(type) {
  switch (type) {
    case HYMN_VALUE_NONE:
      return 'none'
    case HYMN_VALUE_BOOL:
      return 'boolean'
    case HYMN_VALUE_INTEGER:
      return 'integer'
    case HYMN_VALUE_FLOAT:
      return 'float'
    case HYMN_VALUE_STRING:
      return 'string'
    case HYMN_VALUE_ARRAY:
      return 'array'
    case HYMN_VALUE_TABLE:
      return 'table'
    case HYMN_VALUE_FUNC:
      return 'function'
    case HYMN_VALUE_FUNC_NATIVE:
      return 'native'
    case HYMN_VALUE_POINTER:
      return 'pointer'
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
      tablePut(copy, item.key, copyValue(item.value))
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

function current(C) {
  return C.scope.func.code
}

function compileError(C, token, format) {
  if (C.error !== null) return

  let error = format

  if (token.type !== TOKEN_EOF && token.length > 0) {
    const source = C.source

    let begin = token.start
    while (true) {
      if (source[begin] === '\n') {
        begin++
        break
      }
      if (begin === 0) break
      begin--
    }

    while (true) {
      if (source[begin] !== ' ' || begin === source.length) break
      begin++
    }

    let end = token.start
    while (true) {
      if (source[end] === '\n' || end === source.length) break
      end++
    }

    if (begin < end) {
      error += '\n  ' + source.substring(begin, end) + '\n  '
      const spaces = token.start - begin
      if (spaces > 0) for (let i = 0; i < spaces; i++) error += ' '
      for (let i = 0; i < token.length; i++) error += '^'
    }
  }

  error += '\n  at ' + (C.script === null ? 'script' : C.script) + ':' + token.row

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

function peekTwoChar(C) {
  if (C.pos + 1 >= C.source.length) return '\0'
  return C.source[C.pos + 1]
}

function token(C, type) {
  const token = C.current
  token.type = type
  token.row = C.row
  token.column = C.column
  token.start = C.pos - 1
  token.length = 1
}

function tokenSpecial(C, type, offset, len) {
  const token = C.current
  token.type = type
  token.row = C.row
  token.column = C.column
  token.start = C.pos - offset
  token.length = len
}

function valueToken(C, type, start, end) {
  const token = C.current
  token.type = type
  token.row = C.row
  token.column = C.column
  token.start = start
  token.length = end - start
}

function numberToken(C, type, start, end, value) {
  const token = C.current
  token.type = type
  token.row = C.row
  token.column = C.column
  token.start = start
  token.length = end - start
  token.number = value
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
      if (size === 5) return identTrie(ident, 1, 'reak', TOKEN_BREAK)
      break
    case 'd':
      if (size === 6) return identTrie(ident, 1, 'elete', TOKEN_DELETE)
      break
    case 'r':
      if (size === 6) return identTrie(ident, 1, 'eturn', TOKEN_RETURN)
      break
    case 's':
      if (size === 3) {
        if (ident[1] === 'e' && ident[2] === 't') return TOKEN_SET
        if (ident[1] === 't' && ident[2] === 'r') return TOKEN_TO_STRING
      }
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
      if (size === 3) return identTrie(ident, 1, 'en', TOKEN_LEN)
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
      if (size === 6) {
        if (ident[1] === 'x') {
          if (ident[2] === 'c') return identTrie(ident, 3, 'ept', TOKEN_EXCEPT)
          if (ident[2] === 'i') return identTrie(ident, 3, 'sts', TOKEN_EXISTS)
        }
      } else if (size === 4) {
        if (ident[1] === 'l') {
          if (ident[2] === 's') {
            if (ident[3] === 'e') {
              return TOKEN_ELSE
            }
          } else if (ident[2] === 'i' && ident[3] === 'f') {
            return TOKEN_ELIF
          }
        } else if (ident[1] === 'c') {
          return identTrie(ident, 2, 'ho', TOKEN_ECHO)
        }
      }
      break
    case 'f':
      if (size === 3) return identTrie(ident, 1, 'or', TOKEN_FOR)
      if (size === 4) return identTrie(ident, 1, 'unc', TOKEN_FUNCTION)
      if (size === 5) {
        if (ident[1] === 'a') return identTrie(ident, 2, 'lse', TOKEN_FALSE)
        if (ident[1] === 'l') return identTrie(ident, 2, 'oat', TOKEN_TO_FLOAT)
      }
      break
    case '_':
      if (size === 6) return identTrie(ident, 1, 'stack', TOKEN_STACK)
      if (size === 7) return identTrie(ident, 1, 'source', TOKEN_SRC)
      if (size === 8) return identTrie(ident, 1, 'opcodes', TOKEN_OPCODES)
      if (size === 10) return identTrie(ident, 1, 'reference', TOKEN_REFERENCE)
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
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c === '_'
}

function stringStatus(C) {
  let i = C.pos
  const source = C.source
  const size = source.length
  let expression = false
  let brackets = 1
  while (true) {
    if (i >= size) return false
    switch (source[i]) {
      case '}':
        if (brackets > 1) {
          expression = true
          i++
          brackets--
          continue
        }
        return expression ? STRING_STATUS_BEGIN : STRING_STATUS_CONTINUE
      case '"':
        return STRING_STATUS_NONE
      case ' ':
      case '\t':
      case '\r':
      case '\n':
        i++
        continue
      case '{':
        expression = true
        i++
        brackets++
        continue
      default:
        expression = true
        i++
        continue
    }
  }
}

function parseString(C, start) {
  while (true) {
    const c = nextChar(C)
    if (c === '\\') {
      nextChar(C)
    } else if (c === '$') {
      if (peekChar(C) === '{') {
        nextChar(C)
        const status = stringStatus(C)
        if (status === STRING_STATUS_BEGIN) {
          C.stringFormat = 1
          C.stringStatus = STRING_STATUS_BEGIN
          const end = C.pos - 2
          valueToken(C, TOKEN_STRING, start, end)
          return
        } else if (status === STRING_STATUS_CONTINUE) {
          C.stringStatus = STRING_STATUS_CONTINUE
          const end = C.pos - 2
          valueToken(C, TOKEN_STRING, start, end)
          while (true) {
            const c = nextChar(C)
            if (c === '}' || c === '\0') return
          }
        } else {
          continue
        }
      }
    } else if (c === '"' || c === '\0') {
      break
    }
  }
  const end = C.pos - 1
  valueToken(C, TOKEN_STRING, start, end)
  return
}

function advance(C) {
  copyToken(C.previous, C.current)
  if (C.previous.type === TOKEN_EOF) {
    return
  }
  switch (C.stringStatus) {
    case STRING_STATUS_BEGIN:
      C.stringStatus = STRING_STATUS_ADD
      token(C, TOKEN_ADD)
      return
    case STRING_STATUS_ADD:
      C.stringStatus = STRING_STATUS_NONE
      token(C, TOKEN_LEFT_PAREN)
      return
    case STRING_STATUS_CLOSE:
      C.stringStatus = STRING_STATUS_CONTINUE
      token(C, TOKEN_ADD)
      return
    case STRING_STATUS_CONTINUE: {
      C.stringStatus = STRING_STATUS_NONE
      const start = C.pos
      parseString(C, start)
      return
    }
    default:
      break
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
      case '#': {
        nextChar(C)
        c = peekChar(C)
        while (c !== '\n' && c !== '\0') {
          nextChar(C)
          c = peekChar(C)
        }
        continue
      }
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
        if (peekChar(C) === '=') {
          nextChar(C)
          tokenSpecial(C, TOKEN_ASSIGN_SUBTRACT, 2, 2)
          return
        } else if (peekChar(C) === '>') {
          nextChar(C)
          tokenSpecial(C, TOKEN_POINTER, 2, 2)
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
        if (C.stringFormat >= 1) {
          C.stringFormat++
        }
        token(C, TOKEN_LEFT_CURLY)
        return
      case '}':
        if (C.stringFormat === 1) {
          C.stringFormat = 0
          C.stringStatus = STRING_STATUS_CLOSE
          token(C, TOKEN_RIGHT_PAREN)
          return
        } else if (C.stringFormat > 1) {
          C.stringFormat--
        }
        token(C, TOKEN_RIGHT_CURLY)
        return
      case ':':
        token(C, TOKEN_COLON)
        return
      case '\0':
        token(C, TOKEN_EOF)
        return
      case '"': {
        const start = C.pos
        parseString(C, start)
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
          if (c === '0') {
            const p = peekChar(C)
            if (p === 'b') {
              nextChar(C)
              while (true) {
                c = peekChar(C)
                if (c !== '0' && c !== '1') {
                  break
                }
                nextChar(C)
              }
              const end = C.pos
              const value = sourceSubstring(C, end - start - 2, start + 2)
              numberToken(C, TOKEN_INTEGER, start, end, parseInt(value, 2))
              return
            } else if (p === 'x') {
              nextChar(C)
              while (true) {
                c = peekChar(C)
                if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f')) {
                  break
                }
                nextChar(C)
              }
              const end = C.pos
              const value = sourceSubstring(C, end - start - 2, start + 2)
              numberToken(C, TOKEN_INTEGER, start, end, parseInt(value, 16))
              return
            }
          }
          while (isDigit(peekChar(C))) {
            nextChar(C)
          }
          const p = peekChar(C)
          if (p === '.') {
            nextChar(C)
            while (isDigit(peekChar(C))) {
              nextChar(C)
            }
            const n = peekChar(C)
            if (n === 'e' || n === 'E') {
              nextChar(C)
              const e = peekChar(C)
              if (e === '-' || e === '+') nextChar(C)
              while (isDigit(peekChar(C))) {
                nextChar(C)
              }
            }
          } else if (p === 'e' || p === 'E') {
            nextChar(C)
            const n = peekChar(C)
            if (n === '-' || n === '+') nextChar(C)
            while (isDigit(peekChar(C))) {
              nextChar(C)
            }
          }
          const end = C.pos
          const value = sourceSubstring(C, end - start, start)
          const number = Number(value)
          if (Number.isSafeInteger(number)) numberToken(C, TOKEN_INTEGER, start, end, number)
          else numberToken(C, TOKEN_FLOAT, start, end, number)
          return
        } else if (isIdent(c)) {
          const start = C.pos - 1
          while (true) {
            c = peekChar(C)
            if (isIdent(c)) {
              nextChar(C)
              continue
            } else if (c === '-') {
              if (isIdent(peekTwoChar(C))) {
                nextChar(C)
                nextChar(C)
                continue
              }
            }
            break
          }
          const end = C.pos
          pushIdentToken(C, start, end)
          return
        } else {
          token(C, TOKEN_ERROR)
          compileError(C, C.current, 'unknown character: ' + c)
        }
      }
    }
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
      if (b.is === a.is) return a.value === b.value
      return false
    default:
      return false
  }
}

function matchValues(a, b) {
  if (a.is !== b.is) return false
  if (a.is === HYMN_VALUE_NONE) return true
  return a.value === b.value
}

function scopeGetLocal(scope, index) {
  if (index < scope.locals.length) {
    return scope.locals[index]
  }
  const local = new Local()
  scope.locals.push(local)
  return local
}

function scopeInit(C, scope, type, begin) {
  scope.enclosing = C.scope
  C.scope = scope

  scope.localCount = 0
  scope.depth = 0
  scope.func = newFunction(C.script)
  scope.type = type
  scope.begin = begin

  if (type === TYPE_FUNCTION) {
    scope.func.name = sourceSubstring(C, C.previous.length, C.previous.start)
  }

  const local = scopeGetLocal(scope, scope.localCount++)
  local.depth = 0
  local.name = null
}

function byteCodeNewConstant(C, value) {
  const code = current(C)
  const constants = code.constants
  const count = constants.length
  for (let c = 0; c < count; c++) {
    if (matchValues(constants[c], value)) {
      return c
    }
  }
  constants.push(value)
  if (count > UINT8_MAX) {
    compileError(C, C.previous, 'too many constants')
    return 0
  }
  return count
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

function emitPop(C) {
  const code = current(C)
  writeByte(code, OP_POP, C.previous.row)
  C.pop = code.count
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
  if (C.current.type !== type) {
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
    compileError(C, C.previous, `expression expected near '${sourceSubstring(C, C.previous.length, C.previous.start)}'`)
    return
  }
  const assign = precedence <= PRECEDENCE_ASSIGN
  prefix(C, assign)
  while (precedence <= rules[C.current.type].precedence) {
    advance(C)
    const infix = rules[C.previous.type].infix
    if (infix === null) {
      compileError(C, C.previous, 'expected infix')
    }
    infix(C, assign)
  }
  if (assign && checkAssign(C)) {
    advance(C)
    compileError(C, C.current, 'invalid assignment')
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
    compileError(C, C.previous, 'too many local variables in scope')
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
        compileError(C, C.previous, 'too many function arguments')
        break
      }
      count++
    } while (match(C, TOKEN_COMMA))
  }
  if (C.current.type === TOKEN_RIGHT_PAREN) {
    advance(C)
  } else {
    compileError(C, C.previous, "function has no closing ')'")
  }
  return count
}

function compileCall(C) {
  const count = args(C)
  emitShort(C, OP_CALL, count)
}

function compileGroup(C) {
  expression(C)
  if (C.current.type === TOKEN_RIGHT_PAREN) {
    advance(C)
  } else {
    compileError(C, C.previous, "parenthesis group has no closing ')'")
  }
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
  emitConstant(C, newInt(C.current.number))
}

function compileFloat(C) {
  emitConstant(C, newFloat(C.current.number))
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
    default:
      return c
  }
}

function parseStringLiteral(string, start, len) {
  const end = start + len
  let literal = ''
  for (let i = start; i < end; i++) {
    const c = string[i]
    if (c === '\\' && i + 1 < end) {
      literal += escapeSequence(string[i + 1])
      i++
    } else {
      literal += c
    }
  }
  return literal
}

function stringLiteral(C) {
  const previous = C.previous
  let string = parseStringLiteral(C.source, previous.start, previous.length)
  while (check(C, TOKEN_STRING)) {
    const current = C.current
    const and = parseStringLiteral(C.source, current.start, current.length)
    string += and
    advance(C)
  }
  return string
}

function compileString(C) {
  emitConstant(C, newString(stringLiteral(C)))
}

function identConstantString(C) {
  return byteCodeNewConstant(C, newString(stringLiteral(C)))
}

function identConstant(C, token) {
  const string = sourceSubstring(C, token.length, token.start)
  return byteCodeNewConstant(C, newString(string))
}

function beginScope(C) {
  C.scope.depth++
}

function endScope(C) {
  const scope = C.scope
  scope.depth--
  while (scope.localCount > 0 && scope.locals[scope.localCount - 1].depth > scope.depth) {
    emitPop(C)
    scope.localCount--
  }
  C.barrier = scope.func.code.count
}

function compileArray(C) {
  emit(C, OP_NEW_ARRAY)
  if (match(C, TOKEN_RIGHT_SQUARE)) {
    return
  }
  while (!check(C, TOKEN_RIGHT_SQUARE) && !check(C, TOKEN_EOF)) {
    emit(C, OP_DUPLICATE)
    expression(C)
    emit(C, OP_ARRAY_PUSH)
    emitPop(C)
    if (!check(C, TOKEN_RIGHT_SQUARE)) {
      consume(C, TOKEN_COMMA, "expected ',' between array elements")
    }
  }
  consume(C, TOKEN_RIGHT_SQUARE, "expected ']' at end of array declaration")
}

function compileTable(C) {
  emit(C, OP_NEW_TABLE)
  if (match(C, TOKEN_RIGHT_CURLY)) {
    return
  }
  while (!check(C, TOKEN_RIGHT_CURLY) && !check(C, TOKEN_EOF)) {
    emit(C, OP_DUPLICATE)
    let name = UINT8_MAX
    if (match(C, TOKEN_IDENT)) {
      name = identConstant(C, C.previous)
    } else if (match(C, TOKEN_STRING)) {
      name = identConstantString(C)
    } else {
      compileError(C, C.current, 'expected table key')
    }
    consume(C, TOKEN_COLON, "expected ':' between table key and value")
    expression(C)
    emitShort(C, OP_SET_PROPERTY, name)
    emitPop(C)
    if (!check(C, TOKEN_RIGHT_CURLY)) {
      consume(C, TOKEN_COMMA, "expected ',' between table elements")
    }
  }
  consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of table declaration")
}

function pushLocal(C, name) {
  const scope = C.scope
  if (scope.localCount === HYMN_UINT8_COUNT) {
    compileError(C, C.previous, 'too many local variables in scope')
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
  const name = sourceSubstring(C, token.length, token.start)
  for (let i = scope.localCount - 1; i >= 0; i--) {
    const local = scope.locals[i]
    if (local.depth !== -1 && local.depth < scope.depth) {
      break
    } else if (name === local.name) {
      compileError(C, C.previous, `variable '${name}' already exists in scope`)
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

function typeDeclaration(C) {
  if (match(C, TOKEN_COLON)) {
    const type = C.current.type
    switch (type) {
      case TOKEN_NONE:
      case TOKEN_TO_FLOAT:
      case TOKEN_TO_STRING:
      case TOKEN_TO_INTEGER:
        advance(C)
        return
      default:
        compileError(C, C.current, 'unavailable type declaration')
        return
    }
  }
}

function defineNewVariable(C) {
  const v = variable(C, 'expected a variable name')
  typeDeclaration(C)
  consume(C, TOKEN_ASSIGN, "expected '=' after variable")
  expression(C)
  finalizeVariable(C, v)
}

function resolveLocal(C, token) {
  const name = sourceSubstring(C, token.length, token.start)
  const scope = C.scope
  for (let i = scope.localCount - 1; i >= 0; i--) {
    const local = scope.locals[i]
    if (name === local.name) {
      if (local.depth === -1) {
        compileError(C, token, `local variable '${name}' referenced before assignment`)
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
  consume(C, TOKEN_IDENT, "expected table key after '.'")
  const name = identConstant(C, C.previous)
  if (assign && match(C, TOKEN_ASSIGN)) {
    expression(C)
    emitShort(C, OP_SET_PROPERTY, name)
  } else {
    emitShort(C, OP_GET_PROPERTY, name)
  }
}

function compilePointer(C) {
  consume(C, TOKEN_IDENT, "expected table key after '->'")
  const name = identConstant(C, C.previous)
  consume(C, TOKEN_LEFT_PAREN, "expected '(' after function name")
  emitShort(C, OP_SELF, name)
  const count = args(C)
  if (count === UINT8_MAX) {
    compileError(C, C.previous, 'too many function arguments')
    return
  }
  emitShort(C, OP_CALL, count + 1)
}

function compileSquare(C, assign) {
  if (match(C, TOKEN_COLON)) {
    emitConstant(C, newInt(0))
    if (match(C, TOKEN_RIGHT_SQUARE)) {
      emitConstant(C, newNone())
    } else {
      expression(C)
      consume(C, TOKEN_RIGHT_SQUARE, "expected ']' after square bracket expression")
    }
    emit(C, OP_SLICE)
  } else {
    expression(C)
    if (match(C, TOKEN_COLON)) {
      if (match(C, TOKEN_RIGHT_SQUARE)) {
        emitConstant(C, newNone())
      } else {
        expression(C)
        consume(C, TOKEN_RIGHT_SQUARE, "expected ']' after square bracket expression")
      }
      emit(C, OP_SLICE)
    } else {
      consume(C, TOKEN_RIGHT_SQUARE, "expected ']' after square bracket expression")
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
    compileError(C, C.previous, 'jump offset too large')
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

function echoIfNone(C) {
  const code = C.scope.func.code
  const count = code.count
  if (C.barrier === count) return
  if (C.pop === count) code.instructions[count - 1] = OP_ECHO
}

function endFunction(C) {
  const scope = C.scope
  const func = scope.func
  if (scope.type === TYPE_DIRECT) echoIfNone(C)
  emitShort(C, OP_NONE, OP_RETURN)
  if (scope.type === TYPE_FUNCTION) func.source = C.source.substring(scope.begin, C.previous.start + C.previous.length)
  C.scope = scope.enclosing
  return func
}

function compileFunction(C, type, begin) {
  const scope = new Scope()
  scopeInit(C, scope, type, begin)

  beginScope(C)

  consume(C, TOKEN_LEFT_PAREN, "expected '(' after function name")

  if (!check(C, TOKEN_RIGHT_PAREN)) {
    do {
      C.scope.func.arity++
      if (C.scope.func.arity > UINT8_MAX) {
        compileError(C, C.previous, 'too many function parameters')
      }
      const parameter = variable(C, 'expected parameter name')
      finalizeVariable(C, parameter)
      typeDeclaration(C)
    } while (match(C, TOKEN_COMMA))
  }

  consume(C, TOKEN_RIGHT_PAREN, "expected ')' after function parameters")
  typeDeclaration(C)
  consume(C, TOKEN_LEFT_CURLY, "expected '{' after function parameters")

  while (!check(C, TOKEN_RIGHT_CURLY) && !check(C, TOKEN_EOF)) {
    declaration(C)
  }

  endScope(C)
  consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of function body")

  const func = endFunction(C)
  emitConstant(C, newFuncValue(func))
}

function functionExpression(C) {
  compileFunction(C, TYPE_FUNCTION, C.previous.start)
}

function declareFunction(C) {
  const begin = C.previous.start
  const global = variable(C, 'expected function name')
  localInitialize(C)
  compileFunction(C, TYPE_FUNCTION, begin)
  finalizeVariable(C, global)
}

function declaration(C) {
  if (match(C, TOKEN_SET)) {
    defineNewVariable(C)
  } else if (match(C, TOKEN_FUNCTION)) {
    declareFunction(C)
  } else {
    statement(C)
  }
}

function block(C) {
  beginScope(C)
  while (!check(C, TOKEN_RIGHT_CURLY) && !check(C, TOKEN_EOF)) {
    declaration(C)
  }
  endScope(C)
}

function ifStatement(C) {
  expression(C)
  let jump = emitJump(C, OP_JUMP_IF_FALSE)

  freeJumpOrList(C)

  consume(C, TOKEN_LEFT_CURLY, "expected '{' after if statement")
  beginScope(C)
  while (!check(C, TOKEN_RIGHT_CURLY) && !check(C, TOKEN_EOF)) {
    declaration(C)
  }
  endScope(C)

  consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of if statement body")

  if (check(C, TOKEN_ELIF) || check(C, TOKEN_ELSE)) {
    const jumpEnd = new JumpList()
    jumpEnd.jump = emitJump(C, OP_JUMP)
    let tail = jumpEnd

    while (match(C, TOKEN_ELIF)) {
      patchJump(C, jump)
      freeJumpAndList(C)

      expression(C)
      jump = emitJump(C, OP_JUMP_IF_FALSE)

      freeJumpOrList(C)

      consume(C, TOKEN_LEFT_CURLY, "missing '{' in elif statement")
      beginScope(C)
      while (!check(C, TOKEN_RIGHT_CURLY) && !check(C, TOKEN_EOF)) {
        declaration(C)
      }
      endScope(C)
      consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of elif statement body")

      const next = new JumpList()
      next.jump = emitJump(C, OP_JUMP)

      tail.next = next
      tail = next
    }

    patchJump(C, jump)
    freeJumpAndList(C)

    if (match(C, TOKEN_ELSE)) {
      consume(C, TOKEN_LEFT_CURLY, "expected '{' after else statement")
      block(C)
      consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of else statement body")
    }

    patchJump(C, jumpEnd.jump)
    freeJumps(C, jumpEnd.next)
  } else {
    patchJump(C, jump)
    freeJumpAndList(C)
  }
}

function emitLoop(C, start) {
  emit(C, OP_LOOP)
  const offset = current(C).count - start + 2
  if (offset > UINT16_MAX) {
    compileError(C, C.previous, 'loop is too large')
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
    variable(C, 'expected variable name in for loop')
    localInitialize(C)
    consume(C, TOKEN_IN, "expected 'in' after variable name in for loop")
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

  consume(C, TOKEN_LEFT_CURLY, "expected '{' after for loop declaration")
  block(C)

  // LOOP

  patchJumpForList(C)
  emitShort(C, OP_FOR_LOOP, object)
  const offset = current(C).count - start + 2
  if (offset > UINT16_MAX) {
    compileError(C, C.previous, 'loop is too large')
  }
  emitShort(C, (offset >> 8) & UINT8_MAX, offset & UINT8_MAX)

  // END

  C.loop = loop.next

  patchJump(C, jump)
  patchJumpList(C)

  endScope(C)

  consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of for loop")
}

function forStatement(C) {
  beginScope(C)

  // ASSIGN

  const index = C.scope.localCount

  variable(C, 'expected variable name in for loop')

  if (match(C, TOKEN_ASSIGN)) {
    expression(C)
    localInitialize(C)
    consume(C, TOKEN_COMMA, "expected ',' in for loop after variable assignment")
  } else if (match(C, TOKEN_COMMA)) {
    iteratorStatement(C, true)
    return
  } else if (match(C, TOKEN_IN)) {
    iteratorStatement(C, false)
    return
  } else {
    compileError(C, C.previous, 'incomplete for loop declaration')
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
    expressionStatement(C)
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

  consume(C, TOKEN_LEFT_CURLY, "expected '{' after for loop declaration")
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

  consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of for loop")
}

function whileStatement(C) {
  const start = current(C).count

  const loop = new LoopList(start, C.scope.depth + 1, C.loop, false)
  C.loop = loop

  expression(C)

  const jump = emitJump(C, OP_JUMP_IF_FALSE)

  consume(C, TOKEN_LEFT_CURLY, "expected '{' after while loop declaration")
  block(C)
  emitLoop(C, start)

  C.loop = loop.next

  patchJump(C, jump)
  patchJumpList(C)

  consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of while loop")
}

function returnStatement(C) {
  if (C.scope.type !== TYPE_FUNCTION) {
    compileError(C, C.previous, 'return statement outside of function')
  }
  if (check(C, TOKEN_RIGHT_CURLY)) {
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
    emitPop(C)
  }
}

function breakStatement(C) {
  if (C.loop === null) {
    compileError(C, C.previous, 'break statement outside of loop')
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
    compileError(C, C.previous, 'continue statement outside of loop')
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

  const func = C.scope.func
  except.next = func.except
  func.except = except

  consume(C, TOKEN_LEFT_CURLY, "expected '{' after try declaration")
  beginScope(C)
  while (!check(C, TOKEN_RIGHT_CURLY) && !check(C, TOKEN_EOF)) {
    declaration(C)
  }
  endScope(C)

  const jump = emitJump(C, OP_JUMP)

  consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of try statement")
  consume(C, TOKEN_EXCEPT, "expected 'except' at end of try statement")

  except.end = current(C).count

  beginScope(C)
  const message = variable(C, 'expected variable name in exception declaration')
  finalizeVariable(C, message)
  consume(C, TOKEN_LEFT_CURLY, "expected '{' after exception declaration")
  while (!check(C, TOKEN_RIGHT_CURLY) && !check(C, TOKEN_EOF)) {
    declaration(C)
  }
  endScope(C)

  consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of exception statement")

  patchJump(C, jump)
}

function echoStatement(C) {
  expression(C)
  emit(C, OP_ECHO)
}

function printStatement(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to 'print'`)
  expression(C)
  consume(C, TOKEN_COMMA, `not enough arguments in call to 'print' (expected 2)`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to 'print'`)
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
  if (match(C, TOKEN_ECHO)) {
    echoStatement(C)
  } else if (match(C, TOKEN_PRINT)) {
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
  } else if (match(C, TOKEN_LEFT_CURLY)) {
    block(C)
    consume(C, TOKEN_RIGHT_CURLY, "expected '}' at end of block statement")
  } else {
    expressionStatement(C)
  }
}

function arrayPushExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to 'push'`)
  expression(C)
  consume(C, TOKEN_COMMA, `not enough arguments in call to 'push' (expected 2)`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to 'push'`)
  emit(C, OP_ARRAY_PUSH)
}

function arrayInsertExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to 'insert'`)
  expression(C)
  consume(C, TOKEN_COMMA, `not enough arguments in call to 'insert' (expected 3)`)
  expression(C)
  consume(C, TOKEN_COMMA, `not enough arguments in call to 'insert' (expected 3)`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to 'insert'`)
  emit(C, OP_INSERT)
}

function arrayPopExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to 'pop'`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to 'pop'`)
  emit(C, OP_ARRAY_POP)
}

function deleteExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to 'delete'`)
  expression(C)
  consume(C, TOKEN_COMMA, `not enough arguments in call to 'delete' (expected 2)`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to 'delete'`)
  emit(C, OP_DELETE)
}

function lenExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to 'len'`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to 'len'`)
  emit(C, OP_LEN)
}

function castIntegerExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to 'int'`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to 'int'`)
  emit(C, OP_INT)
}

function castFloatExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to 'float'`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to 'float'`)
  emit(C, OP_FLOAT)
}

function castStringExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to 'str'`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to 'str'`)
  emit(C, OP_STRING)
}

function typeExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to 'type'`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to 'type'`)
  emit(C, OP_TYPE)
}

function clearExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to 'clear'`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to 'clear'`)
  emit(C, OP_CLEAR)
}

function copyExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to 'copy'`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to 'copy'`)
  emit(C, OP_COPY)
}

function keysExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to 'keys'`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to 'keys'`)
  emit(C, OP_KEYS)
}

function indexExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to 'index'`)
  expression(C)
  consume(C, TOKEN_COMMA, `not enough arguments in call to 'index' (expected 2)`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to 'index'`)
  emit(C, OP_INDEX)
}

function existsExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to 'exists'`)
  expression(C)
  consume(C, TOKEN_COMMA, `not enough arguments in call to 'exists' (expected 2)`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to 'exists'`)
  emit(C, OP_EXISTS)
}

function sourceExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to '_source'`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to '_source'`)
  emit(C, OP_SOURCE)
}

function opCodesExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to '_opcodes'`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to '_opcodes'`)
  emit(C, OP_CODES)
}

function stackExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to '_stack'`)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to '_stack'`)
  emit(C, OP_STACK)
}

function referenceExpression(C) {
  consume(C, TOKEN_LEFT_PAREN, `expected opening '(' in call to '_reference'`)
  expression(C)
  consume(C, TOKEN_RIGHT_PAREN, `expected closing ')' in call to '_reference'`)
  emit(C, OP_REFERENCE)
}

function expressionStatement(C) {
  expression(C)
  emitPop(C)
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

function compile(H, script, source, type) {
  const scope = new Scope()
  const C = new Compiler(script, source, H)
  scopeInit(C, scope, type, 0)

  advance(C)
  while (!match(C, TOKEN_EOF)) {
    declaration(C)
  }

  const func = endFunction(C)
  return { func: func, error: C.error }
}

function quoteString(string) {
  let quoted = '"'
  const size = string.length
  for (let i = 0; i < size; i++) {
    const c = string[i]
    if (c === '\\') {
      quoted += '\\\\'
    } else if (c === '"') {
      quoted += '\\"'
    } else {
      quoted += c
    }
  }
  return quoted + '"'
}

function valueToStringRecursive(value, set, quote) {
  switch (value.is) {
    case HYMN_VALUE_NONE:
      return 'none'
    case HYMN_VALUE_BOOL:
      return value.value ? 'true' : 'false'
    case HYMN_VALUE_INTEGER:
    case HYMN_VALUE_FLOAT:
    case HYMN_VALUE_POINTER:
      return String(value.value)
    case HYMN_VALUE_STRING:
      if (quote) {
        return quoteString(value.value)
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
        print += quoteString(key) + ': ' + valueToStringRecursive(item, set, true)
      }
      print += ' }'
      return print
    }
    case HYMN_VALUE_FUNC: {
      const func = value.value
      if (func.name) return func.name
      if (func.script) return func.script
      return 'script'
    }
    case HYMN_VALUE_FUNC_NATIVE:
      return value.value.name
  }
}

function valueToString(value) {
  return valueToStringRecursive(value, null, false)
}

function valueToInspect(value) {
  if (value.is === HYMN_VALUE_FUNC) {
    const func = value.value
    if (func.source) return func.source
  }
  return valueToStringRecursive(value, null, false)
}

function valueToDebug(value) {
  if (value.is === HYMN_VALUE_FUNC) {
    const func = value.value
    return disassembleByteCode(func.code, func.name)
  }
  return valueToStringRecursive(value, null, false)
}

function hymnConcat(a, b) {
  return newString(valueToString(a) + valueToString(b))
}

function debugValueToString(value) {
  return valueType(value.is) + ': ' + valueToString(value)
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
    console.error('nothing on stack to peek')
    return newNone()
  }
  return copyValue(H.stack[H.stackTop - dist])
}

function hymnPop(H) {
  if (H.stackTop === 0) {
    console.error('nothing on stack to pop')
    return newNone()
  }
  return copyValue(H.stack[--H.stackTop])
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
    trace += '  at'
    if (func.name !== null) trace += ' ' + func.name
    trace += (func.script === null ? ' script:' : ' ' + func.script + ':') + row
    if (i > 0) trace += '\n'
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
  error += '\n' + hymnStacktrace(H)
  return hymnPushError(H, error)
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
    if (count < func.arity) return hymnThrowError(H, `not enough arguments in call to '${func.name}' (expected ${func.arity})`)
    return hymnThrowError(H, `too many arguments in call to '${func.name}' (expected ${func.arity})`)
  } else if (H.frameCount === HYMN_FRAMES_MAX) {
    return hymnThrowError(H, 'stack overflow')
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
      const top = H.stackTop - count - 1
      H.stackTop = top
      hymnPush(H, result)
      return currentFrame(H)
    }
    default: {
      return hymnThrowError(H, `can't call ${valueType(value.is)} (expected function)`)
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
    const parent = script ? nodePath.dirname(script) : null

    for (let i = 0; i < size; i++) {
      const value = paths[i]
      if (!isString(value)) {
        continue
      }
      const question = value.value

      const replace = question.replace(/<path>/g, file)
      const path = parent ? replace.replace(/<parent>/g, parent) : replace
      const use = nodePath.resolve(path)

      if (tableGet(imports, use) !== null) {
        return currentFrame(H)
      }

      if (nodeFs.existsSync(use)) {
        module = use
        break
      }
    }

    if (module === null) {
      let missing = 'import not found: ' + file

      for (let i = 0; i < size; i++) {
        const value = paths[i]
        if (!isString(value)) {
          continue
        }
        const question = value.value

        const replace = question.replace(/<path>/g, file)
        const path = parent ? replace.replace(/<parent>/g, parent) : replace
        const use = nodePath.resolve(path)

        missing += '\n  no file: ' + use
      }

      return hymnThrowError(H, missing)
    }

    tablePut(imports, module, newBool(true))

    source = nodeFs.readFileSync(module, { encoding: 'utf-8' })
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
      let missing = 'import not found: ' + file

      for (let i = 0; i < size; i++) {
        const value = paths[i]
        if (!isString(value)) {
          continue
        }
        const question = value.value

        const replace = question.replace(/<path>/g, file)
        const use = parent ? replace.replace(/<parent>/g, parent) : replace

        missing += '\n  no file: ' + use
      }

      return hymnThrowError(H, missing)
    }

    tablePut(imports, module, newBool(true))
  }

  const result = compile(H, module, source, TYPE_SCRIPT)

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
  debug[0] += `${name}: [${constant}] [${debugValueToString(code.constants[constant])}]`
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
    case OP_INSERT:
      return debugInstruction(debug, 'OP_INSERT', index)
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
    case OP_SELF:
      return debugConstantInstruction(debug, 'OP_SELF', code, index)
    case OP_CLEAR:
      return debugInstruction(debug, 'OP_CLEAR', index)
    case OP_CONSTANT:
      return debugConstantInstruction(debug, 'OP_CONSTANT', code, index)
    case OP_NEW_ARRAY:
      return debugInstruction(debug, 'OP_NEW_ARRAY', index)
    case OP_NEW_TABLE:
      return debugInstruction(debug, 'OP_NEW_TABLE', index)
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
    case OP_ECHO:
      return debugInstruction(debug, 'OP_ECHO', index)
    case OP_EXISTS:
      return debugInstruction(debug, 'OP_EXISTS', index)
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
    case OP_GREATER:
      return debugInstruction(debug, 'OP_GREATER', index)
    case OP_GREATER_EQUAL:
      return debugInstruction(debug, 'OP_GREATER_EQUAL', index)
    case OP_INCREMENT_LOCAL_AND_SET:
      return debugThreeByteInstruction(debug, 'OP_INCREMENT_LOCAL_AND_SET', code, index)
    case OP_INDEX:
      return debugInstruction(debug, 'OP_INDEX', index)
    case OP_JUMP:
      return debugJumpInstruction(debug, 'OP_JUMP', 1, code, index)
    case OP_JUMP_IF_FALSE:
      return debugJumpInstruction(debug, 'OP_JUMP_IF_FALSE', 1, code, index)
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
    case OP_THROW:
      return debugInstruction(debug, 'OP_THROW', index)
    case OP_FLOAT:
      return debugInstruction(debug, 'OP_FLOAT', index)
    case OP_INT:
      return debugInstruction(debug, 'OP_INT', index)
    case OP_STRING:
      return debugInstruction(debug, 'OP_STRING', index)
    case OP_TRUE:
      return debugInstruction(debug, 'OP_TRUE', index)
    case OP_TYPE:
      return debugInstruction(debug, 'OP_TYPE', index)
    case OP_USE:
      return debugInstruction(debug, 'OP_USE', index)
    case OP_SOURCE:
      return debugInstruction(debug, 'OP_SOURCE', index)
    case OP_CODES:
      return debugInstruction(debug, 'OP_CODES', index)
    case OP_STACK:
      return debugInstruction(debug, 'OP_STACK', index)
    case OP_REFERENCE:
      return debugInstruction(debug, 'OP_REFERENCE', index)
    default:
      return (debug[0] += 'UNKNOWN_OPCODE ' + instruction)
  }
}

function disassembleByteCode(code) {
  const debug = ['']
  if (code.count > 0) {
    let index = disassembleInstruction(debug, code, 0)
    while (index < code.count) {
      debug[0] += '\n'
      index = disassembleInstruction(debug, code, index)
    }
  }
  return debug[0]
}

async function hymnRun(H) {
  let frame = currentFrame(H)
  while (true) {
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
      case OP_SELF: {
        const table = hymnPeek(H, 1)
        if (!isTable(table)) {
          frame = hymnThrowError(H, `can't get property of ${valueType(table.is)} (expected table)`)
          if (frame === null) return
          else break
        }
        const name = readConstant(frame).value
        const fun = tableGet(table.value, name)
        copyValueToFrom(hymnStackGet(H, H.stackTop - 1), fun)
        hymnPush(H, table)
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
            H.stack[frame.stack + slot + 2] = copyValue(next.value)
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
            H.stack[frame.stack + slot + 2] = copyValue(item)
            frame.ip += 2
          }
        } else {
          H.stack[frame.stack + slot + 1] = newNone()
          H.stack[frame.stack + slot + 2] = newNone()
          frame = hymnThrowError(H, `can't iterate over ${valueType(object.is)} (expected array or table)`)
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
            H.stack[frame.stack + value] = copyValue(next.value)
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
            H.stack[frame.stack + value] = copyValue(item)
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
          frame = hymnThrowError(H, `comparison '<' can't use ${valueType(a.is)} and ${valueType(b.is)} (expected numbers)`)
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
          frame = hymnThrowError(H, `comparison '<=' can't use ${valueType(a.is)} and ${valueType(b.is)} (expected numbers)`)
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
          frame = hymnThrowError(H, `comparison '>' can't use ${valueType(a.is)} and ${valueType(b.is)} (expected numbers)`)
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
          frame = hymnThrowError(H, `comparison '>=' can't use ${valueType(a.is)} and ${valueType(b.is)} (expected numbers)`)
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
            frame = hymnThrowError(H, `can't add ${valueType(a.is)} and ${valueType(b.is)}`)
            if (frame === null) return
            else break
          }
        } else if (isBool(a)) {
          if (isString(b)) {
            hymnPush(H, hymnConcat(a, b))
          } else {
            frame = hymnThrowError(H, `can't add ${valueType(a.is)} and ${valueType(b.is)}`)
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
            frame = hymnThrowError(H, `can't add ${valueType(a.is)} and ${valueType(b.is)}`)
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
            frame = hymnThrowError(H, `can't add ${valueType(a.is)} and ${valueType(b.is)}`)
            if (frame === null) return
            else break
          }
        } else if (isString(a)) {
          hymnPush(H, hymnConcat(a, b))
        } else {
          frame = hymnThrowError(H, `can't add ${valueType(a.is)} and ${valueType(b.is)}`)
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
            frame = hymnThrowError(H, `can't subtract ${valueType(a.is)} and ${valueType(b.is)} (expected numbers)`)
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
            frame = hymnThrowError(H, `can't subtract ${valueType(a.is)} and ${valueType(b.is)} (expected numbers)`)
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, `can't subtract ${valueType(a.is)} and ${valueType(b.is)} (expected numbers)`)
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
            frame = hymnThrowError(H, `can't multiply ${valueType(a.is)} and ${valueType(b.is)} (expected numbers)`)
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
            frame = hymnThrowError(H, `can't multiply ${valueType(a.is)} and ${valueType(b.is)} (expected numbers)`)
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, `can't multiply ${valueType(a.is)} and ${valueType(b.is)} (expected numbers)`)
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
            frame = hymnThrowError(H, `can't divide ${valueType(a.is)} and ${valueType(b.is)} (expected numbers)`)
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
            frame = hymnThrowError(H, `can't divide ${valueType(a.is)} and ${valueType(b.is)} (expected numbers)`)
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, `can't divide ${valueType(a.is)} and ${valueType(b.is)} (expected numbers)`)
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
            frame = hymnThrowError(H, `can't modulo ${valueType(a.is)} and ${valueType(b.is)} (expected integers)`)
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, `can't modulo ${valueType(a.is)} and ${valueType(b.is)} (expected integers)`)
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
          frame = hymnThrowError(H, `bitwise '~' can't use ${valueType(value.is)} (expected integer)`)
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
            frame = hymnThrowError(H, `bitwise '|' can't use ${valueType(a.is)} and ${valueType(b.is)} (expected integers)`)
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, `bitwise '|' can't use ${valueType(a.is)} and ${valueType(b.is)} (expected integers)`)
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
            frame = hymnThrowError(H, `bitwise '&' can't use ${valueType(a.is)} and ${valueType(b.is)} (expected integers)`)
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, `bitwise '&' can't use ${valueType(a.is)} and ${valueType(b.is)} (expected integers)`)
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
            frame = hymnThrowError(H, `bitwise '^' can't use ${valueType(a.is)} and ${valueType(b.is)} (expected integers)`)
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, `bitwise '^' can't use ${valueType(a.is)} and ${valueType(b.is)} (expected integers)`)
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
            frame = hymnThrowError(H, `bitwise '<<' can't use ${valueType(a.is)} and ${valueType(b.is)} (expected integers)`)
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, `bitwise '<<' can't use ${valueType(a.is)} and ${valueType(b.is)} (expected integers)`)
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
            frame = hymnThrowError(H, `bitwise '>>' can't use ${valueType(a.is)} and ${valueType(b.is)} (expected integers)`)
            if (frame === null) return
            else break
          }
        } else {
          frame = hymnThrowError(H, `bitwise '>>' can't use ${valueType(a.is)} and ${valueType(b.is)} (expected integers)`)
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
          frame = hymnThrowError(H, `negation '-' can't use ${valueType(value.is)} (expected number)`)
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
          frame = hymnThrowError(H, `not '!' can't use ${valueType(value.is)} (expected boolean)`)
          if (frame === null) return
          else break
        }
        hymnPush(H, value)
        break
      }
      case OP_CONSTANT: {
        const value = readConstant(frame)
        hymnPush(H, value)
        break
      }
      case OP_NEW_ARRAY: {
        const value = newArrayValue([])
        hymnPush(H, value)
        break
      }
      case OP_NEW_TABLE: {
        const value = newTableValue(new HymnTable())
        hymnPush(H, value)
        break
      }
      case OP_DEFINE_GLOBAL: {
        const name = readConstant(frame).value
        const value = hymnPop(H)
        const previous = tablePut(H.globals, name, value)
        if (previous !== null) {
          tablePut(H.globals, name, previous)
          frame = hymnThrowError(H, `multiple global definitions of '${name}'`)
          if (frame === null) return
          else break
        }
        break
      }
      case OP_SET_GLOBAL: {
        const name = readConstant(frame).value
        const value = hymnPeek(H, 1)
        tablePut(H.globals, name, value)
        break
      }
      case OP_GET_GLOBAL: {
        const name = readConstant(frame).value
        const get = tableGet(H.globals, name)
        if (get === null) {
          frame = hymnThrowError(H, `undefined global '${name}'`)
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
      case OP_INCREMENT_LOCAL_AND_SET: {
        const slot = readByte(frame)
        const increment = readByte(frame)
        const value = copyValue(H.stack[frame.stack + slot])
        if (isInt(value) || isFloat(value)) {
          value.value += increment
        } else {
          frame = hymnThrowError(H, `can't increment ${valueType(value.is)} (expected number)`)
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
          frame = hymnThrowError(H, `can't set property of ${valueType(value.is)} (expected table)`)
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
        const value = hymnPop(H)
        if (!isTable(value)) {
          frame = hymnThrowError(H, `can't get property of ${valueType(value.is)} (expected table)`)
          if (frame === null) return
          else break
        }
        const table = value.value
        const name = readConstant(frame).value
        const g = tableGet(table, name)
        if (g === null) hymnPush(H, newNone())
        else hymnPush(H, g)
        break
      }
      case OP_EXISTS: {
        const v = hymnPop(H)
        const o = hymnPop(H)
        if (!isTable(o)) {
          frame = hymnThrowError(H, `call to 'exists' can't use ${valueType(o.is)} for 1st argument (expected table)`)
          if (frame === null) return
          else break
        }
        if (!isString(v)) {
          frame = hymnThrowError(H, `call to 'exists' can't use ${valueType(v.is)} for 2nd argument (expected string)`)
          if (frame === null) return
          else break
        }
        const table = o.value
        const name = v.value
        const g = tableGet(table, name)
        if (g === null) {
          hymnPush(H, newBool(false))
        } else {
          hymnPush(H, newBool(true))
        }
        break
      }
      case OP_SET_DYNAMIC: {
        const s = hymnPop(H)
        const i = hymnPop(H)
        const v = hymnPop(H)
        if (isArray(v)) {
          if (!isInt(i)) {
            frame = hymnThrowError(H, `array assignment index can't be ${valueType(i.is)} (expected integer)`)
            if (frame === null) return
            else break
          }
          const array = v.value
          const size = array.length
          let index = i.value
          if (index > size) {
            frame = hymnThrowError(H, `array assignment index out of bounds: ${index} > ${size}`)
            if (frame === null) return
            else break
          }
          if (index < 0) {
            index = size + index
            if (index < 0) {
              frame = hymnThrowError(H, `negative array assignment index: ${index}`)
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
            frame = hymnThrowError(H, `table assignment key can't be ${valueType(i.is)} (expected string)`)
            if (frame === null) return
            else break
          }
          const table = v.value
          const name = i.value
          tablePut(table, name, s)
        } else {
          frame = hymnThrowError(H, `can't assign value to ${valueType(v.is)} (expected array or table)`)
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
              frame = hymnThrowError(H, `string index can't be ${valueType(i.is)} (expected integer)`)
              if (frame === null) return
              else break
            }
            const string = v.value
            const size = string.length
            let index = i.value
            if (index >= size) {
              frame = hymnThrowError(H, `string index out of bounds: ${index} >= ${size}`)
              if (frame === null) return
              else break
            }
            if (index < 0) {
              index = size + index
              if (index < 0) {
                frame = hymnThrowError(H, `negative string index: ${index}`)
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
              frame = hymnThrowError(H, `array index can't be ${valueType(i.is)} (expected integer)`)
              if (frame === null) return
              else break
            }
            const array = v.value
            const size = array.length
            let index = i.value
            if (index >= size) {
              frame = hymnThrowError(H, `array index out of bounds: ${index} >= ${size}`)
              if (frame === null) return
              else break
            }
            if (index < 0) {
              index = size + index
              if (index < 0) {
                frame = hymnThrowError(H, `negative array index: ${index}`)
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
              frame = hymnThrowError(H, `table key can't be ${valueType(i.is)} (expected string)`)
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
            frame = hymnThrowError(H, `can't get value from ${valueType(v.is)} (expected array, table, or string)`)
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
            frame = hymnThrowError(H, `call to 'len' can't use ${valueType(value.is)} (expected array, string, or table)`)
            if (frame === null) return
            else break
        }

        break
      }
      case OP_ARRAY_POP: {
        const a = hymnPop(H)
        if (!isArray(a)) {
          frame = hymnThrowError(H, `call to 'pop' can't use ${valueType(a.is)} (expected array)`)
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
          frame = hymnThrowError(H, `call to 'push' can't use ${valueType(v.is)} for 1st argument (expected array)`)
          if (frame === null) return
          else break
        } else {
          a.value.push(v)
          hymnPush(H, v)
        }
        break
      }
      case OP_INSERT: {
        const p = hymnPop(H)
        const i = hymnPop(H)
        const v = hymnPop(H)
        if (isArray(v)) {
          if (!isInt(i)) {
            frame = hymnThrowError(H, `call to 'insert' can't use ${valueType(v.is)} for 2nd argument (expected integer)`)
            if (frame === null) return
            else break
          }
          const array = v.value
          const size = array.length
          let index = i.value
          if (index > size) {
            frame = hymnThrowError(H, `index out of bounds in call to 'insert': ${index} > ${size}`)
            if (frame === null) return
            else break
          }
          if (index < 0) {
            index = size + index
            if (index < 0) {
              frame = hymnThrowError(H, `negative index in 'insert' call: ${index}`)
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
          frame = hymnThrowError(H, `call to 'insert' can't use ${valueType(v.is)} for 1st argument (expected array)`)
          if (frame === null) return
          else break
        }
        break
      }
      case OP_DELETE: {
        const at = hymnPop(H)
        const value = hymnPop(H)
        if (isArray(value)) {
          if (!isInt(at)) {
            frame = hymnThrowError(H, `call to 'delete' can't use ${valueType(at.is)} for 2nd argument (expected integer)`)
            if (frame === null) return
            else break
          }
          const array = value.value
          const size = array.length
          let index = at.value
          if (index >= size) {
            frame = hymnThrowError(H, `index out of bounds in call to 'delete': ${index} >= ${size}`)
            if (frame === null) return
            else break
          }
          if (index < 0) {
            index = size + index
            if (index < 0) {
              frame = hymnThrowError(H, `negative index in 'delete' call: ${index}`)
              if (frame === null) return
              else break
            }
          }
          const item = array.splice(index, 1)[0]
          hymnPush(H, item)
        } else if (isTable(value)) {
          if (!isString(at)) {
            frame = hymnThrowError(H, `call to 'delete' can't use ${valueType(at.is)} for 2nd argument (expected string)`)
            if (frame === null) return
            else break
          }
          const table = value.value
          const name = at.value
          const item = tableGet(table, name)
          if (item !== null) {
            tableRemove(table, name)
            hymnPush(H, item)
          } else {
            hymnPush(H, newNone())
          }
        } else {
          frame = hymnThrowError(H, `call to 'delete' can't use ${valueType(value.is)} for 1st argument (expected array or table)`)
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
          frame = hymnThrowError(H, `slice can't use ${valueType(a.is)} (expected integer)`)
          if (frame === null) return
          else break
        }
        const start = a.value
        if (start < 0) {
          frame = hymnThrowError(H, `negative slice start: ${start}`)
          if (frame === null) return
          else break
        }
        if (isString(v)) {
          const original = v.value
          const size = original.length
          let end
          if (isInt(b)) {
            end = b.value
          } else if (isNone(b)) {
            end = size
          } else {
            frame = hymnThrowError(H, `slice can't use ${valueType(b.is)} (expected integer)`)
            if (frame === null) return
            else break
          }
          if (end > size) {
            frame = hymnThrowError(H, `slice out of bounds: ${end} > ${size}`)
            if (frame === null) return
            else break
          }
          if (end < 0) {
            end = size + end
            if (end < 0) {
              frame = hymnThrowError(H, `negative slice end: ${end}`)
              if (frame === null) return
              else break
            }
          }
          if (start >= end) {
            frame = hymnThrowError(H, `slice out of range: ${start} >= ${end}`)
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
            frame = hymnThrowError(H, `slice can't use ${valueType(b.is)} (expected integer)`)
            if (frame === null) return
            else break
          }
          if (end > size) {
            frame = hymnThrowError(H, `slice out of bounds: ${end} > ${size}`)
            if (frame === null) return
            else break
          }
          if (end < 0) {
            end = size + end
            if (end < 0) {
              frame = hymnThrowError(H, `negative slice end: ${end}`)
              if (frame === null) return
              else break
            }
          }
          if (start >= end) {
            frame = hymnThrowError(H, `slice out of range: ${start} >= ${end}`)
            if (frame === null) return
            else break
          }
          const copy = array.slice(start, end)
          hymnPush(H, newArrayValue(copy))
        } else {
          frame = hymnThrowError(H, `can't slice ${valueType(v.is)} (expected string or array)`)
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
          frame = hymnThrowError(H, `call to 'keys' can't use ${valueType(value.is)} (expected table)`)
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
              frame = hymnThrowError(H, `call to 'index' can't use ${valueType(b.is)} for 2nd argument (expected string)`)
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
            frame = hymnThrowError(H, `call to 'index' can't use ${valueType(a.is)} for 1st argument (expected string, array, or table)`)
            if (frame === null) return
            else break
        }
        break
      }
      case OP_TYPE: {
        const value = hymnPop(H)
        hymnPush(H, newString(valueType(value.is)))
        break
      }
      case OP_INT: {
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
          frame = hymnThrowError(H, `can't cast ${valueType(value.is)} to integer`)
          if (frame === null) return
          else break
        }
        break
      }
      case OP_FLOAT: {
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
          frame = hymnThrowError(H, `can't cast ${valueType(value.is)} to float`)
          if (frame === null) return
          else break
        }
        break
      }
      case OP_STRING: {
        const value = hymnPop(H)
        hymnPush(H, newString(valueToString(value)))
        break
      }
      case OP_ECHO: {
        const value = hymnPop(H)
        H.printLine(valueToString(value))
        break
      }
      case OP_PRINT: {
        const value = hymnPop(H)
        const route = hymnPop(H)
        if (hymnFalse(route)) {
          H.print(valueToString(value))
        } else {
          H.printError(valueToString(value))
        }
        break
      }
      case OP_SOURCE: {
        const value = hymnPop(H)
        hymnPush(H, newString(valueToInspect(value)))
        break
      }
      case OP_CODES: {
        const value = hymnPop(H)
        hymnPush(H, newString(valueToDebug(value)))
        break
      }
      case OP_STACK: {
        if (H.stackTop !== 0) {
          let debug = ''
          for (let i = 0; i < H.stackTop; i++) {
            debug += '[' + debugValueToString(H.stack[i]) + ']\n'
          }
          hymnPush(H, newString(debug))
        } else {
          hymnPush(H, newString(''))
        }
        break
      }
      case OP_REFERENCE: {
        hymnPop(H)
        hymnPush(H, newInt(0))
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
          frame = hymnThrowError(H, `import can't use ${valueType(file.is)} (expected string)`)
          if (frame === null) return
          else break
        }
        break
      }
      default:
        console.error('unknown instruction')
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
  const result = compile(H, script, source, TYPE_SCRIPT)

  const func = result.func
  if (result.error !== null) {
    return result.error
  }

  console.debug(`\n-- ${script !== null ? script : 'script'} --\n${disassembleByteCode(func.code)}`)

  const constants = func.code.constants

  for (let i = 0; i < constants.length; i++) {
    const constant = constants[i]
    if (isFunc(constant)) {
      const value = constant.value
      console.debug(`\n-- ${value.name !== null ? value.name : 'script'} --\n${disassembleByteCode(value.code)}`)
    }
  }

  hymnResetStack(H)

  return null
}

async function interpretScript(H, script, source, type) {
  const result = compile(H, script, source, type)

  const func = result.func
  if (result.error !== null) {
    return result.error
  }

  const funcVal = newFuncValue(func)
  hymnPush(H, funcVal)
  hymnCall(H, func, 0)

  await hymnRun(H)
  if (H.error !== null) return H.error

  hymnResetStack(H)
  return null
}

async function interpret(H, source) {
  return interpretScript(H, null, source, TYPE_SCRIPT)
}

async function direct(H, source) {
  return interpretScript(H, null, source, TYPE_DIRECT)
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

  tablePut(H.globals, 'PATHS', newArrayValue(H.paths))
  tablePut(H.globals, 'IMPORTS', newTableValue(H.imports))
  tablePut(H.globals, 'GLOBALS', newTableValue(H.globals))

  return H
}

if (node) {
  module.exports = {
    version: HYMN_VERSION,
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
    interpretScript: interpretScript,
    interpret: interpret,
    direct: direct,
    newVM: newVM,
    debug: debugScript,
  }
}
