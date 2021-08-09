use std::{
  collections::HashMap,
  collections::HashSet,
  fs,
  io::{self, Write},
  option::Option,
  process,
  rc::Rc,
  result::Result,
};

#[derive(Copy, Clone)]
enum TokenType {
  End,
  Add,
  Subtract,
}

enum Precedence {
  None,
  Assign,
  Bits,
  Or,
  And,
  Equality,
  Compare,
  Term,
  Factor,
  Unary,
  Call,
  Primary,
}

enum OpCode {
  Add,
  Insert,
  Pop,
  Push,
}

#[derive(Copy, Clone)]
struct Token {
  is: TokenType,
  row: i32,
  column: i32,
  start: usize,
  length: i32,
}

pub enum Value {
  Nil,
  Bool(bool),
  Integer(i64),
  Float(f64),
  DynString(Rc<str>),
}

pub struct Hymn {
  globals: HashMap<String, Value>,
  strings: HashSet<String>,
}

struct Compiler {
  pos: usize,
  row: i32,
  column: i32,
  script: String,
  source: String,
  previous: Token,
  current: Token,
  error: Option<Rc<str>>,
}

impl Token {
  pub fn new() -> Token {
    Token {
      is: TokenType::End,
      row: 0,
      column: 0,
      start: 0,
      length: 0,
    }
  }
}

impl Hymn {
  pub fn new() -> Hymn {
    Hymn {
      globals: HashMap::new(),
      strings: HashSet::new(),
    }
  }
}

impl Compiler {
  pub fn new() -> Compiler {
    Compiler {
      pos: 0,
      row: 0,
      column: 0,
      script: String::new(),
      source: String::new(),
      previous: Token::new(),
      current: Token::new(),
      error: Option::None,
    }
  }
}

fn next_char(cm: &mut Compiler) -> char {
  let pos = cm.pos;
  if pos == cm.source.len() {
    return '\0';
  }
  let c = cm.source.chars().nth(pos).unwrap();
  cm.pos += 1;
  if c == '\n' {
    cm.row += 1;
    cm.column = 0;
  } else {
    cm.column += 1;
  }
  c
}

fn peek_char(cm: &mut Compiler) -> char {
  if cm.pos == cm.source.len() {
    return '\0';
  }
  cm.source.chars().nth(cm.pos).unwrap()
}

fn advance(cm: &mut Compiler) {
  cm.previous = cm.current;
  if matches!(cm.previous.is, TokenType::End) {
    return;
  }
  loop {
    let c = next_char(cm);
    match c {
      '#' => (),
      _ => return,
    };
  }
}

fn compile(vm: &mut Hymn, source: &str) -> Result<String, String> {
  let mut compiler = Compiler::new();
  Ok(String::from(""))
}

fn interpret(vm: &mut Hymn, source: &str) -> Result<String, String> {
  if source.chars().nth(0).unwrap() == 'a' {
    return Err(String::from("foo"));
  }
  Ok(String::from(""))
}

pub fn read(path: &str) {
  let mut vm = Hymn::new();
  let file = fs::read_to_string(path);
  match file {
    Ok(input) => {
      let result = interpret(&mut vm, &input);
      match result {
        Ok(_) => (),
        Err(error) => println!("Error: {}", error),
      }
    }
    Err(error) => {
      println!("Failed to read file: {}", error);
      process::exit(74);
    }
  };
}

pub fn repl() {
  let mut vm = Hymn::new();
  let mut input = String::new();

  loop {
    print!("> ");
    io::stdout().flush().expect("Failed to flush stdout.");

    io::stdin()
      .read_line(&mut input)
      .expect("Failed to read line.");

    let result = interpret(&mut vm, &input);
    match result {
      Ok(_) => (),
      Err(error) => println!("Error: {}", error),
    }
  }
}
