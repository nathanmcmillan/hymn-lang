const HYMN_DEBUG_TOKEN = true
const HYMN_DEBUG_TRACE = true
const HYMN_DEBUG_STACK = true

const UINT8_MAX = 255
const UINT16_MAX = 65535
const HYMN_UINT8_COUNT = UINT8_MAX + 1
const HYMN_FRAMES_MAX = 64
const HYMN_STACK_MAX = HYMN_FRAMES_MAX * HYMN_UINT8_COUNT

const HYMN_VALUE_UNDEFINED = 0
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

class HymnValue {
  constructor(is, value) {
    this.is = is
    this.value = value
  }
}

function copyValue(dest, src) {
  dest.is = src.is
  dest.value = src.value
}

class HymnNativeFunction {
  constructor(name, call) {
    this.name = name
    this.call = call
  }
}

class HymnByteCode {
  constructor() {
    this.count = 0
    this.instructions = new Uint8Array(128)
    this.rows = new Uint32Array(128)
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

class Hymn {
  constructor() {
    this.stack = []
    this.stackTop = 0
    this.frames = []
    this.frameCount = 0
    this.globals = null
    this.paths = null
    this.imports = null
    this.error = null
  }
}

const STRING_UNDEFINED = 'Undefined'
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
const TOKEN_BEGIN = 3
const TOKEN_BIT_AND = 4
const TOKEN_BIT_LEFT_SHIFT = 5
const TOKEN_BIT_NOT = 6
const TOKEN_BIT_OR = 7
const TOKEN_BIT_RIGHT_SHIFT = 8
const TOKEN_BIT_XOR = 9
const TOKEN_BREAK = 10
const TOKEN_CASE = 11
const TOKEN_CLEAR = 12
const TOKEN_COLON = 13
const TOKEN_COMMA = 14
const TOKEN_CONST = 15
const TOKEN_CONTINUE = 16
const TOKEN_COPY = 17
const TOKEN_DELETE = 18
const TOKEN_DIVIDE = 19
const TOKEN_DO = 20
const TOKEN_DOT = 21
const TOKEN_ELIF = 22
const TOKEN_ELSE = 23
const TOKEN_END = 24
const TOKEN_EOF = 25
const TOKEN_EQUAL = 26
const TOKEN_ERROR = 27
const TOKEN_EXCEPT = 28
const TOKEN_FALSE = 29
const TOKEN_FLOAT = 30
const TOKEN_FOR = 31
const TOKEN_FUNCTION = 32
const TOKEN_GREATER = 33
const TOKEN_GREATER_EQUAL = 34
const TOKEN_IDENT = 35
const TOKEN_IF = 36
const TOKEN_IN = 37
const TOKEN_INDEX = 38
const TOKEN_INSERT = 39
const TOKEN_INTEGER = 40
const TOKEN_ITERATE = 41
const TOKEN_KEYS = 42
const TOKEN_LEFT_CURLY = 43
const TOKEN_LEFT_PAREN = 44
const TOKEN_LEFT_SQUARE = 45
const TOKEN_LEN = 46
const TOKEN_LESS = 47
const TOKEN_LESS_EQUAL = 48
const TOKEN_LET = 49
const TOKEN_LINE = 50
const TOKEN_MODULO = 51
const TOKEN_MULTIPLY = 52
const TOKEN_NONE = 53
const TOKEN_NOT = 54
const TOKEN_NOT_EQUAL = 55
const TOKEN_OR = 56
const TOKEN_PASS = 57
const TOKEN_POP = 58
const TOKEN_PRINT = 59
const TOKEN_PUSH = 60
const TOKEN_RETURN = 61
const TOKEN_RIGHT_CURLY = 62
const TOKEN_RIGHT_PAREN = 63
const TOKEN_RIGHT_SQUARE = 64
const TOKEN_SEMICOLON = 65
const TOKEN_STRING = 66
const TOKEN_SUBTRACT = 67
const TOKEN_SWITCH = 68
const TOKEN_THROW = 69
const TOKEN_TO_FLOAT = 70
const TOKEN_TO_INTEGER = 71
const TOKEN_TO_STRING = 72
const TOKEN_TRUE = 73
const TOKEN_TRY = 74
const TOKEN_TYPE = 75
const TOKEN_UNDEFINED = 76
const TOKEN_USE = 77
const TOKEN_VALUE = 78
const TOKEN_WHILE = 79

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
const PRECEDENCE_PRIMARY = 11

const OP_ADD = 0
const OP_ARRAY_INSERT = 1
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
const OP_CONSTANT_TWO = 13
const OP_COPY = 14
const OP_DEFINE_GLOBAL = 15
const OP_DELETE = 16
const OP_DIVIDE = 17
const OP_DO = 18
const OP_DUPLICATE = 19
const OP_EQUAL = 20
const OP_FALSE = 21
const OP_GET_DYNAMIC = 22
const OP_GET_GLOBAL = 23
const OP_GET_LOCAL = 24
const OP_GET_PROPERTY = 25
const OP_GREATER = 26
const OP_GREATER_EQUAL = 27
const OP_INDEX = 28
const OP_JUMP = 29
const OP_JUMP_IF_FALSE = 30
const OP_JUMP_IF_TRUE = 31
const OP_KEYS = 32
const OP_LEN = 33
const OP_LESS = 34
const OP_LESS_EQUAL = 35
const OP_LOOP = 36
const OP_MODULO = 37
const OP_MULTIPLY = 38
const OP_NEGATE = 39
const OP_NONE = 40
const OP_NOT = 41
const OP_NOT_EQUAL = 42
const OP_POP = 43
const OP_PRINT = 44
const OP_RETURN = 45
const OP_SET_DYNAMIC = 46
const OP_SET_GLOBAL = 47
const OP_SET_LOCAL = 48
const OP_SET_PROPERTY = 49
const OP_SLICE = 50
const OP_SUBTRACT = 51
const OP_THROW = 52
const OP_TO_FLOAT = 53
const OP_TO_INTEGER = 54
const OP_TO_STRING = 55
const OP_TRUE = 56
const OP_TYPE = 57
const OP_USE = 58

const TYPE_FUNCTION = 0
const TYPE_SCRIPT = 1

const MACHINE_ERROR = 0
const MACHINE_FATAL = 1
const MACHINE_OK = 2

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
    this.nameStart = 0
    this.nameLen = 0
    this.depth = 0
    this.constant = false
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
  constructor(script, source, hymn) {
    this.pos = 0
    this.row = 1
    this.column = 1
    this.script = script
    this.source = source
    this.previous = new Token()
    this.current = new Token()
    this.hymn = hymn
    this.scope = null
    this.loop = null
    this.jump = null
    this.iteratorJump = null
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
  constructor() {
    this.start = 0
    this.depth = 0
    this.next = null
  }
}

const rules = []
rules[TOKEN_ADD] = new Rule(null, compileBinary, PRECEDENCE_TERM)
rules[TOKEN_AND] = new Rule(null, compileAnd, PRECEDENCE_AND)
rules[TOKEN_ASSIGN] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_BEGIN] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_BIT_AND] = new Rule(null, compileBinary, PRECEDENCE_BITS)
rules[TOKEN_BIT_LEFT_SHIFT] = new Rule(null, compileBinary, PRECEDENCE_BITS)
rules[TOKEN_BIT_NOT] = new Rule(compileUnary, null, PRECEDENCE_NONE)
rules[TOKEN_BIT_OR] = new Rule(null, compileBinary, PRECEDENCE_BITS)
rules[TOKEN_BIT_RIGHT_SHIFT] = new Rule(null, compileBinary, PRECEDENCE_BITS)
rules[TOKEN_BIT_XOR] = new Rule(null, compileBinary, PRECEDENCE_BITS)
rules[TOKEN_BREAK] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_CASE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_CLEAR] = new Rule(clearExpression, null, PRECEDENCE_NONE)
rules[TOKEN_COMMA] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_CONST] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_CONTINUE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_COPY] = new Rule(copyExpression, null, PRECEDENCE_NONE)
rules[TOKEN_DO] = new Rule(null, null, PRECEDENCE_NONE)
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
rules[TOKEN_ITERATE] = new Rule(null, null, PRECEDENCE_NONE)
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
rules[TOKEN_PASS] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_POP] = new Rule(arrayPopExpression, null, PRECEDENCE_NONE)
rules[TOKEN_PRINT] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_PUSH] = new Rule(arrayPushExpression, null, PRECEDENCE_NONE)
rules[TOKEN_RETURN] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_RIGHT_CURLY] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_RIGHT_PAREN] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_RIGHT_SQUARE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_STRING] = new Rule(compileString, null, PRECEDENCE_NONE)
rules[TOKEN_SUBTRACT] = new Rule(compileUnary, compileBinary, PRECEDENCE_TERM)
rules[TOKEN_SWITCH] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_TO_FLOAT] = new Rule(castFloatExpression, null, PRECEDENCE_NONE)
rules[TOKEN_TO_INTEGER] = new Rule(castIntegerExpression, null, PRECEDENCE_NONE)
rules[TOKEN_TO_STRING] = new Rule(castStringExpression, null, PRECEDENCE_NONE)
rules[TOKEN_TRUE] = new Rule(compileTrue, null, PRECEDENCE_NONE)
rules[TOKEN_TRY] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_THROW] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_TYPE] = new Rule(typeExpression, null, PRECEDENCE_NONE)
rules[TOKEN_UNDEFINED] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_USE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_VALUE] = new Rule(null, null, PRECEDENCE_NONE)
rules[TOKEN_WHILE] = new Rule(null, null, PRECEDENCE_NONE)

function valueName(type) {
  switch (type) {
    case HYMN_VALUE_UNDEFINED:
      return STRING_UNDEFINED
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

function tokenName(token) {
  switch (token.type) {
    case TOKEN_ADD:
      return 'ADD'
    case TOKEN_AND:
      return 'AND'
    case TOKEN_ASSIGN:
      return 'ASSIGN'
    case TOKEN_BEGIN:
      return 'BEGIN'
    case TOKEN_BIT_AND:
      return 'BTIWISE_AND'
    case TOKEN_BIT_LEFT_SHIFT:
      return 'BTIWISE_LEFT_SHIFT'
    case TOKEN_BIT_NOT:
      return 'BTIWISE_NOT'
    case TOKEN_BIT_OR:
      return 'BTIWISE_OR'
    case TOKEN_BIT_RIGHT_SHIFT:
      return 'BTIWISE_RIGHT_SHIFT'
    case TOKEN_BIT_XOR:
      return 'BTIWISE_XOR'
    case TOKEN_BREAK:
      return 'BREAK'
    case TOKEN_CASE:
      return 'CASE'
    case TOKEN_CLEAR:
      return 'CLEAR'
    case TOKEN_COLON:
      return 'COLON'
    case TOKEN_CONST:
      return 'CONST'
    case TOKEN_CONTINUE:
      return 'CONTINUE'
    case TOKEN_COPY:
      return 'COPY'
    case TOKEN_DO:
      return 'DO'
    case TOKEN_DELETE:
      return 'DELETE'
    case TOKEN_DIVIDE:
      return 'DIVIDE'
    case TOKEN_ELIF:
      return 'ELIF'
    case TOKEN_ELSE:
      return 'ELSE'
    case TOKEN_END:
      return 'END'
    case TOKEN_EOF:
      return 'EOF'
    case TOKEN_EQUAL:
      return 'EQUAL'
    case TOKEN_EXCEPT:
      return 'EXCEPT'
    case TOKEN_FALSE:
      return 'FALSE'
    case TOKEN_FLOAT:
      return 'FLOAT'
    case TOKEN_FOR:
      return 'FOR'
    case TOKEN_FUNCTION:
      return 'FUNCTION'
    case TOKEN_GREATER:
      return 'GREATER'
    case TOKEN_GREATER_EQUAL:
      return 'GREATER_EQUAL'
    case TOKEN_IDENT:
      return 'IDENTITY'
    case TOKEN_IF:
      return 'IF'
    case TOKEN_IN:
      return 'IN'
    case TOKEN_INDEX:
      return 'INDEX'
    case TOKEN_INSERT:
      return 'INSERT'
    case TOKEN_INTEGER:
      return 'INTEGER'
    case TOKEN_ITERATE:
      return 'ITERATE'
    case TOKEN_KEYS:
      return 'KEYS'
    case TOKEN_LEFT_PAREN:
      return 'LEFT_PAREN'
    case TOKEN_LEN:
      return 'LEN'
    case TOKEN_LESS:
      return 'LESS'
    case TOKEN_LESS_EQUAL:
      return 'LESS_EQUAL'
    case TOKEN_LET:
      return 'LET'
    case TOKEN_MODULO:
      return 'MODULO'
    case TOKEN_MULTIPLY:
      return 'MULTIPLY'
    case TOKEN_NONE:
      return 'NONE'
    case TOKEN_NOT:
      return 'NOT'
    case TOKEN_NOT_EQUAL:
      return 'NOT_EQUAL'
    case TOKEN_OR:
      return 'OR'
    case TOKEN_PASS:
      return 'PASS'
    case TOKEN_POP:
      return 'POP'
    case TOKEN_PRINT:
      return 'PRINT'
    case TOKEN_PUSH:
      return 'PUSH'
    case TOKEN_RETURN:
      return 'RETURN'
    case TOKEN_RIGHT_PAREN:
      return 'RIGHT_PAREN'
    case TOKEN_SEMICOLON:
      return 'SEMICOLON'
    case TOKEN_STRING:
      return 'STRING'
    case TOKEN_SUBTRACT:
      return 'SUBTRACT'
    case TOKEN_SWITCH:
      return 'SWITCH'
    case TOKEN_TO_FLOAT:
      return 'FLOAT'
    case TOKEN_TO_INTEGER:
      return 'INT'
    case TOKEN_TO_STRING:
      return 'STRING'
    case TOKEN_TRUE:
      return 'TRUE'
    case TOKEN_TRY:
      return 'TRY'
    case TOKEN_THROW:
      return 'THROW'
    case TOKEN_TYPE:
      return 'TYPE'
    case TOKEN_WHILE:
      return 'WHILE'
    case TOKEN_USE:
      return 'USE'
    case TOKEN_LEFT_CURLY:
      return 'LEFT_CURLY'
    case TOKEN_RIGHT_CURLY:
      return 'RIGHT_CURLY'
    case TOKEN_LEFT_SQUARE:
      return 'LEFT_SQUARE'
    case TOKEN_RIGHT_SQUARE:
      return 'RIGHT_SQUARE'
    default:
      return '?'
  }
}

function debugValueToString(value) {
  let string = valueName(value.is) + ': '
  switch (value.is) {
    case HYMN_VALUE_UNDEFINED:
      return string + STRING_UNDEFINED
    case HYMN_VALUE_NONE:
      return string + STRING_NONE
    case HYMN_VALUE_BOOL:
    case HYMN_VALUE_INTEGER:
    case HYMN_VALUE_FLOAT:
    case HYMN_VALUE_STRING:
    case HYMN_VALUE_ARRAY:
      return string + value.value
    case HYMN_VALUE_FUNC:
    case HYMN_VALUE_FUNC_NATIVE:
      return string + value.value.name
    default:
      return string + '?'
  }
}

function sourceSubstring(compiler, len, start) {
  return compiler.source.substring(start, start + len)
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

function newArray() {
  return []
}

function newTable() {
  return new Map()
}

function isUndefined(value) {
  return value === HYMN_VALUE_UNDEFINED
}

function isNone(value) {
  return value === HYMN_VALUE_NONE
}

function isBool(value) {
  return value === HYMN_VALUE_BOOL
}

function isInt(value) {
  return value === HYMN_VALUE_INTEGER
}

function isFloat(value) {
  return value === HYMN_VALUE_FLOAT
}

function isString(value) {
  return value === HYMN_VALUE_STRING
}

function isArray(value) {
  return value === HYMN_VALUE_ARRAY
}

function isTable(value) {
  return value === HYMN_VALUE_TABLE
}

function isFunc(value) {
  return value === HYMN_VALUE_FUNC
}

function isFuncNative(value) {
  return value === HYMN_VALUE_FUNC_NATIVE
}

function isPointer(value) {
  return value === HYMN_VALUE_POINTER
}

function currentFunc(compiler) {
  return compiler.scope.func
}

function current(compiler) {
  return compiler.scope.func.code
}

function compileError(compiler, token, format) {
  if (compiler.error !== null) return

  let error = format

  compiler.error = error

  compiler.previous.type = TOKEN_EOF
  compiler.current.type = TOKEN_EOF
}

function nextChar(compiler) {
  let pos = compiler.pos
  if (pos === compiler.source.length) return '\0'
  let c = compiler.source[pos]
  compiler.pos = pos + 1
  if (c === '\n') {
    compiler.row++
    compiler.column = 0
  } else {
    compiler.column++
  }
  return c
}

function peekChar(compiler) {
  if (compiler.pos === compiler.source.length) return '\0'
  return compiler.source[compiler.pos]
}

function debugToken(compiler, token) {
  return 'TOKEN: ' + tokenName(token) + ': ' + sourceSubstring(compiler, token.len, token.start)
}

function token(compiler, type) {
  let token = compiler.current
  token.type = type
  token.row = compiler.row
  token.column = compiler.column
  token.start = compiler.pos - 1
  token.len = 1
  if (HYMN_DEBUG_TOKEN) console.debug(debugToken(compiler, token))
}

function tokenSpecial(compiler, type, offset, len) {
  let token = compiler.current
  token.type = type
  token.row = compiler.row
  token.column = compiler.column
  token.start = compiler.pos - offset
  token.len = len
  if (HYMN_DEBUG_TOKEN) console.debug(debugToken(compiler, token))
}

function valueToken(compiler, type, start, end) {
  let token = compiler.current
  token.type = type
  token.row = compiler.row
  token.column = compiler.column
  token.start = start
  token.len = end - start
  if (HYMN_DEBUG_TOKEN) console.debug(debugToken(compiler, token))
}

function identTrie(ident, offset, rest, type) {
  let i = 0
  do {
    if (ident[offset + 1] !== rest[i]) {
      return TOKEN_UNDEFINED
    }
  } while (rest[i] !== '\0')
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
      if (size === 2 && ident[1] === 'o') return TOKEN_DO
      break
    case 'r':
      if (size === 6) return identTrie(ident, 1, 'eturn', TOKEN_RETURN)
      break
    case 's':
      if (size === 6) {
        if (ident[1] === 'w') return identTrie(ident, 2, 'itch', TOKEN_SWITCH)
        if (ident[1] === 't') return identTrie(ident, 2, 'ring', TOKEN_TO_STRING)
      }
      break
    case 'k':
      if (size === 4) return identTrie(ident, 1, 'eys', TOKEN_KEYS)
      break
    case 'c':
      if (size === 8) return identTrie(ident, 1, 'ontinue', TOKEN_CONTINUE)
      if (size === 4) {
        if (ident[1] === 'o') return identTrie(ident, 2, 'py', TOKEN_COPY)
        if (ident[1] === 'a') return identTrie(ident, 2, 'se', TOKEN_CASE)
      } else if (size === 5) {
        if (ident[1] === 'l') return identTrie(ident, 2, 'ear', TOKEN_CLEAR)
        if (ident[1] === 'o') return identTrie(ident, 2, 'nst', TOKEN_CONST)
      }
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
        if (ident[1] === 'y') return identTrie(ident, 2, 'pe', TOKEN_TYPE)
      }
      break
    case 'i':
      if (size === 3) return identTrie(ident, 1, 'nt', TOKEN_TO_INTEGER)
      if (size === 5) return identTrie(ident, 1, 'ndex', TOKEN_INDEX)
      if (size === 6) return identTrie(ident, 1, 'nsert', TOKEN_INSERT)
      if (size === 7) return identTrie(ident, 1, 'terate', TOKEN_ITERATE)
      if (size === 2) {
        if (ident[1] === 'f') return TOKEN_IF
        if (ident[1] === 'n') return TOKEN_IN
      }
      break
    case 'p':
      if (size === 3) return identTrie(ident, 1, 'op', TOKEN_POP)
      if (size === 5) return identTrie(ident, 1, 'rint', TOKEN_PRINT)
      if (size === 4) {
        if (ident[1] === 'u') return identTrie(ident, 2, 'sh', TOKEN_PUSH)
        if (ident[1] === 'a') return identTrie(ident, 2, 'ss', TOKEN_PASS)
      }
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
      if (size === 3) return identTrie(ident, 1, '||', TOKEN_FOR)
      if (size === 8) return identTrie(ident, 1, 'unction', TOKEN_FUNCTION)
      if (size === 5) {
        if (ident[1] === 'a') return identTrie(ident, 2, 'lse', TOKEN_FALSE)
        if (ident[1] === 'l') return identTrie(ident, 2, 'oat', TOKEN_TO_FLOAT)
      }
      break
  }
  return TOKEN_UNDEFINED
}

function pushIdentToken(compiler, start, end) {
  const ident = compiler.source.substring(start, end)
  const keyword = identKey(ident, end - start)
  if (keyword !== TOKEN_UNDEFINED) {
    valueToken(compiler, keyword, start, end)
  }
  valueToken(compiler, TOKEN_IDENT, start, end)
}

function isDigit(c) {
  return '0' <= c && c <= '9'
}

function isIdent(c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c === '_'
}

function advance(compiler) {
  copyToken(compiler.previous, compiler.current)
  if (compiler.previous.type === TOKEN_EOF) {
    return
  }
  while (true) {
    let c = nextChar(compiler)
    switch (c) {
      case '#':
        c = peekChar(compiler)
        while (c !== '\n' && c !== '\0') {
          nextChar(compiler)
          c = peekChar(compiler)
        }
        continue
      case ' ':
      case '\t':
      case '\r':
      case '\n':
        c = peekChar(compiler)
        while (c !== '\0' && (c === ' ' || c === '\t' || c === '\r' || c === '\n')) {
          nextChar(compiler)
          c = peekChar(compiler)
        }
        continue
      case '!':
        if (peekChar(compiler) === '=') {
          nextChar(compiler)
          tokenSpecial(compiler, TOKEN_NOT_EQUAL, 2, 2)
        } else {
          token(compiler, TOKEN_NOT)
        }
        return
      case '=':
        if (peekChar(compiler) === '=') {
          nextChar(compiler)
          tokenSpecial(compiler, TOKEN_EQUAL, 2, 2)
        } else {
          token(compiler, TOKEN_ASSIGN)
        }
        return
      case '>':
        if (peekChar(compiler) === '=') {
          nextChar(compiler)
          tokenSpecial(compiler, TOKEN_GREATER_EQUAL, 2, 2)
        } else if (peekChar(compiler) === '>') {
          nextChar(compiler)
          tokenSpecial(compiler, TOKEN_BIT_RIGHT_SHIFT, 2, 2)
        } else {
          token(compiler, TOKEN_GREATER)
        }
        return
      case '<':
        if (peekChar(compiler) === '=') {
          nextChar(compiler)
          tokenSpecial(compiler, TOKEN_LESS_EQUAL, 2, 2)
        } else if (peekChar(compiler) === '<') {
          nextChar(compiler)
          tokenSpecial(compiler, TOKEN_BIT_LEFT_SHIFT, 2, 2)
        } else {
          token(compiler, TOKEN_LESS)
        }
        return
      case '&':
        token(compiler, TOKEN_BIT_AND)
        return
      case '|':
        token(compiler, TOKEN_BIT_OR)
        return
      case '^':
        token(compiler, TOKEN_BIT_XOR)
        return
      case '~':
        token(compiler, TOKEN_BIT_NOT)
        return
      case '+':
        token(compiler, TOKEN_ADD)
        return
      case '-':
        token(compiler, TOKEN_SUBTRACT)
        return
      case '%':
        token(compiler, TOKEN_MODULO)
        return
      case '*':
        token(compiler, TOKEN_MULTIPLY)
        return
      case '/':
        token(compiler, TOKEN_DIVIDE)
        return
      case ',':
        token(compiler, TOKEN_COMMA)
        return
      case '.':
        token(compiler, TOKEN_DOT)
        return
      case '(':
        token(compiler, TOKEN_LEFT_PAREN)
        return
      case ')':
        token(compiler, TOKEN_RIGHT_PAREN)
        return
      case '[':
        token(compiler, TOKEN_LEFT_SQUARE)
        return
      case ']':
        token(compiler, TOKEN_RIGHT_SQUARE)
        return
      case '{':
        token(compiler, TOKEN_LEFT_CURLY)
        return
      case '}':
        token(compiler, TOKEN_RIGHT_CURLY)
        return
      case ':':
        token(compiler, TOKEN_COLON)
        return
      case ';':
        token(compiler, TOKEN_SEMICOLON)
        return
      case '\0':
        token(compiler, TOKEN_EOF)
        return
      case '"': {
        const start = compiler.pos
        while (true) {
          c = nextChar(compiler)
          if (c === '\\') {
            nextChar(compiler)
            continue
          } else if (c === '"' || c === '\0') {
            break
          }
        }
        const end = compiler.pos - 1
        valueToken(compiler, TOKEN_STRING, start, end)
        return
      }
      case "'": {
        const start = compiler.pos
        while (true) {
          c = nextChar(compiler)
          if (c === '\\') {
            nextChar(compiler)
            continue
          } else if (c === "'" || c === '\0') {
            break
          }
        }
        const end = compiler.pos - 1
        valueToken(compiler, TOKEN_STRING, start, end)
        return
      }
      default: {
        if (isDigit(c)) {
          const start = compiler.pos - 1
          while (isDigit(peekChar(compiler))) {
            nextChar(compiler)
          }
          let discrete = true
          if (peekChar(compiler) === '.') {
            discrete = false
            nextChar(compiler)
            while (isDigit(peekChar(compiler))) {
              nextChar(compiler)
            }
          }
          const end = compiler.pos
          if (discrete) {
            valueToken(compiler, TOKEN_INTEGER, start, end)
          } else {
            valueToken(compiler, TOKEN_FLOAT, start, end)
          }
          return
        } else if (isIdent(c)) {
          const start = compiler.pos - 1
          while (isIdent(peekChar(compiler))) {
            nextChar(compiler)
          }
          const end = compiler.pos
          pushIdentToken(compiler, start, end)
          return
        } else {
          compileError(compiler, compiler.current, 'Unknown Character: `' + c + '`')
        }
      }
    }
  }
}

function valueMatch(a, b) {
  if (a.is !== b.is) {
    return false
  }
  switch (a.is) {
    case HYMN_VALUE_UNDEFINED:
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

function scopeInit(compiler, scope, type) {
  scope.enclosing = compiler.scope
  compiler.scope = scope

  scope.localCount = 0
  scope.depth = 0
  scope.func = newFunction(compiler.script)
  scope.type = type

  if (type !== TYPE_SCRIPT) {
    scope.func.name = sourceSubstring(compiler, compiler.previous.len, compiler.previous.start)
  }

  const local = scopeGetLocal(scope, scope.localCount++)
  local.depth = 0
  local.nameStart = 0
  local.nameLen = 0
}

function newCompiler(script, source, hymn, scope) {
  const compiler = new Compiler(script, source, hymn)
  scopeInit(compiler, scope, TYPE_SCRIPT)
  return compiler
}

function byteCodeAddConstant(code, value) {
  code.constants.push(value)
  return code.constants.length - 1
}

function writeOp(code, byte, row) {
  const count = code.count
  const size = code.instructions.length
  if (count + 1 > size) {
    const instructions = new Uint8Array(size * 2)
    const rows = new Uint32Array(size * 2)
    for (let i = 0; i < size; i++) instructions[i] = code.instructions[i]
    for (let i = 0; i < size; i++) rows[i] = code.rows[i]
    code.instructions = instructions
    code.rows = rows
  }
  code.instructions[count] = byte
  code.rows[count] = row
  code.count = count + 1
}

function writeTwoOp(code, byteOne, byteTwo, row) {
  writeOp(code, byteOne, row)
  writeOp(code, byteTwo, row)
}

function writeConstant(compiler, value, row) {
  let constant = byteCodeAddConstant(current(compiler), value)
  if (constant > UINT8_MAX) {
    compileError(compiler, compiler.previous, 'Too many constants.')
    constant = 0
  }
  writeTwoOp(current(compiler), OP_CONSTANT, constant, row)
  return constant
}

function hymnSetGlobal(hymn, name, value) {
  hymn.globals.set(name, value)
}

function check(compiler, type) {
  return compiler.current.type === type
}

function match(compiler, type) {
  if (!check(compiler, type)) {
    return false
  }
  advance(compiler)
  return true
}

function emit(compiler, byte) {
  writeOp(current(compiler), byte, compiler.previous.row)
}

function emitTwo(compiler, byteOne, byteTwo) {
  writeTwoOp(current(compiler), byteOne, byteTwo, compiler.previous.row)
}

function compileWithPrecedence(compiler, precedence) {
  advance(compiler)
  const rule = rules[compiler.previous.type]
  const prefix = rule.prefix
  if (prefix === null) {
    compileError(compiler, compiler.previous, 'Syntax Error: Expected expression following `' + sourceSubstring(compiler, compiler.previous.len, compiler.previous.start) + '`.')
    return
  }
  const assign = precedence <= PRECEDENCE_ASSIGN
  prefix(compiler, assign)
  while (precedence <= rules[compiler.current.type].precedence) {
    advance(compiler)
    const infix = rules[compiler.previous.type].infix
    if (infix === null) {
      compileError(compiler, compiler.previous, 'Expected infix.')
    }
    infix(compiler, assign)
  }
  if (assign && match(compiler, TOKEN_ASSIGN)) {
    compileError(compiler, compiler.current, 'Invalid assignment target.')
  }
}

function consume(compiler, type, error) {
  if (compiler.current.type === type) {
    advance(compiler)
    return
  }
  compileError(compiler, compiler.current, error)
}

function pushHiddenLocal(compiler) {
  const scope = compiler.scope
  if (scope.localCount === HYMN_UINT8_COUNT) {
    compileError(compiler, compiler.previous, 'Too many local variables in scope.')
    return 0
  }
  const index = scope.localCount++
  const local = scopeGetLocal(scope, index)
  local.nameStart = 0
  local.nameLen = 0
  local.constant = true
  local.depth = scope.depth
  return index
}

function args(compiler) {
  let count = 0
  if (!check(compiler, TOKEN_RIGHT_PAREN)) {
    do {
      expression(compiler)
      if (count === UINT8_MAX) {
        compileError(compiler, compiler.previous, "Can't have more than 255 function arguments.")
        break
      }
      count++
    } while (match(compiler, TOKEN_COMMA))
  }
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after function arguments.")
  return count
}

function compileCall(compiler, assign) {
  emitTwo(compiler, OP_CALL, args(compiler))
}

function compileGroup(compiler, assign) {
  expression(compiler)
  consume(compiler, TOKEN_RIGHT_PAREN, 'Expected right parenthesis.')
}

function compileNone(compiler, assign) {
  emit(compiler, OP_NONE)
}

function compileTrue(compiler, assign) {
  emit(compiler, OP_TRUE)
}

function compileFalse(compiler, assign) {
  emit(compiler, OP_FALSE)
}

function compileInteger(compiler, assign) {
  const alpha = compiler.previous
  const number = parseInt(sourceSubstring(compiler, alpha.len, alpha.start))
  writeConstant(compiler, newInt(number), alpha.row)
}

function compileFloat(compiler, assign) {
  const alpha = compiler.previous
  const number = parseFloat(sourceSubstring(compiler, alpha.len, alpha.start))
  writeConstant(compiler, newFloat(number), alpha.row)
}

function compileString(compiler, assign) {
  const alpha = compiler.previous
  const string = sourceSubstring(compiler, alpha.len, alpha.start)
  writeConstant(compiler, newString(string), alpha.row)
}

function identConstant(compiler, token) {
  const string = sourceSubstring(compiler, token.len, token.start)
  return byteCodeAddConstant(current(compiler), newString(string))
}

function beginScope(compiler) {
  compiler.scope.depth++
}

function endScope(compiler) {
  const scope = compiler.scope
  scope.depth--
  while (scope.localCount > 0 && scope.locals[scope.localCount - 1].depth > scope.depth) {
    emit(compiler, OP_POP)
    scope.localCount--
  }
}

function compileArray(compiler, assign) {
  writeConstant(compiler, newArrayValue(null), compiler.previous.row)
  if (match(compiler, TOKEN_RIGHT_SQUARE)) {
    return
  }
  while (!check(compiler, TOKEN_RIGHT_SQUARE) && !check(compiler, TOKEN_EOF)) {
    emit(compiler, OP_DUPLICATE)
    expression(compiler)
    emitTwo(compiler, OP_ARRAY_PUSH, OP_POP)
    if (!check(compiler, TOKEN_RIGHT_SQUARE)) {
      consume(compiler, TOKEN_COMMA, "Expected ','.")
    }
  }
  consume(compiler, TOKEN_RIGHT_SQUARE, "Expected ']' declaring array.")
}

function compileTable(compiler, assign) {
  writeConstant(compiler, new_table_value(null), compiler.previous.row)
  if (match(compiler, TOKEN_RIGHT_CURLY)) {
    return
  }
  while (!check(compiler, TOKEN_RIGHT_CURLY) && !check(compiler, TOKEN_EOF)) {
    emit(compiler, OP_DUPLICATE)
    consume(compiler, TOKEN_IDENT, 'Expected property name')
    const name = ident_constant(compiler, compiler.previous)
    consume(compiler, TOKEN_COLON, "Expected ':'.")
    expression(compiler)
    emitTwo(compiler, OP_SET_PROPERTY, name)
    emit(compiler, OP_POP)
    if (!check(compiler, TOKEN_RIGHT_CURLY)) {
      consume(compiler, TOKEN_COMMA, "Expected ','.")
    }
  }
  consume(compiler, TOKEN_RIGHT_CURLY, "Expected '}' declaring table.")
}

function pushLocal(compiler, name, isConstant) {
  const scope = compiler.scope
  if (scope.localCount === HYMN_UINT8_COUNT) {
    compileError(compiler, name, 'Too many local variables in scope')
    return
  }
  const local = scope.locals[scope.localCount++]
  local.name = name
  local.constant = isConstant
  local.depth = -1
}

function variable(compiler, isConstant, error) {
  consume(compiler, TOKEN_IDENT, error)
  const scope = compiler.scope
  if (scope.depth === 0) {
    return identConstant(compiler, compiler.previous)
  }
  const name = compiler.previous
  for (let i = scope.localCount - 1; i >= 0; i--) {
    const local = scope.locals[i]
    if (local.depth !== -1 && local.depth < scope.depth) {
      break
    } else if (name === local.name) {
      compileError(compiler, name, 'Scope Error: Variable `' + sourceSubstring(compiler, name.len, name.start) + '` already exists in compiler scope.')
    }
  }
  pushLocal(compiler, name, isConstant)
  return 0
}

function localInitialize(compiler) {
  const scope = compiler.scope
  if (scope.depth === 0) {
    return
  }
  scope.locals[scope.localCount - 1].depth = scope.depth
}

function finalizeVariable(compiler, global) {
  if (compiler.scope.depth > 0) {
    localInitialize(compiler)
    return
  }
  emitTwo(compiler, OP_DEFINE_GLOBAL, global)
}

function defineNewVariable(compiler, isConstant) {
  const v = variable(compiler, isConstant, 'Syntax Error: Expected variable name.')
  consume(compiler, TOKEN_ASSIGN, "Assignment Error: Expected '=' after variable.")
  expression(compiler)
  finalizeVariable(compiler, v)
}

function resolveLocal(compiler, name) {
  const scope = compiler.scope
  for (let i = scope.localCount - 1; i >= 0; i--) {
    const local = scope.locals[i]
    if (name === local.name) {
      if (local.depth === -1) {
        compileError(compiler, name, 'Reference Error: Local variable `' + sourceSubstring(name.len, name.start) + '` referenced before assignment.')
      }
      return [i, local.constant]
    }
  }
  return [-1, false]
}

function namedVariable(compiler, token, assign) {
  let get
  let set
  let r = resolveLocal(compiler, token)
  let v = r[0]
  let constant = r[1]
  if (v !== -1) {
    get = OP_GET_LOCAL
    set = OP_SET_LOCAL
  } else {
    get = OP_GET_GLOBAL
    set = OP_SET_GLOBAL
    v = identConstant(compiler, token)
  }
  if (assign && match(compiler, TOKEN_ASSIGN)) {
    if (constant) {
      compileError(compiler, token, "Constant variable can't be modified.")
    }
    expression(compiler)
    emit(compiler, set)
  } else {
    emit(compiler, get)
  }
  emit(compiler, v)
}

function compileVariable(compiler, assign) {
  namedVariable(compiler, compiler.previous, assign)
}

function compileUnary(compiler, assign) {
  const type = compiler.previous.type
  compileWithPrecedence(compiler, PRECEDENCE_UNARY)
  switch (type) {
    case TOKEN_NOT:
      emit(compiler, OP_NOT)
      break
    case TOKEN_SUBTRACT:
      emit(compiler, OP_NEGATE)
      break
    case TOKEN_BIT_NOT:
      emit(compiler, OP_BIT_NOT)
      break
    default:
      return
  }
}

function compileBinary(compiler, assign) {
  const type = compiler.previous.type
  const rule = rules[type]
  compileWithPrecedence(compiler, rule.precedence + 1)
  switch (type) {
    case TOKEN_ADD:
      emit(compiler, OP_ADD)
      break
    case TOKEN_SUBTRACT:
      emit(compiler, OP_SUBTRACT)
      break
    case TOKEN_MODULO:
      emit(compiler, OP_MODULO)
      break
    case TOKEN_MULTIPLY:
      emit(compiler, OP_MULTIPLY)
      break
    case TOKEN_DIVIDE:
      emit(compiler, OP_DIVIDE)
      break
    case TOKEN_EQUAL:
      emit(compiler, OP_EQUAL)
      break
    case TOKEN_NOT_EQUAL:
      emit(compiler, OP_NOT_EQUAL)
      break
    case TOKEN_LESS:
      emit(compiler, OP_LESS)
      break
    case TOKEN_LESS_EQUAL:
      emit(compiler, OP_LESS_EQUAL)
      break
    case TOKEN_GREATER:
      emit(compiler, OP_GREATER)
      break
    case TOKEN_GREATER_EQUAL:
      emit(compiler, OP_GREATER_EQUAL)
      break
    case TOKEN_BIT_OR:
      emit(compiler, OP_BIT_OR)
      break
    case TOKEN_BIT_AND:
      emit(compiler, OP_BIT_AND)
      break
    case TOKEN_BIT_XOR:
      emit(compiler, OP_BIT_XOR)
      break
    case TOKEN_BIT_LEFT_SHIFT:
      emit(compiler, OP_BIT_LEFT_SHIFT)
      break
    case TOKEN_BIT_RIGHT_SHIFT:
      emit(compiler, OP_BIT_RIGHT_SHIFT)
      break
    default:
      return
  }
}

function compileDot(compiler, assign) {
  consume(compiler, TOKEN_IDENT, "Expected property name after '.'.")
  const name = identConstant(compiler, compiler.previous)
  if (assign && match(compiler, TOKEN_ASSIGN)) {
    expression(compiler)
    emitTwo(compiler, OP_SET_PROPERTY, compiler)
  } else {
    emitTwo(compiler, OP_GET_PROPERTY, compiler)
  }
}

function compileSquare(compiler, assign) {
  if (match(compiler, TOKEN_COLON)) {
    writeConstant(compiler, newInt(0), compiler.previous.row)
    if (match(compiler, TOKEN_RIGHT_SQUARE)) {
      writeConstant(compiler, newNone(), compiler.previous.row)
    } else {
      expression(compiler)
      consume(compiler, TOKEN_RIGHT_SQUARE, "Expected ']' after expression.")
    }
    emit(compiler, OP_SLICE)
  } else {
    expression(compiler)
    if (match(compiler, TOKEN_COLON)) {
      if (match(compiler, TOKEN_RIGHT_SQUARE)) {
        writeConstant(compiler, newNone(), compiler.previous.row)
      } else {
        expression(compiler)
        consume(compiler, TOKEN_RIGHT_SQUARE, "Expected ']' after expression.")
      }
      emit(compiler, OP_SLICE)
    } else {
      consume(compiler, TOKEN_RIGHT_SQUARE, "Expected ']' after expression.")
      if (assign && match(compiler, TOKEN_ASSIGN)) {
        expression(compiler)
        emit(compiler, OP_SET_DYNAMIC)
      } else {
        emit(compiler, OP_GET_DYNAMIC)
      }
    }
  }
}

function emitJump(compiler, instruction) {
  emit(compiler, instruction)
  emitTwo(compiler, UINT8_MAX, UINT8_MAX)
  return current(compiler).count - 2
}

function patchJump(compiler, jump) {
  const code = current(compiler)
  const offset = code.count - jump - 2
  if (offset > UINT16_MAX) {
    compileError(compiler, compiler.previous, 'Jump offset too large.')
    return
  }
  code.instructions[jump] = (offset >> 8) & UINT8_MAX
  code.instructions[jump + 1] = offset & UINT8_MAX
}

function compileAnd(compiler, assign) {
  const jump = emitJump(compiler, OP_JUMP_IF_FALSE)
  emit(compiler, OP_POP)
  compileWithPrecedence(compiler, PRECEDENCE_AND)
  patchJump(compiler, jump)
}

function compileOr(compiler, assign) {
  const jumpElse = emitJump(compiler, OP_JUMP_IF_FALSE)
  const jump = emitJump(compiler, OP_JUMP)
  patchJump(compiler, jumpElse)
  emit(compiler, OP_POP)
  compileWithPrecedence(compiler, PRECEDENCE_OR)
  patchJump(compiler, jump)
}

function endFunction(compiler) {
  emitTwo(compiler, OP_NONE, OP_RETURN)
  const func = compiler.scope.func
  compiler.scope = compiler.scope.enclosing
  return func
}

function compileFunction(compiler, type) {
  const scope = new Scope()
  scopeInit(compiler, scope, type)

  beginScope(compiler)

  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after function name.")

  if (!check(compiler, TOKEN_RIGHT_PAREN)) {
    do {
      compiler.scope.func.arity++
      if (compiler.scope.func.arity > 255) {
        compileError(compiler, compiler.previous, "Can't have more than 255 function parameters.")
      }
      const parameter = variable(compiler, false, 'Expected parameter name.')
      finalizeVariable(compiler, parameter)
    } while (match(compiler, TOKEN_COMMA))
  }

  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after function parameters.")

  while (!check(compiler, TOKEN_END) && !check(compiler, TOKEN_EOF)) {
    declaration(compiler)
  }

  endScope(compiler)
  consume(compiler, TOKEN_END, "Expected 'end' after function body.")

  const func = endFunction(compiler)
  writeConstant(compiler, newFuncValue(func), compiler.previous.row)
}

function declareFunction(compiler) {
  const global = variable(compiler, false, 'Expected function name.')
  localInitialize(compiler)
  compileFunction(compiler, TYPE_FUNCTION)
  finalizeVariable(compiler, global)
}

function declaration(compiler) {
  if (match(compiler, TOKEN_LET)) {
    defineNewVariable(compiler, false)
  } else if (match(compiler, TOKEN_CONST)) {
    defineNewVariable(compiler, true)
  } else if (match(compiler, TOKEN_FUNCTION)) {
    declareFunction(compiler)
  } else {
    statement(compiler)
  }
}

function block(compiler) {
  beginScope(compiler)
  while (!check(compiler, TOKEN_END) && !check(compiler, TOKEN_EOF)) {
    declaration(compiler)
  }
  endScope(compiler)
}

function ifStatement(compiler) {
  expression(compiler)
  const jump = emitJump(compiler, OP_JUMP_IF_FALSE)

  emit(compiler, OP_POP)
  beginScope(compiler)
  while (!check(compiler, TOKEN_ELIF) && !check(compiler, TOKEN_ELSE) && !check(compiler, TOKEN_END) && !check(compiler, TOKEN_EOF)) {
    declaration(compiler)
  }
  endScope(compiler)

  const jumpEnd = new JumpList()
  jumpEnd.jump = emitJump(compiler, OP_JUMP)
  let tail = jumpEnd

  while (match(compiler, TOKEN_ELIF)) {
    patchJump(compiler, jump)
    emit(compiler, OP_POP)

    expression(compiler)
    jump = emitJump(compiler, OP_JUMP_IF_FALSE)

    emit(compiler, OP_POP)
    beginScope(compiler)
    while (!check(compiler, TOKEN_ELIF) && !check(compiler, TOKEN_ELSE) && !check(compiler, TOKEN_END) && !check(compiler, TOKEN_EOF)) {
      declaration(compiler)
    }
    endScope(compiler)

    const next = new JumpList()
    next.jump = emitJump(compiler, OP_JUMP)

    tail.next = next
    tail = next
  }

  patchJump(compiler, jump)
  emit(compiler, OP_POP)

  if (match(compiler, TOKEN_ELSE)) {
    block(compiler)
  }

  patchJump(compiler, jumpEnd.jump)
  let current = jumpEnd.next
  while (current !== null) {
    patchJump(compiler, current.jump)
    current = current.next
  }

  consume(compiler, TOKEN_END, "Expected 'end' after if statement.")
}

function compileLiteral(compiler) {
  advance(compiler)
  switch (compiler.previous.type) {
    case TOKEN_NONE:
      compileNone(compiler, false)
      return true
    case TOKEN_TRUE:
      compileTrue(compiler, false)
      return true
    case TOKEN_FALSE:
      compileFalse(compiler, false)
      return true
    case TOKEN_INTEGER:
      compileInteger(compiler, false)
      return true
    case TOKEN_FLOAT:
      compileFloat(compiler, false)
      return true
    case TOKEN_STRING:
      compileString(compiler, false)
      return true
    default:
      return false
  }
}

function switchStatement(compiler) {
  beginScope(compiler)

  const local = pushHiddenLocal(compiler)
  expression(compiler)

  if (!check(compiler, TOKEN_CASE)) {
    compileError(compiler, compiler.current, 'Expected case.')
    return
  }

  let jump = -1

  let head = null
  let tail = null

  while (match(compiler, TOKEN_CASE)) {
    if (jump !== -1) {
      patchJump(compiler, jump)
      emit(compiler, OP_POP)
    }

    if (!compileLiteral(compiler)) {
      compileError(compiler, compiler.current, 'Expected literal for case.')
    }
    emitTwo(compiler, OP_GET_LOCAL, local)
    emit(compiler, OP_EQUAL)

    let body = null

    if (match(compiler, TOKEN_OR)) {
      body = new JumpList()
      let link = body
      body.jump = emitJump(compiler, OP_JUMP_IF_TRUE)
      emit(compiler, OP_POP)

      while (true) {
        if (!compileLiteral(compiler)) {
          compileError(compiler, compiler.current, "Expected literal after 'or' in case.")
        }
        emitTwo(compiler, OP_GET_LOCAL, local)
        emit(compiler, OP_EQUAL)

        if (match(compiler, TOKEN_OR)) {
          let next = new JumpList()
          next.jump = emitJump(compiler, OP_JUMP_IF_TRUE)
          emit(compiler, OP_POP)

          link.next = next
          link = next
        } else {
          break
        }
      }
    }

    jump = emitJump(compiler, OP_JUMP_IF_FALSE)

    while (body !== null) {
      patchJump(compiler, body.jump)
      body = body.next
    }

    emit(compiler, OP_POP)

    beginScope(compiler)
    while (!check(compiler, TOKEN_CASE) && !check(compiler, TOKEN_ELSE) && !check(compiler, TOKEN_END) && !check(compiler, TOKEN_EOF)) {
      declaration(compiler)
    }
    endScope(compiler)

    let next = new JumpList()
    next.jump = emitJump(compiler, OP_JUMP)

    if (head === null) {
      head = next
      tail = next
    } else {
      tail.next = next
      tail = next
    }
  }

  if (jump !== -1) {
    patchJump(compiler, jump)
    emit(compiler, OP_POP)
  }

  if (match(compiler, TOKEN_ELSE)) {
    block(compiler)
  }

  while (head !== null) {
    patchJump(compiler, head.jump)
    head = head.next
  }

  endScope(compiler)

  consume(compiler, TOKEN_END, "Expected 'end' after switch statement.")
}

function emitLoop(compiler, start) {
  emit(compiler, OP_LOOP)
  const offset = current(compiler).count - start + 2
  if (offset > UINT16_MAX) {
    compileError(compiler, compiler.previous, 'Loop is too large.')
  }
  emitTwo(compiler, (offset >> 8) & UINT8_MAX, offset & UINT8_MAX)
}

function patchJumpList(compiler) {
  while (compiler.jump !== null) {
    let depth = 1
    if (compiler.loop !== null) {
      depth = compiler.loop.depth + 1
    }
    if (compiler.jump.depth < depth) {
      break
    }
    patchJump(compiler, compiler.jump.jump)
    compiler.jump = compiler.jump.next
  }
}

function patchIteratorJumpList(compiler) {
  while (compiler.iteratorJump !== null) {
    let depth = 1
    if (compiler.loop !== null) {
      depth = compiler.loop.depth + 1
    }
    if (compiler.iteratorJump.depth < depth) {
      break
    }
    patchJump(compiler, compiler.iteratorJump.jump)
    compiler.iteratorJump = compiler.iteratorJump.next
  }
}

function iterateStatement(compiler) {
  beginScope(compiler)

  // parameters

  let index

  let value = compiler.scope.localCount
  variable(compiler, true, 'Expected parameter name.')
  localInitialize(compiler)

  if (match(compiler, TOKEN_COMMA)) {
    index = value
    writeConstant(compiler, newInt(0), compiler.previous.row)

    value = compiler.scope.local_count
    variable(compiler, true, 'Expected second parameter name.')
    localInitialize(compiler)
    emit(compiler, OP_NONE)
  } else {
    emit(compiler, OP_NONE)

    index = pushHiddenLocal(compiler)
    writeConstant(compiler, newInt(0), compiler.previous.row)
  }

  consume(compiler, TOKEN_IN, "Expected 'in' after iterate parameters.")

  // setup

  const local = pushHiddenLocal(compiler)
  expression(compiler)

  const len = pushHiddenLocal(compiler)
  emitTwo(compiler, OP_GET_LOCAL, local)
  emit(compiler, OP_LEN)

  // compare

  const compare = current(compiler).count

  const loop = new LoopList()
  loop.start = -1
  loop.depth = compiler.scope.depth + 1
  loop.next = compiler.loop
  compiler.loop = loop

  emitTwo(compiler, OP_GET_LOCAL, index)
  emitTwo(compiler, OP_GET_LOCAL, len)
  emit(compiler, OP_LESS)

  const jump = emitJump(compiler, OP_JUMP_IF_FALSE)
  emit(compiler, OP_POP)

  // update

  emitTwo(compiler, OP_GET_LOCAL, local)
  emitTwo(compiler, OP_GET_LOCAL, index)
  emit(compiler, OP_GET_DYNAMIC)

  emitTwo(compiler, OP_SET_LOCAL, value)
  emit(compiler, OP_POP)

  // inside

  block(compiler)

  // increment

  patchIteratorJumpList(compiler)

  emitTwo(compiler, OP_GET_LOCAL, index)
  writeConstant(compiler, newInt(1), compiler.previous.row)
  emit(compiler, OP_ADD)
  emitTwo(compiler, OP_SET_LOCAL, index)
  emit(compiler, OP_POP)

  // next

  emitLoop(compiler, compare)

  // end

  compiler.loop = loop.next

  patchJump(compiler, jump)
  emit(compiler, OP_POP)

  patchJumpList(compiler)
  endScope(compiler)

  consume(compiler, TOKEN_END, "Expected 'end'.")
}

function forStatement(compiler) {
  beginScope(compiler)

  // assign

  if (match(compiler, TOKEN_LET)) {
    defineNewVariable(compiler, false)
  } else if (match(compiler, TOKEN_CONST)) {
    defineNewVariable(compiler, true)
  } else if (!check(compiler, TOKEN_SEMICOLON)) {
    expression_statement(compiler)
  }

  consume(compiler, TOKEN_SEMICOLON, "Expected ';' in for.")

  // compare

  const compare = current(compiler).count

  expression(compiler)

  const jump = emitJump(compiler, OP_JUMP_IF_FALSE)
  emit(compiler, OP_POP)

  consume(compiler, TOKEN_SEMICOLON, "Expected ';' in for.")

  // increment

  const body = emitJump(compiler, OP_JUMP)
  const increment = current(compiler).count

  const loop = new LoopList()
  loop.start = increment
  loop.depth = compiler.scope.depth + 1
  loop.next = compiler.loop
  compiler.loop = loop

  expression(compiler)

  emit(compiler, OP_POP)
  emitLoop(compiler, compare)

  // body

  patchJump(compiler, body)

  block(compiler)
  emitLoop(compiler, increment)

  // end

  compiler.loop = loop.next

  patchJump(compiler, jump)
  emit(compiler, OP_POP)

  patchJumpList(compiler)
  endScope(compiler)

  consume(compiler, TOKEN_END, "Expected 'end' after for loop.")
}

function whileStatement(compiler) {
  const start = current(compiler).count

  const loop = new LoopList()
  loop.start = start
  loop.depth = compiler.scope.depth + 1
  loop.next = compiler.loop
  compiler.loop = loop

  expression(compiler)
  const jump = emitJump(compiler, OP_JUMP_IF_FALSE)

  emit(compiler, OP_POP)
  block(compiler)
  emitLoop(compiler, start)

  compiler.loop = loop.next

  patchJump(compiler, jump)
  emit(compiler, OP_POP)

  patchJumpList(compiler)

  consume(compiler, TOKEN_END, "Expected 'end' after while loop.")
}

function returnStatement(compiler) {
  if (compiler.scope.type === TYPE_SCRIPT) {
    compileError(compiler, compiler.previous, "Can't return from outside a function.")
  }
  expression(compiler)
  emit(compiler, OP_RETURN)
}

function popStackLoop(compiler) {
  const depth = compiler.loop.depth
  const scope = compiler.scope
  for (let i = scope.localCount; i > 0; i--) {
    if (scope.locals[i - 1].depth < depth) {
      return
    }
    emit(compiler, OP_POP)
  }
}

function breakStatement(compiler) {
  if (compiler.loop === null) {
    compileError(compiler, compiler.previous, "Can't use 'break' outside of a loop.")
  }
  popStackLoop(compiler)
  const jumpNext = compiler.jump
  const jump = new JumpList()
  jump.jump = emitJump(compiler, OP_JUMP)
  jump.depth = compiler.loop.depth
  jump.next = jumpNext
  compiler.jump = jump
}

function continueStatement(compiler) {
  if (compiler.loop === null) {
    compileError(compiler, compiler.previous, "Can't use 'continue' outside of a loop.")
  }
  popStackLoop(compiler)
  if (compiler.loop.start === -1) {
    const jumpNext = compiler.iterator_jump
    const jump = new JumpList()
    jump.jump = emitJump(compiler, OP_JUMP)
    jump.depth = compiler.loop.depth
    jump.next = jumpNext
    compiler.iterator_jump = jump
  } else {
    emit_loop(compiler, compiler.loop.start)
  }
}

function tryStatement(compiler) {
  const except = new ExceptList()
  except.stack = compiler.scope.localCount
  except.start = current(compiler).count

  const func = currentFunc(compiler)
  except.next = func.except
  func.except = except

  beginScope(compiler)
  while (!check(compiler, TOKEN_EXCEPT) && !check(compiler, TOKEN_EOF)) {
    declaration(compiler)
  }
  endScope(compiler)

  const jump = emitJump(compiler, OP_JUMP)

  consume(compiler, TOKEN_EXCEPT, "Expected 'except' after 'try'.")

  except.end = current(compiler).count

  beginScope(compiler)
  const message = variable(compiler, false, "Expected variable after 'except'.")
  finalize_variable(compiler, message)
  while (!check(compiler, TOKEN_END) && !check(compiler, TOKEN_EOF)) {
    declaration(compiler)
  }
  endScope(compiler)

  consume(compiler, TOKEN_END, "Expected 'end' after 'except'.")

  patchJump(compiler, jump)
}

function printStatement(compiler) {
  expression(compiler)
  emit(compiler, OP_PRINT)
}

function useStatement(compiler) {
  expression(compiler)
  emit(compiler, OP_USE)
}

function doStatement(compiler) {
  expression(compiler)
  emit(compiler, OP_DO)
}

function throwStatement(compiler) {
  expression(compiler)
  emit(compiler, OP_THROW)
}

function statement(compiler) {
  if (match(compiler, TOKEN_PRINT)) {
    printStatement(compiler)
  } else if (match(compiler, TOKEN_DO)) {
    doStatement(compiler)
  } else if (match(compiler, TOKEN_USE)) {
    useStatement(compiler)
  } else if (match(compiler, TOKEN_IF)) {
    ifStatement(compiler)
  } else if (match(compiler, TOKEN_SWITCH)) {
    switchStatement(compiler)
  } else if (match(compiler, TOKEN_ITERATE)) {
    iterateStatement(compiler)
  } else if (match(compiler, TOKEN_FOR)) {
    forStatement(compiler)
  } else if (match(compiler, TOKEN_WHILE)) {
    whileStatement(compiler)
  } else if (match(compiler, TOKEN_RETURN)) {
    returnStatement(compiler)
  } else if (match(compiler, TOKEN_BREAK)) {
    breakStatement(compiler)
  } else if (match(compiler, TOKEN_CONTINUE)) {
    continueStatement(compiler)
  } else if (match(compiler, TOKEN_TRY)) {
    tryStatement(compiler)
  } else if (match(compiler, TOKEN_THROW)) {
    throwStatement(compiler)
  } else if (match(compiler, TOKEN_PASS)) {
    // do nothing
  } else if (match(compiler, TOKEN_BEGIN)) {
    block(compiler)
    consume(compiler, TOKEN_END, "Expected 'end' after block.")
  } else {
    expressionStatement(compiler)
  }
}

function arrayPushExpression(compiler, assign) {
  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after push.")
  expression(compiler)
  consume(compiler, TOKEN_COMMA, "Expected ',' between push arguments.")
  expression(compiler)
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after push expression.")
  emit(compiler, OP_ARRAY_PUSH)
}

function arrayInsertExpression(compiler, assign) {
  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after insert.")
  expression(compiler)
  consume(compiler, TOKEN_COMMA, "Expected ',' between insert arguments.")
  expression(compiler)
  consume(compiler, TOKEN_COMMA, "Expected ',' between insert arguments.")
  expression(compiler)
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after insert expression.")
  emit(compiler, OP_ARRAY_INSERT)
}

function arrayPopExpression(compiler, assign) {
  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after pop.")
  expression(compiler)
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after pop expression.")
  emit(compiler, OP_ARRAY_POP)
}

function deleteExpression(compiler, assign) {
  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after delete.")
  expression(compiler)
  consume(compiler, TOKEN_COMMA, "Expected ',' between delete arguments.")
  expression(compiler)
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after delete expression.")
  emit(compiler, OP_DELETE)
}

function lenExpression(compiler, assign) {
  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after len.")
  expression(compiler)
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after len expression.")
  emit(compiler, OP_LEN)
}

function castIntegerExpression(compiler, assign) {
  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after integer.")
  expression(compiler)
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after integer expression.")
  emit(compiler, OP_TO_INTEGER)
}

function castFloatExpression(compiler, assign) {
  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after float.")
  expression(compiler)
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after float expression.")
  emit(compiler, OP_TO_FLOAT)
}

function castStringExpression(compiler, assign) {
  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after string.")
  expression(compiler)
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after string expression.")
  emit(compiler, OP_TO_STRING)
}

function typeExpression(compiler, assign) {
  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after type.")
  expression(compiler)
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after type expression.")
  emit(compiler, OP_TYPE)
}

function clearExpression(compiler, assign) {
  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after clear.")
  expression(compiler)
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after clear expression.")
  emit(compiler, OP_CLEAR)
}

function copyExpression(compiler, assign) {
  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after copy.")
  expression(compiler)
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after copy expression.")
  emit(compiler, OP_COPY)
}

function keysExpression(compiler, assign) {
  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after keys.")
  expression(compiler)
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after keys expression.")
  emit(compiler, OP_KEYS)
}

function indexExpression(compiler, assign) {
  consume(compiler, TOKEN_LEFT_PAREN, "Missing '(' for paramters in `index` function.")
  expression(compiler)
  consume(compiler, TOKEN_COMMA, 'Expected 2 arguments for `index` function.')
  expression(compiler)
  consume(compiler, TOKEN_RIGHT_PAREN, "Missing ')' after parameters in `index` function.")
  emit(compiler, OP_INDEX)
}

function expressionStatement(compiler) {
  expression(compiler)
  emit(compiler, OP_POP)
}

function expression(compiler) {
  compileWithPrecedence(compiler, PRECEDENCE_ASSIGN)
}

function parentFrame(hymn, offset) {
  const frameCount = hymn.frameCount
  if (offset > frameCount) {
    return null
  }
  return hymn.frames[frameCount - offset]
}

function currentFrame(hymn) {
  return hymn.frames[hymn.frameCount - 1]
}

function compile(hymn, script, source) {
  const scope = new Scope()
  const compiler = newCompiler(script, source, hymn, scope)

  advance(compiler)
  while (!match(compiler, TOKEN_EOF)) {
    declaration(compiler)
  }

  const func = endFunction(compiler)
  return { func: func, error: compiler.error }
}

function hymnResetStack(hymn) {
  hymn.stackTop = 0
  hymn.frameCount = 0
}

function isObject(value) {
  switch (value.is) {
    case HYMN_VALUE_STRING:
    case HYMN_VALUE_ARRAY:
    case HYMN_VALUE_TABLE:
    case HYMN_VALUE_FUNC:
      return true
    default:
      return false
  }
}

function hymnStackGet(hymn, index) {
  if (index < hymn.stack.length) {
    return hymn.stack[index]
  }
  const value = new HymnValue(HYMN_VALUE_UNDEFINED, null)
  hymn.stack.push(value)
  return value
}

function hymnPush(hymn, value) {
  copyValue(hymnStackGet(hymn, hymn.stackTop++), value)
}

function hymnPeek(hymn, dist) {
  if (dist > hymn.stackTop) {
    console.error('Nothing on stack to peek')
    return newNone()
  }
  return hymn.stack[hymn.stackTop - dist]
}

function hymnPop(hymn) {
  if (hymn.stackTop === 0) {
    console.error('Nothing on stack to pop')
    return newNone()
  }
  return hymn.stack[--hymn.stackTop]
}

function valueToNewString(value) {
  switch (value.is) {
    case HYMN_VALUE_UNDEFINED:
    case HYMN_VALUE_NONE:
      return STRING_NONE
    case HYMN_VALUE_BOOL:
      return value.value ? STRING_TRUE : STRING_FALSE
    case HYMN_VALUE_INTEGER:
    case HYMN_VALUE_FLOAT:
    case HYMN_VALUE_STRING:
    case HYMN_VALUE_ARRAY:
    case HYMN_VALUE_TABLE:
    case HYMN_VALUE_FUNC:
    case HYMN_VALUE_FUNC_NATIVE:
    case HYMN_VALUE_POINTER:
      return value.value.toString()
  }
  return null
}

function hymnException(hymn) {
  const frame = currentFrame(hymn)
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
    const result = hymnPop(hymn)
    if (except !== null) {
      while (hymn.stack_top > frame.stack + except.stack) {
        hymn.stack[--hymn.stack_top]
      }
      frame.ip = except.end
      hymnPush(hymn, result)
      return MACHINE_ERROR
    }
    while (hymn.stack_top != frame.stack) {
      hymn.stack[--hymn.stack_top]
    }
    hymn.frame_count--
    if (hymn.frame_count == 0 || frame.func.name === null) {
      hymn.error = valueToNewString(result)
      return MACHINE_FATAL
    }
    hymnPush(hymn, result)
    frame = currentFrame(hymn)
  }
}

function hymnStacktrace(hymn) {
  let trace = ''

  for (let i = hymn.frameCount - 1; i >= 0; i--) {
    const frame = hymn.frames[i]
    const func = frame.func
    const ip = frame.ip - 1
    const row = func.code.rows[ip]

    trace += 'at'

    if (func.name !== null) {
      trace += ' ' + func.name
    }

    if (func.script === null) {
      trace += ' script:'
    } else {
      trace += ' ' + func.script + ':'
    }

    trace += row
  }

  return trace
}

function hymnPushError(hymn, error) {
  hymnPush(hymn, newString(error))
  return hymnException(hymn)
}

function hymnThrowExistingError(hymn, error) {
  return machinePushError(hymn, error)
}

function hymnThrowError(hymn, error) {
  error += '\n\n'
  error += hymnStacktrace(hymn)
  return hymnPushError(hymn, error)
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

function hymnFalse(hymn) {
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
      return value.length === 0
    case HYMN_VALUE_ARRAY:
    case HYMN_VALUE_TABLE:
    case HYMN_VALUE_FUNC:
    case HYMN_VALUE_FUNC_NATIVE:
      return value.value === null
    default:
      return false
  }
}

function hymnFrameGet(hymn, index) {
  if (index < hymn.frames.length) {
    return hymn.frames[index]
  }
  const frame = new HymnFrame()
  hymn.frames.push(frame)
  return frame
}

function hymnCall(hymn, func, count) {
  if (count !== func.arity) {
    return hymnThrowError(hymn, 'Expected ' + func.arity + ' function arguments but found ' + count + '.')
  } else if (hymn.frameCount === HYMN_FRAMES_MAX) {
    return hymnThrowError(hymn, 'Stack overflow.')
  }

  const frame = hymnFrameGet(hymn, hymn.frameCount++)
  frame.func = func
  frame.ip = 0
  frame.stack = hymn.stackTop - count - 1

  return MACHINE_OK
}

function hymnCallValue(hymn, call, count) {
  switch (call.is) {
    case HYMN_VALUE_FUNC:
      return hymnCall(hymn, call.value, count)
    case HYMN_VALUE_FUNC_NATIVE: {
      const func = call.value.func
      const result = func(hymn, count, hymn.stack[hymn.stackTop - count])
      const top = hymn.stackTop - (count + 1)
      while (hymn.stackTop !== top) {
        --hymn.stackTop
      }
      hymnPush(hymn, result)
      return MACHINE_OK
    }
    default:
      return hymnThrowError(hymn, 'Only functions can be called.')
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

function hymnDo(hymn, source) {
  const result = compile(hymn, null, source)

  const func = result.func
  const error = result.error

  if (error) {
    return machineThrowExistingError(hymn, error)
  }

  const funcValue = newFuncValue(func)

  hymnPush(hymn, funcValue)
  hymnCall(hymn, func, 0)

  error = machineInterpretInternal(hymn)
  if (error) {
    return machineThrowExistingError(hymn, error)
  }

  return MACHINE_OK
}

function hymnImport(hymn, file) {
  throw 'Import not implemented'
}

function debugConstantInstruction(name, code, index) {
  const constant = code.instructions[index + 1]
  console.warn('--.', code.constants)
  return name + ': [' + debugValueToString(code.constants[constant]) + ']'
}

function debugByteInstruction(name, code, index) {
  const b = code.instructions[index + 1]
  return name + ': [' + b + ']'
}

function debugJumpInstruction(name, sign, code, index) {
  const jump = (code.instructions[index + 1] << 8) | code.instructions[index + 2]
  return name + ': [' + index + '] . [' + (index + 3 + sign * jump) + ']'
}

function debugJumpInstruction(name) {
  return name
}

function disassembleInstruction(code, index) {
  let debug = null
  if (index > 0 && code.rows[index] === code.rows[index - 1]) {
    debug = '   | '
  } else {
    debug = String(code.rows[index]).padStart(4, ' ') + ' '
  }
  const op = code.instructions[index]
  switch (op) {
    case OP_ADD:
      return debug + debugInstruction('OP_ADD', index)
    case OP_ARRAY_INSERT:
      return debug + debugInstruction('OP_ARRAY_INSERT', index)
    case OP_ARRAY_POP:
      return debug + debugInstruction('OP_ARRAY_POP', index)
    case OP_ARRAY_PUSH:
      return debug + debugInstruction('OP_ARRAY_PUSH', index)
    case OP_BIT_AND:
      return debug + debugInstruction('OP_BIT_AND', index)
    case OP_BIT_LEFT_SHIFT:
      return debug + debugInstruction('OP_BIT_LEFT_SHIFT', index)
    case OP_BIT_NOT:
      return debug + debugInstruction('OP_BIT_NOT', index)
    case OP_BIT_OR:
      return debug + debugInstruction('OP_BIT_OR', index)
    case OP_BIT_RIGHT_SHIFT:
      return debug + debugInstruction('OP_BIT_RIGHT_SHIFT', index)
    case OP_BIT_XOR:
      return debug + debugInstruction('OP_BIT_XOR', index)
    case OP_CALL:
      return debug + debugByteInstruction('OP_CALL', code, index)
    case OP_CLEAR:
      return debug + debugInstruction('OP_CLEAR', index)
    case OP_CONSTANT:
      return debug + debugConstantInstruction('OP_CONSTANT', code, index)
    case OP_COPY:
      return debug + debugInstruction('OP_COPY', index)
    case OP_DO:
      return debug + debugInstruction('OP_DO', index)
    case OP_DUPLICATE:
      return debug + debugInstruction('OP_DUPLICATE', index)
    case OP_DEFINE_GLOBAL:
      return debug + debugConstantInstruction('OP_DEFINE_GLOBAL', code, index)
    case OP_DELETE:
      return debug + debugInstruction('OP_DELETE', index)
    case OP_DIVIDE:
      return debug + debugInstruction('OP_DIVIDE', index)
    case OP_EQUAL:
      return debug + debugInstruction('OP_EQUAL', index)
    case OP_FALSE:
      return debug + debugInstruction('OP_FALSE', index)
    case OP_GET_DYNAMIC:
      return debug + debugInstruction('OP_GET_DYNAMIC', index)
    case OP_GET_GLOBAL:
      return debug + debugConstantInstruction('OP_GET_GLOBAL', code, index)
    case OP_GET_LOCAL:
      return debug + debugByteInstruction('OP_GET_LOCAL', code, index)
    case OP_GET_PROPERTY:
      return debug + debugConstantInstruction('OP_GET_PROPERTY', code, index)
    case OP_GREATER:
      return debug + debugInstruction('OP_GREATER', index)
    case OP_GREATER_EQUAL:
      return debug + debugInstruction('OP_GREATER_EQUAL', index)
    case OP_INDEX:
      return debug + debugInstruction('OP_INDEX', index)
    case OP_JUMP:
      return debug + debug_jump_instruction('OP_JUMP', 1, code, index)
    case OP_JUMP_IF_FALSE:
      return debug + debug_jump_instruction('OP_JUMP_IF_FALSE', 1, code, index)
    case OP_JUMP_IF_TRUE:
      return debug + debug_jump_instruction('OP_JUMP_IF_TRUE', 1, code, index)
    case OP_KEYS:
      return debug + debugInstruction('OP_KEYS', index)
    case OP_LEN:
      return debug + debugInstruction('OP_LEN', index)
    case OP_LESS:
      return debug + debugInstruction('OP_LESS', index)
    case OP_LESS_EQUAL:
      return debug + debugInstruction('OP_LESS_EQUAL', index)
    case OP_LOOP:
      return debug + debug_jump_instruction('OP_LOOP', -1, code, index)
    case OP_MODULO:
      return debug + debugInstruction('OP_MODULO', index)
    case OP_MULTIPLY:
      return debug + debugInstruction('OP_MULTIPLY', index)
    case OP_NEGATE:
      return debug + debugInstruction('OP_NEGATE', index)
    case OP_NONE:
      return debug + debugInstruction('OP_NONE', index)
    case OP_NOT:
      return debug + debugInstruction('OP_NOT', index)
    case OP_NOT_EQUAL:
      return debug + debugInstruction('OP_NOT_EQUAL', index)
    case OP_POP:
      return debug + debugInstruction('OP_POP', index)
    case OP_PRINT:
      return debug + debugInstruction('OP_PRINT', index)
    case OP_THROW:
      return debug + debugInstruction('OP_THROW', index)
    case OP_SET_DYNAMIC:
      return debug + debugInstruction('OP_SET_DYNAMIC', index)
    case OP_SET_GLOBAL:
      return debug + debugConstantInstruction('OP_SET_GLOBAL', code, index)
    case OP_SET_LOCAL:
      return debug + debugByteInstruction('OP_SET_LOCAL', code, index)
    case OP_SET_PROPERTY:
      return debug + debugConstantInstruction('OP_SET_PROPERTY', code, index)
    case OP_SLICE:
      return debug + debugInstruction('OP_SLICE', index)
    case OP_SUBTRACT:
      return debug + debugInstruction('OP_SUBTRACT', index)
    case OP_TO_FLOAT:
      return debug + debugInstruction('OP_TO_FLOAT', index)
    case OP_TO_INTEGER:
      return debug + debugInstruction('OP_TO_INTEGER', index)
    case OP_TO_STRING:
      return debug + debugInstruction('OP_TO_STRING', index)
    case OP_TRUE:
      return debug + debugInstruction('OP_TRUE', index)
    case OP_TYPE:
      return debug + debugInstruction('OP_TYPE', index)
    case OP_USE:
      return debug + debugInstruction('OP_USE', index)
    case OP_RETURN:
      return debug + debugInstruction('OP_RETURN', index)
    default:
      return debug + 'UNKNOWN OPCODE ' + op
  }
}

function debugStack(hymn) {
  if (hymn.stackTop === 0) {
    return
  }
  let debug = ''
  for (let i = 0; i < hymn.stackTop; i++) {
    debug += '[' + debugValueToString(hymn.stack[i]) + '] '
  }
  return debug
}

function hymnRun(hymn) {
  let frame = currentFrame(hymn)
  while (true) {
    if (HYMN_DEBUG_TRACE) console.debug(disassembleInstruction(frame.func.code, frame.ip))
    if (HYMN_DEBUG_STACK) console.debug(debugStack(hymn))
    const op = readByte(frame)
    switch (op) {
      case OP_RETURN: {
        const result = hymnPop(hymn)
        hymn.frameCount--
        if (hymn.frameCount === 0 || frame.func.name === null) {
          hymnPop(hymn)
          return
        }
        while (hymn.stackTop !== frame.stack) {
          --hymn.stackTop
        }
        hymnPush(hymn, result)
        frame = currentFrame(hymn)
      }
      case OP_POP:
        hymnPop(hymn)
      case OP_TRUE:
        hymnPush(newBool(true))
      case OP_FALSE:
        hymnPush(newBool(false))
      case OP_NONE:
        hymnPush(newNone())
      case OP_CALL: {
        const count = readByte(frame)
        const call = hymnPeek(hymn, count + 1)
        const response = hymnCallValue(hymn, call, count)
        if (response == MACHINE_FATAL) return
        if (response == MACHINE_OK) frame = currentFrame(hymn)
        break
      }
      case OP_JUMP: {
        const jump = readShort(frame)
        frame.ip += jump
        break
      }
      case OP_JUMP_IF_FALSE: {
        const jump = readShort(frame)
        if (hymnFalse(hymnPeek(hymn, 1))) {
          frame.ip += jump
        }
        break
      }
      case OP_JUMP_IF_TRUE: {
        const jump = readShort(frame)
        if (!hymnFalse(hymnPeek(hymn, 1))) {
          frame.ip += jump
        }
        break
      }
      case OP_LOOP: {
        const jump = readShort(frame)
        frame.ip -= jump
        break
      }
      case OP_EQUAL: {
        const b = hymnPop(hymn)
        const a = hymnPop(hymn)
        hymnPush(hymn, newBool(machineEqual(a, b)))
        break
      }
      case OP_NOT_EQUAL: {
        const b = hymnPop(hymn)
        const a = hymnPop(hymn)
        hymnPush(hymn, newBool(!machineEqual(a, b)))
        break
      }
      case OP_LESS: {
        const b = hymnPop(hymn)
        const a = hymnPop(hymn)
        if (isInt(a)) {
          if (isInt(b)) {
            hymnPush(hymn, newBool(a.value < b.value))
          } else if (isFloat(b)) {
            hymnPush(hymn, newBool(a.value < b.value))
          } else {
            frame = hymnThrowError(hymn, 'Operands must be numbers.')
            if (frame === null) return
          }
        } else if (isFloat(a)) {
          if (isInt(b)) {
            hymnPush(hymn, newBool(a.value < b.value))
          } else if (isFloat(b)) {
            hymnPush(hymn, newBool(a.value < b.value))
          } else {
            frame = hymnThrowError(hymn, 'Operands must be numbers.')
            if (frame === null) return
          }
        } else {
          if (hymnThrowError(hymn, 'Operands must be numbers.') == MACHINE_FATAL) {
            return
          } else {
            frame = currentFrame(hymn)
            break
          }
        }
        break
      }
      default:
        console.error('Unknown instruction:', op)
        return
    }
  }
}

function hymnAddFunction(hymn, name, func) {
  const value = newNativeFunction(name, func)
  hymn.globals.put(name, newFuncNativeValue(value))
}

function hymnAddPointer(hymn, name, pointer) {
  hymn.globals.put(name, newPointerValue(pointer))
}

function hymnInterpret(hymn, source) {
  const result = compile(hymn, null, source)

  const func = result.func
  if (result.error !== null) {
    return result.error
  }

  const funcVal = newFuncValue(func)
  hymnPush(hymn, funcVal)
  hymnCall(hymn, func, 0)

  hymnRun(hymn)
  if (hymn.error !== null) {
    return hymn.error
  }

  hymnResetStack(hymn)
  return null
}

function newHymn() {
  return new Hymn()
}
