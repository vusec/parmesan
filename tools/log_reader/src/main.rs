use std::env;
use runtime::get_log_data;
use std::path::Path;

#[macro_use]
extern crate serde_json;

fn main() {
    let args: Vec<String> = env::args().collect();
    //println!("{:?}", args);
    let track_file_name = Path::new(&args[1]);

    let data = get_log_data(track_file_name).unwrap();
    //println!("{:?}", data);

    println!("{}", serde_json::to_string_pretty(&data).unwrap());
}
