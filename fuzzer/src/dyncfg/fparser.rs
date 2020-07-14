use crate::dyncfg::cfg::{BasicBlockId, CmpId, Edge, ControlFlowGraph};

use std::collections::{HashMap, HashSet};
use std::path::Path;
use std::io::{Error, ErrorKind};
use std::fs;



fn parse_cfg_file(path: &Path) -> Result<HashSet<Edge>, Error> {
    let file = fs::File::open(path)?;
    let mut result = HashSet::new();
    let mut rdr = csv::Reader::from_reader(file);
    for entry in rdr.deserialize() {
        let edge: Edge = entry?;
        result.insert(edge);
    }
    Ok(result)
}

fn parse_cmp_file(path: &Path) -> Result<HashMap<BasicBlockId, CmpId>, Error> {
    let file = fs::File::open(path)?;
    let mut result = HashMap::new();
    let mut rdr = csv::Reader::from_reader(file);
    for entry in rdr.deserialize() {
        let (k,v) : (CmpId, BasicBlockId) = entry?;
        result.insert(v,k);
    }
    Ok(result)
}

fn parse_targets_file(path: &Path) -> Result<HashSet<BasicBlockId>, Error> {
    let file = fs::File::open(path)?;
    let mut result = HashSet::new();
    let mut rdr = csv::Reader::from_reader(file);
    for entry in rdr.deserialize() {
        let bb: BasicBlockId = entry?;
        result.insert(bb);
    }
    Ok(result)
}

pub fn parse_cfg(cfg_path: &Path, cmp_path: &Path, targets_path: &Path) -> Result<ControlFlowGraph, Error> {

    let edges = parse_cfg_file(cfg_path)?;
    let bb_cmp = parse_cmp_file(cmp_path)?;
    print!("{:?}", edges);
    print!("{:?}", bb_cmp);
    let targets = parse_targets_file(targets_path)?;
    //let r = ControlFlowGraph::new_with(edges, targets, bb_cmp);
    let mut r = ControlFlowGraph::new(targets);
    for e in edges {
        r.add_edge(e)
    }

    Ok(r)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parse_example() {
        let cfg_file = Path::new("cfg.csv");
        let cmp_file = Path::new("cmp.csv");
        let targets_file = Path::new("targets.csv");
        let data = parse_cfg(cfg_file, cmp_file, targets_file).unwrap();
        assert!(data.has_edge((39,59)));
        assert!(data.has_edge((15,25)));
        //assert!(data.targets.contains(&60));
    }


}
