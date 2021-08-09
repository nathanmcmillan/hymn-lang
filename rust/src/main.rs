mod hymn;

fn main() {
    let args: Vec<String> = std::env::args().collect();
    match args.len() {
        1 => hymn::repl(),
        _ => hymn::read(&args[1]),
    }
}
