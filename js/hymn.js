const HYMN_UINT8_COUNT = 256
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
  constructor() {
    this.is = HYMN_VALUE_UNDEFINED
    this.value = null
  }
}

class HymnNativeFunction {
  constructor(name, call) {
    this.name = name
    this.call = call
  }
}

class HymnByteCode {
  constructor() {
    this.instructions = new Uint8Array()
    this.rows = new Uint16Array()
    this.constants = null
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
    this.strings = null
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
    this.type = TOKEN_EOF
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

const rules = {
  TOKEN_ADD: new Rule(null, compileBinary, PRECEDENCE_TERM),
  TOKEN_AND: new Rule(null, compileAnd, PRECEDENCE_AND),
  TOKEN_ASSIGN: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_BEGIN: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_BIT_AND: new Rule(null, compileBinary, PRECEDENCE_BITS),
  TOKEN_BIT_LEFT_SHIFT: new Rule(null, compileBinary, PRECEDENCE_BITS),
  TOKEN_BIT_NOT: new Rule(compileUnary, null, PRECEDENCE_NONE),
  TOKEN_BIT_OR: new Rule(null, compileBinary, PRECEDENCE_BITS),
  TOKEN_BIT_RIGHT_SHIFT: new Rule(null, compileBinary, PRECEDENCE_BITS),
  TOKEN_BIT_XOR: new Rule(null, compileBinary, PRECEDENCE_BITS),
  TOKEN_BREAK: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_CASE: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_CLEAR: new Rule(clearExpression, null, PRECEDENCE_NONE),
  TOKEN_COMMA: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_CONST: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_CONTINUE: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_COPY: new Rule(copyExpression, null, PRECEDENCE_NONE),
  TOKEN_DO: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_DELETE: new Rule(deleteExpression, null, PRECEDENCE_NONE),
  TOKEN_DIVIDE: new Rule(null, compileBinary, PRECEDENCE_FACTOR),
  TOKEN_DOT: new Rule(null, compileDot, PRECEDENCE_CALL),
  TOKEN_ELIF: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_ELSE: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_END: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_EOF: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_EQUAL: new Rule(null, compileBinary, PRECEDENCE_EQUALITY),
  TOKEN_ERROR: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_EXCEPT: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_FALSE: new Rule(compileFalse, null, PRECEDENCE_NONE),
  TOKEN_FLOAT: new Rule(compileFloat, null, PRECEDENCE_NONE),
  TOKEN_FOR: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_FUNCTION: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_GREATER: new Rule(null, compileBinary, PRECEDENCE_COMPARE),
  TOKEN_GREATER_EQUAL: new Rule(null, compileBinary, PRECEDENCE_COMPARE),
  TOKEN_IDENT: new Rule(compileVariable, null, PRECEDENCE_NONE),
  TOKEN_IF: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_IN: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_INDEX: new Rule(indexExpression, null, PRECEDENCE_NONE),
  TOKEN_INSERT: new Rule(arrayInsertExpression, null, PRECEDENCE_NONE),
  TOKEN_INTEGER: new Rule(compileInteger, null, PRECEDENCE_NONE),
  TOKEN_ITERATE: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_KEYS: new Rule(keysExpression, null, PRECEDENCE_NONE),
  TOKEN_LEFT_CURLY: new Rule(compileTable, null, PRECEDENCE_NONE),
  TOKEN_LEFT_PAREN: new Rule(compileGroup, compileCall, PRECEDENCE_CALL),
  TOKEN_LEFT_SQUARE: new Rule(compileArray, compileSquare, PRECEDENCE_CALL),
  TOKEN_LEN: new Rule(lenExpression, null, PRECEDENCE_NONE),
  TOKEN_LESS: new Rule(null, compileBinary, PRECEDENCE_COMPARE),
  TOKEN_LESS_EQUAL: new Rule(null, compileBinary, PRECEDENCE_COMPARE),
  TOKEN_LET: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_LINE: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_MODULO: new Rule(null, compileBinary, PRECEDENCE_FACTOR),
  TOKEN_MULTIPLY: new Rule(null, compileBinary, PRECEDENCE_FACTOR),
  TOKEN_NONE: new Rule(compileNone, null, PRECEDENCE_NONE),
  TOKEN_NOT: new Rule(compileUnary, null, PRECEDENCE_NONE),
  TOKEN_NOT_EQUAL: new Rule(null, compileBinary, PRECEDENCE_EQUALITY),
  TOKEN_OR: new Rule(null, compileOr, PRECEDENCE_OR),
  TOKEN_PASS: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_POP: new Rule(arrayPopExpression, null, PRECEDENCE_NONE),
  TOKEN_PRINT: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_PUSH: new Rule(arrayPushExpression, null, PRECEDENCE_NONE),
  TOKEN_RETURN: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_RIGHT_CURLY: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_RIGHT_PAREN: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_RIGHT_SQUARE: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_STRING: new Rule(compileString, null, PRECEDENCE_NONE),
  TOKEN_SUBTRACT: new Rule(compileUnary, compileBinary, PRECEDENCE_TERM),
  TOKEN_SWITCH: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_TO_FLOAT: new Rule(castFloatExpression, null, PRECEDENCE_NONE),
  TOKEN_TO_INTEGER: new Rule(castIntegerExpression, null, PRECEDENCE_NONE),
  TOKEN_TO_STRING: new Rule(castStringExpression, null, PRECEDENCE_NONE),
  TOKEN_TRUE: new Rule(compileTrue, null, PRECEDENCE_NONE),
  TOKEN_TRY: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_THROW: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_TYPE: new Rule(typeExpression, null, PRECEDENCE_NONE),
  TOKEN_UNDEFINED: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_USE: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_VALUE: new Rule(null, null, PRECEDENCE_NONE),
  TOKEN_WHILE: new Rule(null, null, PRECEDENCE_NONE),
}

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

function token(compiler, type) {
  let token = compiler.token
  token.type = type
  token.row = compiler.row
  token.column = compiler.column
  token.start = compiler.pos - 1
  token.len = 1
}

function tokenSpecial(compiler, type, offset, len) {
  let token = compiler.token
  token.type = type
  token.row = compiler.row
  token.column = compiler.column
  token.start = compiler.pos - offset
  token.len = len
}

function valueToken(compiler, type, start, end) {
  let token = compiler.token
  token.type = type
  token.row = compiler.row
  token.column = compiler.column
  token.start = start
  token.len = end - start
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
  if (compiler.previous === TOKEN_EOF) {
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

  const local = scope.locals[scope.localCount++]
  local.depth = 0
  local.name.start = 0
  local.name.length = 0
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
  code.instructions[count] = byte
  code.rows[count] = row
  code.count = count + 1
}

function writeTwoOp(code, byteOne, byteTwo, row) {
  const count = code.count
  code.instructions[count] = byteOne
  code.instructions[count + 1] = byteTwo
  code.rows[count] = row
  code.rows[count + 1] = row
  code.count = count + 2
}

function writeConstant(compiler, value, row) {
  let constant = byteCodeAddConstant(current(compiler), value)
  if (constant > 255) {
    compileError(compiler, compiler.previous, 'Too many constants.')
    constant = 0
  }
  writeTwoOp(current(this), OP_CONSTANT, constant, row)
  return constant
}

function hymnSetGlobal(hymn, name, value) {
  hymn.globals.set(name, value)
}

function check(compiler, type) {
  return compiler.current.type === type
}

function match(compiler, type) {
  if (!check(compiler.type)) {
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

function sourceSubstring(compiler, len, start) {
  return compiler.source.substring(start, start + len)
}

function compileWithPrecedence(compiler, assign) {
  advance(compiler)
  const rule = rules[compiler.previous.type]
  const prefix = rule.prefix
  if (prefix === null) {
    compileError(compiler, compiler.previous, 'Syntax Error: Expected expression following `' + sourceSubstring(compiler, compiler.previous.len, compiler.previous.start) + '`.')
    return
  }
}

function consume(compiler, type, error) {}

function pushHiddenLocal(compiler) {}

function args(compiler) {}

function compileCall(compiler, assign) {}

function compileGroup(compiler, assign) {}

function compileNone(compiler, assign) {}

function compileTrue(compiler, assign) {}

function compileFalse(compiler, assign) {}

function compileInteger(compiler, assign) {}

function compileFloat(compiler, assign) {}

function compileString(compiler, assign) {}

function identConstant(compiler, token) {}

function beginScope(compiler) {}

function endScope(compiler) {}

function compileArray(compiler, assign) {}

function compileTable(compiler, assign) {}

function pushLocal(compiler, name, isConstant) {}

function variable(compiler, isConstant, error) {}

function localInitialize(compiler) {}

function finalizeVariable(compiler, global) {}

function defineNewVariable(compiler, isConstant) {}

function resolveLocal(compiler, name, isConstant) {}

function namedVariable(compiler, token, assign) {}

function compileVariable(compiler, assign) {}

function compileUnary(compiler, assign) {}

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

function compileDot(compiler, assign) {}

function compileSquare(compiler, assign) {}

function emitJump(compiler, instruction) {}

function patchJump(compiler, jump) {}

function compileAnd(compiler, assign) {}

function compileOr(compiler, assign) {}

function endFunction(compiler) {}

function compileFunction(compiler, type) {}

function declareFunction(compiler) {}

function declaration(compiler) {
  advance(compiler)
  // if (match(compiler, TOKEN_LET)) {
  //   defineNewVariable(compiler, false)
  // } else if (match(compiler, TOKEN_CONST)) {
  //   defineNewVariable(compiler, true)
  // } else if (match(compiler, TOKEN_FUNCTION)) {
  //   declareFunction(compiler)
  // } else {
  //   statement(compiler)
  // }
}

function block(compiler) {}

function ifStatement(compiler) {}

function compileLiteral(compiler) {}

function switchStatement(compiler) {}

function emitLoop(compiler) {}

function patchJumpList(compiler) {}

function patchIteratorJumpList(compiler) {}

function iterateStatement(compiler) {}

function forStatement(compiler) {}

function whileStatement(compiler) {}

function returnStatement(compiler) {}

function popStackLoop(compiler) {}

function breakStatement(compiler) {}

function continueStatement(compiler) {}

function tryStatement(compiler) {}

function printStatement(compiler) {}

function useStatement(compiler) {}

function doStatement(compiler) {}

function throwStatement(compiler) {}

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

function arrayPushExpression(compiler, assign) {}

function arrayInsertExpression(compiler, assign) {}

function arrayPopExpression(compiler, assign) {}

function deleteExpression(compiler, assign) {}

function lenExpression(compiler, assign) {}

function castIntegerExpression(compiler, assign) {}

function castFloatExpression(compiler, assign) {}

function castStringExpression(compiler, assign) {}

function typeExpression(compiler, assign) {}

function clearExpression(compiler, assign) {}

function copyExpression(compiler, assign) {}

function keysExpression(compiler, assign) {}

function indexExpression(compiler, assign) {}

function expressionStatement(compiler) {}

function expression(compiler) {}

function parentFrame(hymn, offset) {}

function currentFrame(hymn) {}

function compile(hymn, script, source) {
  const scope = new Scope()
  const compiler = newCompiler(script, source, hymn, scope)
  advance(compiler)
  while (!match(compiler, TOKEN_EOR)) {
    declaration(compiler)
  }
  const func = endFunction(compiler)
  return { func: func, error: compiler.error }
}

function hymnPush(hymn, value) {}

function hymnCall(hymn, func, arity) {}

function hymnInterpretInternal(hymn) {}

function hymnResetStack(hymn) {}

function newHymn() {
  return new Hymn()
}

function hymnInterpret(hymn, source) {
  const result = compile(hymn, null, source)
  const func = result.func
  let error = result.error
  if (error) {
    functionDelete(func)
    return error
  }

  const funcVal = newFuncValue(func)
  hymnPush(hymn, funcVal)
  hymnCall(hymn, func, 0)

  error = hymnInterpetInternal(hymn)
  if (error) {
    return error
  }

  hymnResetStack(hymn)
  return null
}
