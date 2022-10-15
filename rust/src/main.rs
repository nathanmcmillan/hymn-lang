use hymn::hymn;
use std::env;
use std::error::Error;
use std::fs;

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() <= 2 {
        println!("Usage: hymn [-b] [-c] FILE");
        println!("Interprets a Hymn script FILE.");
        println!();
        println!("  -b  Print compiled byte code");
        println!("  -c  Run FILE as source code");
    }

    let vm = hymn::new_vm();

    let err: Result<(), Error>;

    if args.len() >= 4 {
        if args[2] == "-b" {
            if args.len() >= 5 {
                if args[3] == "-c" {
                    let source = &args[4];
                    let res = hymn::debug_interpret(vm, source);
                } else {
                    println!("Unknown second argument: {}", args[3]);
                }
            } else {
                let script = &args[3];
                let read = fs::read_to_string(script);
                if err == nil {
                    let source = string(content);
                    err = hymn::debug_interpret_script(vm, script, source);
                }
            }
        } else if args[2] == "-c" {
            let source = args[2];
            err = hymn::interpret(vm, source);
        } else {
            println!("Unknown argument: {}", args[2]);
        }
    } else {
        let script = &args[2];
        let read = fs::read_to_string(script);
        let content;
        match read {
            Ok(r) => content = r,
            Err(e) => err = e,
        };
    }

    if err.ok != nil {
        println!("{}", err.to_string());
    }
}
