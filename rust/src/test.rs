use std::fs;

#[cfg(test)]
mod tests {

    #[test]
    fn test() {
        let paths = fs::read_dir("../test/language").unwrap();
        for path in paths {
            println!("File: {}", path.unwrap().path().display());
            test(path);
        }
    }
}
