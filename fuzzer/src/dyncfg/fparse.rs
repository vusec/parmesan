use std::path::Path;
use std::collections::{HashSet, HashMap};
use std::io;
use std::fs::File;
use std::io::BufReader;

use super::cfg::{CmpId, CallSiteId, Edge};
use serde::de;
use serde::de::{Deserialize, Deserializer};
use std::hash::Hash;
use std::str::FromStr;
use std::fmt::Display;

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct CfgFile {
    pub targets: HashSet<CmpId>,
    #[serde(default)]
    pub edges: HashSet<Edge>,
    #[serde(default, deserialize_with = "de_int_key")]
    pub callsite_dominators: HashMap<CallSiteId, HashSet<CmpId>>,
}

fn de_int_key<'de, D, K, V>(deserializer: D) -> Result<HashMap<K, V>, D::Error>
where
    D: Deserializer<'de>,
    K: Eq + Hash + FromStr,
    K::Err: Display,
    V: Deserialize<'de>,
{
    let string_map = <HashMap<String, V>>::deserialize(deserializer)?;
    let mut map = HashMap::with_capacity(string_map.len());
    for (s, v) in string_map {
        let k = K::from_str(&s).map_err(de::Error::custom)?;
        map.insert(k, v);
    }
    Ok(map)
}


pub fn parse_targets_file(path: &Path) -> io::Result<CfgFile> {
    let reader = BufReader::new(File::open(path).expect("Could not read targets file"));
    let result = serde_json::from_reader(reader)?;

    return Ok(result);
}
