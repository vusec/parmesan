use std::path::Path;
use std::collections::{HashSet, HashMap};
use std::io;
use std::fs::File;
use std::io::{BufReader, BufRead};
use serde::{Deserialize, Serialize};
use serde_json::Result;
use crate::dyncfg::cfg::CmpId;

pub type Target = CmpId;
pub type Edge = (Target, Target);

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct CfgFile {
    pub targets: HashSet<Target>,
    pub edges: HashSet<Edge>,
}

pub fn parse_targets_file(path: &Path) -> io::Result<CfgFile> {
    /*
    let mut result = HashMap::new();

    let reader = BufReader::new(File::open(path).expect("Could not read targets file"));
    for line in reader.lines() {
        let data = line.unwrap();
        let s : Vec<&str> = data.split(" ").collect();
        let t = s[0];
        let d = s[1];
        //let entry = line.unwrap().parse::<Target>().unwrap();
        let entry = t.parse::<Target>().unwrap();
        let distance = t.parse::<u32>().unwrap();
        result.insert(entry, distance);
    }

    debug!("Targets: {:?}", result);
    */
    let reader = BufReader::new(File::open(path).expect("Could not read targets file"));
    let result = serde_json::from_reader(reader)?;
    //let targets = HashSet::new();
    //let edges = HashSet::new();
    //let result = CfgFile{targets, edges};

    return Ok(result);
}
