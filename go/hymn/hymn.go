package hymn

type ValueType int

const (
	HYMN_VALUE_NONE        ValueType = 1
	HYMN_VALUE_BOOL                  = 2
	HYMN_VALUE_INTEGER               = 3
	HYMN_VALUE_FLOAT                 = 4
	HYMN_VALUE_STRING                = 5
	HYMN_VALUE_ARRAY                 = 6
	HYMN_VALUE_TABLE                 = 7
	HYMN_VALUE_FUNC                  = 8
	HYMN_VALUE_FUNC_NATIVE           = 9
	HYMN_VALUE_POINTER               = 10
)

type Value struct {
	is    ValueType
	value interface{}
}

type NativeFunction struct {
	name string
	fun  int
}

type ByteCode struct {
	count        int
	instructions []uint8
	lines        []uint
	constants    []int
}

type ExceptList struct {
	start int
	end   int
	stack int
	next  int
}

type Function struct {
	count  int
	name   string
	script string
	arity  int
	code   int
	except int
}

type Frame struct {
	fun   int
	ip    int
	stack int
}

type Hymn struct {
	stack      []uint8
	stackTop   int
	frames     []int
	frameCount int
	globals    int
	paths      []string
	err        string
	imports    []string
	print      func(string)
	printErr   func(string)
}

type TokenType int

const (
	TOKEN_ADD                    TokenType = 0
	TOKEN_AND                              = 1
	TOKEN_ASSIGN                           = 2
	TOKEN_ASSIGN_ADD                       = 3
	TOKEN_ASSIGN_BIT_AND                   = 4
	TOKEN_ASSIGN_BIT_LEFT_SHIFT            = 5
	TOKEN_ASSIGN_BIT_OR                    = 6
	TOKEN_ASSIGN_BIT_RIGHT_SHIFT           = 7
	TOKEN_ASSIGN_BIT_XOR                   = 8
	TOKEN_ASSIGN_DIVIDE                    = 9
	TOKEN_ASSIGN_MODULO                    = 10
	TOKEN_ASSIGN_MULTIPLY                  = 11
	TOKEN_ASSIGN_SUBTRACT                  = 12
	TOKEN_BIT_AND                          = 13
	TOKEN_BIT_LEFT_SHIFT                   = 14
	TOKEN_BIT_NOT                          = 15
	TOKEN_BIT_OR                           = 16
	TOKEN_BIT_RIGHT_SHIFT                  = 17
	TOKEN_BIT_XOR                          = 18
	TOKEN_BREAK                            = 19
	TOKEN_CLEAR                            = 20
	TOKEN_COLON                            = 21
	TOKEN_COMMA                            = 22
	TOKEN_CONTINUE                         = 23
	TOKEN_COPY                             = 24
	TOKEN_DELETE                           = 25
	TOKEN_DIVIDE                           = 26
	TOKEN_DOT                              = 27
	TOKEN_ECHO                             = 28
	TOKEN_ELIF                             = 29
	TOKEN_ELSE                             = 30
	TOKEN_EOF                              = 31
	TOKEN_EQUAL                            = 32
	TOKEN_ERROR                            = 33
	TOKEN_EXCEPT                           = 34
	TOKEN_EXISTS                           = 35
	TOKEN_FALSE                            = 36
	TOKEN_FLOAT                            = 37
	TOKEN_FOR                              = 38
	TOKEN_FUNCTION                         = 39
	TOKEN_GREATER                          = 40
	TOKEN_GREATER_EQUAL                    = 41
	TOKEN_IDENT                            = 42
	TOKEN_IF                               = 43
	TOKEN_IN                               = 44
	TOKEN_INDEX                            = 45
	TOKEN_INSERT                           = 46
	TOKEN_INTEGER                          = 47
	TOKEN_KEYS                             = 48
	TOKEN_LEFT_CURLY                       = 49
	TOKEN_LEFT_PAREN                       = 50
	TOKEN_LEFT_SQUARE                      = 51
	TOKEN_LEN                              = 52
	TOKEN_LESS                             = 53
	TOKEN_LESS_EQUAL                       = 54
	TOKEN_LET                              = 55
	TOKEN_MODULO                           = 57
	TOKEN_MULTIPLY                         = 58
	TOKEN_NONE                             = 59
	TOKEN_NOT                              = 60
	TOKEN_NOT_EQUAL                        = 61
	TOKEN_OR                               = 62
	TOKEN_POP                              = 63
	TOKEN_PRINT                            = 64
	TOKEN_PUSH                             = 65
	TOKEN_RETURN                           = 66
	TOKEN_RIGHT_CURLY                      = 67
	TOKEN_RIGHT_PAREN                      = 68
	TOKEN_RIGHT_SQUARE                     = 69
	TOKEN_SEMICOLON                        = 70
	TOKEN_STRING                           = 71
	TOKEN_SUBTRACT                         = 72
	TOKEN_THROW                            = 73
	TOKEN_TO_FLOAT                         = 74
	TOKEN_TO_INTEGER                       = 75
	TOKEN_TO_STRING                        = 76
	TOKEN_TRUE                             = 77
	TOKEN_TRY                              = 78
	TOKEN_TYPE_FUNC                        = 79
	TOKEN_UNDEFINED                        = 80
	TOKEN_USE                              = 81
	TOKEN_VALUE                            = 82
	TOKEN_WHILE                            = 83
)

type Precedence int

const (
	PRECEDENCE_NONE     Precedence = 0
	PRECEDENCE_ASSIGN              = 1
	PRECEDENCE_BITS                = 2
	PRECEDENCE_OR                  = 3
	PRECEDENCE_AND                 = 4
	PRECEDENCE_EQUALITY            = 5
	PRECEDENCE_COMPARE             = 6
	PRECEDENCE_TERM                = 7
	PRECEDENCE_FACTOR              = 8
	PRECEDENCE_UNARY               = 9
	PRECEDENCE_CALL                = 10
)

type StringStatus int

const (
	STRING_STATUS_NONE     StringStatus = 0
	STRING_STATUS_BEGIN                 = 1
	STRING_STATUS_ADD                   = 2
	STRING_STATUS_CLOSE                 = 3
	STRING_STATUS_CONTINUE              = 4
)

type Operation int

const (
	OP_ADD                     Operation = 0
	OP_ARRAY_INSERT                      = 2
	OP_ARRAY_POP                         = 3
	OP_ARRAY_PUSH                        = 4
	OP_BIT_AND                           = 5
	OP_BIT_LEFT_SHIFT                    = 6
	OP_BIT_NOT                           = 7
	OP_BIT_OR                            = 8
	OP_BIT_RIGHT_SHIFT                   = 9
	OP_BIT_XOR                           = 10
	OP_CALL                              = 11
	OP_CLEAR                             = 12
	OP_CONSTANT                          = 13
	OP_COPY                              = 14
	OP_DEFINE_GLOBAL                     = 15
	OP_DELETE                            = 16
	OP_DIVIDE                            = 17
	OP_DUPLICATE                         = 18
	OP_ECHO                              = 19
	OP_EQUAL                             = 20
	OP_EXISTS                            = 21
	OP_FALSE                             = 22
	OP_GET_DYNAMIC                       = 23
	OP_GET_GLOBAL                        = 24
	OP_GET_LOCAL                         = 25
	OP_GET_PROPERTY                      = 26
	OP_GREATER                           = 27
	OP_GREATER_EQUAL                     = 28
	OP_INDEX                             = 29
	OP_JUMP                              = 30
	OP_JUMP_IF_FALSE                     = 37
	OP_JUMP_IF_TRUE                      = 38
	OP_KEYS                              = 39
	OP_LEN                               = 40
	OP_LESS                              = 41
	OP_LESS_EQUAL                        = 42
	OP_LOOP                              = 43
	OP_MODULO                            = 44
	OP_MULTIPLY                          = 45
	OP_NEGATE                            = 46
	OP_NONE                              = 47
	OP_NOT                               = 48
	OP_NOT_EQUAL                         = 49
	OP_POP                               = 50
	OP_PRINT                             = 51
	OP_RETURN                            = 52
	OP_SET_DYNAMIC                       = 53
	OP_SET_GLOBAL                        = 54
	OP_SET_LOCAL                         = 55
	OP_INCREMENT_LOCAL_AND_SET           = 56
	OP_SET_PROPERTY                      = 57
	OP_SLICE                             = 58
	OP_SUBTRACT                          = 59
	OP_THROW                             = 60
	OP_FLOAT                             = 61
	OP_INT                               = 62
	OP_STRING                            = 63
	OP_TRUE                              = 64
	OP_TYPE                              = 65
	OP_USE                               = 66
	OP_FOR                               = 67
	OP_FOR_LOOP                          = 68
	OP_NEW_ARRAY                         = 69
	OP_NEW_TABLE                         = 70
)

type FunctionType int

const (
	TYPE_FUNCTION FunctionType = 0
	TYPE_SCRIPT                = 1
)

type Token struct {
	is     TokenType
	row    int
	column int
	start  int
	length int
}

type Local struct {
	is     TokenType
	row    int
	column int
	start  int
	length int
}

type Rule struct {
	prefix func(*Compiler, bool)
	infix  func(*Compiler, bool)
	prec   Precedence
}

type Scope struct {
	enclosing  int
	fun        int
	kind       FunctionType
	locals     []Local
	localCount int
	depth      int
}

type Compiler struct {
	pos       int
	row       int
	column    int
	script    int
	source    []rune
	size      int
	previous  Token
	current   Token
	strFormat int
	strStatus StringStatus
	hymn      Hymn
}

type JumpList struct {
	jump  int
	depth int
	next  int
}

type LoopList struct {
	start int
	depth int
	next  int
	isFor bool
}

var rules = map[TokenType]Rule{
	TOKEN_ADD:                    {nil, compileBinary, PRECEDENCE_TERM},
	TOKEN_AND:                    {nil, compileAnd, PRECEDENCE_AND},
	TOKEN_ASSIGN:                 {nil, nil, PRECEDENCE_NONE},
	TOKEN_ASSIGN_ADD:             {nil, nil, PRECEDENCE_NONE},
	TOKEN_ASSIGN_BIT_AND:         {nil, nil, PRECEDENCE_NONE},
	TOKEN_ASSIGN_BIT_LEFT_SHIFT:  {nil, nil, PRECEDENCE_NONE},
	TOKEN_ASSIGN_BIT_OR:          {nil, nil, PRECEDENCE_NONE},
	TOKEN_ASSIGN_BIT_RIGHT_SHIFT: {nil, nil, PRECEDENCE_NONE},
	TOKEN_ASSIGN_BIT_XOR:         {nil, nil, PRECEDENCE_NONE},
	TOKEN_ASSIGN_DIVIDE:          {nil, nil, PRECEDENCE_NONE},
	TOKEN_ASSIGN_MODULO:          {nil, nil, PRECEDENCE_NONE},
	TOKEN_ASSIGN_MULTIPLY:        {nil, nil, PRECEDENCE_NONE},
	TOKEN_ASSIGN_SUBTRACT:        {nil, nil, PRECEDENCE_NONE},
	TOKEN_BIT_AND:                {nil, compileBinary, PRECEDENCE_BITS},
	TOKEN_BIT_LEFT_SHIFT:         {nil, compileBinary, PRECEDENCE_BITS},
	TOKEN_BIT_NOT:                {compileUnary, nil, PRECEDENCE_NONE},
	TOKEN_BIT_OR:                 {nil, compileBinary, PRECEDENCE_BITS},
	TOKEN_BIT_RIGHT_SHIFT:        {nil, compileBinary, PRECEDENCE_BITS},
	TOKEN_BIT_XOR:                {nil, compileBinary, PRECEDENCE_BITS},
	TOKEN_BREAK:                  {nil, nil, PRECEDENCE_NONE},
	TOKEN_COLON:                  {nil, nil, PRECEDENCE_NONE},
	TOKEN_CLEAR:                  {clearExpression, nil, PRECEDENCE_NONE},
	TOKEN_COMMA:                  {nil, nil, PRECEDENCE_NONE},
	TOKEN_CONTINUE:               {nil, nil, PRECEDENCE_NONE},
	TOKEN_COPY:                   {copyExpression, nil, PRECEDENCE_NONE},
	TOKEN_DELETE:                 {deleteExpression, nil, PRECEDENCE_NONE},
	TOKEN_DIVIDE:                 {nil, compileBinary, PRECEDENCE_FACTOR},
	TOKEN_DOT:                    {nil, compileDot, PRECEDENCE_CALL},
	TOKEN_ECHO:                   {nil, nil, PRECEDENCE_NONE},
	TOKEN_ELIF:                   {nil, nil, PRECEDENCE_NONE},
	TOKEN_ELSE:                   {nil, nil, PRECEDENCE_NONE},
	TOKEN_EOF:                    {nil, nil, PRECEDENCE_NONE},
	TOKEN_EQUAL:                  {nil, compileBinary, PRECEDENCE_EQUALITY},
	TOKEN_ERROR:                  {nil, nil, PRECEDENCE_NONE},
	TOKEN_EXCEPT:                 {nil, nil, PRECEDENCE_NONE},
	TOKEN_EXISTS:                 {existsExpression, nil, PRECEDENCE_NONE},
	TOKEN_FALSE:                  {compileFalse, nil, PRECEDENCE_NONE},
	TOKEN_FLOAT:                  {compileFloat, nil, PRECEDENCE_NONE},
	TOKEN_FOR:                    {nil, nil, PRECEDENCE_NONE},
	TOKEN_FUNCTION:               {functionExpression, nil, PRECEDENCE_NONE},
	TOKEN_GREATER:                {nil, compileBinary, PRECEDENCE_COMPARE},
	TOKEN_GREATER_EQUAL:          {nil, compileBinary, PRECEDENCE_COMPARE},
	TOKEN_IDENT:                  {compileVariable, nil, PRECEDENCE_NONE},
	TOKEN_IF:                     {nil, nil, PRECEDENCE_NONE},
	TOKEN_IN:                     {nil, nil, PRECEDENCE_NONE},
	TOKEN_INDEX:                  {indexExpression, nil, PRECEDENCE_NONE},
	TOKEN_INSERT:                 {arrayInsertExpression, nil, PRECEDENCE_NONE},
	TOKEN_INTEGER:                {compileInteger, nil, PRECEDENCE_NONE},
	TOKEN_KEYS:                   {keysExpression, nil, PRECEDENCE_NONE},
	TOKEN_LEFT_CURLY:             {compileTable, nil, PRECEDENCE_NONE},
	TOKEN_LEFT_PAREN:             {compileGroup, compileCall, PRECEDENCE_CALL},
	TOKEN_LEFT_SQUARE:            {compileArray, compileSquare, PRECEDENCE_CALL},
	TOKEN_LEN:                    {lenExpression, nil, PRECEDENCE_NONE},
	TOKEN_LESS:                   {nil, compileBinary, PRECEDENCE_COMPARE},
	TOKEN_LESS_EQUAL:             {nil, compileBinary, PRECEDENCE_COMPARE},
	TOKEN_LET:                    {nil, nil, PRECEDENCE_NONE},
	TOKEN_MODULO:                 {nil, compileBinary, PRECEDENCE_FACTOR},
	TOKEN_MULTIPLY:               {nil, compileBinary, PRECEDENCE_FACTOR},
	TOKEN_NONE:                   {compileNone, nil, PRECEDENCE_NONE},
	TOKEN_NOT:                    {compileUnary, nil, PRECEDENCE_NONE},
	TOKEN_NOT_EQUAL:              {nil, compileBinary, PRECEDENCE_EQUALITY},
	TOKEN_OR:                     {nil, compileOr, PRECEDENCE_OR},
	TOKEN_POP:                    {arrayPopExpression, nil, PRECEDENCE_NONE},
	TOKEN_PRINT:                  {nil, nil, PRECEDENCE_NONE},
	TOKEN_PUSH:                   {arrayPushExpression, nil, PRECEDENCE_NONE},
	TOKEN_RETURN:                 {nil, nil, PRECEDENCE_NONE},
	TOKEN_RIGHT_CURLY:            {nil, nil, PRECEDENCE_NONE},
	TOKEN_RIGHT_PAREN:            {nil, nil, PRECEDENCE_NONE},
	TOKEN_RIGHT_SQUARE:           {nil, nil, PRECEDENCE_NONE},
	TOKEN_STRING:                 {compileString, nil, PRECEDENCE_NONE},
	TOKEN_SUBTRACT:               {compileUnary, compileBinary, PRECEDENCE_TERM},
	TOKEN_TO_FLOAT:               {castFloatExpression, nil, PRECEDENCE_NONE},
	TOKEN_TO_INTEGER:             {castIntegerExpression, nil, PRECEDENCE_NONE},
	TOKEN_TO_STRING:              {castStringExpression, nil, PRECEDENCE_NONE},
	TOKEN_TRUE:                   {compileTrue, nil, PRECEDENCE_NONE},
	TOKEN_TRY:                    {nil, nil, PRECEDENCE_NONE},
	TOKEN_THROW:                  {nil, nil, PRECEDENCE_NONE},
	TOKEN_TYPE_FUNC:              {typeExpression, nil, PRECEDENCE_NONE},
	TOKEN_UNDEFINED:              {nil, nil, PRECEDENCE_NONE},
	TOKEN_USE:                    {nil, nil, PRECEDENCE_NONE},
	TOKEN_VALUE:                  {nil, nil, PRECEDENCE_NONE},
	TOKEN_WHILE:                  {nil, nil, PRECEDENCE_NONE},
	TOKEN_SEMICOLON:              {nil, nil, PRECEDENCE_NONE},
}

func valueType(t ValueType) string {
	switch t {
	case HYMN_VALUE_NONE:
		return "none"
	default:
		return "?"
	}
}

func compileAnd(c *Compiler, b bool) {
}

func clearExpression(c *Compiler, b bool) {
}

func copyExpression(c *Compiler, b bool) {
}

func deleteExpression(c *Compiler, b bool) {
}

func compileDot(c *Compiler, b bool) {
}

func existsExpression(c *Compiler, b bool) {
}

func compileFalse(c *Compiler, b bool) {
}

func compileFloat(c *Compiler, b bool) {
}

func functionExpression(c *Compiler, b bool) {
}

func compileVariable(c *Compiler, b bool) {
}

func indexExpression(c *Compiler, b bool) {
}

func arrayInsertExpression(c *Compiler, b bool) {
}

func compileInteger(c *Compiler, b bool) {
}

func keysExpression(c *Compiler, b bool) {
}

func compileSquare(c *Compiler, b bool) {
}

func compileCall(c *Compiler, b bool) {
}

func compileBinary(c *Compiler, b bool) {
}

func arrayPushExpression(c *Compiler, b bool) {
}

func arrayPopExpression(c *Compiler, b bool) {
}

func compileOr(c *Compiler, b bool) {
}

func compileNone(c *Compiler, b bool) {
}

func lenExpression(c *Compiler, b bool) {
}

func compileArray(c *Compiler, b bool) {
}

func compileTable(c *Compiler, b bool) {
}

func compileGroup(c *Compiler, b bool) {
}

func compileUnary(c *Compiler, b bool) {
}

func typeExpression(c *Compiler, b bool) {
}

func compileTrue(c *Compiler, b bool) {
}

func compileString(c *Compiler, b bool) {
}

func castFloatExpression(c *Compiler, b bool) {
}

func castStringExpression(c *Compiler, b bool) {
}

func castIntegerExpression(c *Compiler, b bool) {
}

func newNone() Value {
	return Value{HYMN_VALUE_NONE, 0}
}

func newBool(b bool) Value {
	return Value{HYMN_VALUE_NONE, b}
}

func nextChar(c *Compiler) rune {
	pos := c.pos
	if pos == c.size {
		return rune(0)
	}
	r := c.source[pos]
	c.pos = pos + 1
	if r == '\n' {
		c.row++
		c.column = 0
	} else {
		c.column++
	}
	return r
}

func peekChar(c *Compiler) rune {
	if c.pos == c.size {
		return rune(0)
	}
	return c.source[c.pos]
}

func token(c *Compiler, t TokenType) {
	a := &c.current
	a.is = t
	a.row = c.row
	a.column = c.column
	if c.pos == 0 {
		a.start = 0
	} else {
		a.start = c.pos - 1
	}
	a.length = 1
}

func tokenSpecial(c *Compiler, t TokenType, offset, length int) {
	a := &c.current
	a.is = t
	a.row = c.row
	a.column = c.column
	if c.pos < offset {
		a.start = 0
	} else {
		a.start = c.pos - offset
	}
	a.length = length
}

func valueToken(c *Compiler, t TokenType, start, end int) {
	a := &c.current
	a.is = t
	a.row = c.row
	a.column = c.column
	a.start = start
	a.length = end - start
}

func NewVM() *Hymn {
	vm := new(Hymn)
	return vm
}

func Interpret(vm *Hymn, source string) error {
	return nil
}

func InterpretScript(vm *Hymn, script, source string) error {
	return nil
}

func DebugInterpret(vm *Hymn, source string) error {
	return nil
}

func DebugInterpretScript(vm *Hymn, script, source string) error {
	return nil
}
