pub struct Hymn {
    frame_count: i32,
}

pub fn new_vm() -> Hymn {
    return Hymn { frame_count: 0 };
}

pub fn interpret(source: &String) -> Result<(), String> {
    return Ok(());
}

pub fn interpret_script(script: &String, source: &String) -> Result<(), String> {
    return Ok(());
}

pub fn debug_interpret(source: &String) -> Result<(), String> {
    return Ok(());
}

pub fn debug_interpret_script(script: &String, source: &String) -> Result<(), String> {
    return Ok(());
}
