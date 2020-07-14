use angora_common::defs;
use std::{self, cmp::Ordering, fmt};

const INIT_PRIORITY: u16 = 0;
const AFL_INIT_PRIORITY: u16 = 0;
const DONE_PRIORITY: u16 = std::u16::MAX;
const INIT_DISTANCE: u32 = std::u32::MAX;

#[derive(Eq, PartialEq, Clone, Copy, Debug)]
pub struct QPriority(u16, u32);
impl QPriority {
    pub fn inc(&self, op: u32) -> Self {
        if op == defs::COND_AFL_OP {
            self.afl_inc()
        } else {
            self.base_inc()
        }
    }

    pub fn new_distance(&self, distance: u32) -> Self {
        QPriority(self.0, distance)
    }

    fn base_inc(&self) -> Self {
        QPriority(self.0 + 1, self.1)
    }

    fn afl_inc(&self) -> Self {
        QPriority(self.0 + 2, self.1)
    }

    pub fn init(op: u32) -> Self {
        Self::init_distance(op, INIT_DISTANCE)
    }

    pub fn init_distance(op: u32, distance: u32) -> Self {
        if op == defs::COND_AFL_OP {
            Self::afl_init(distance)
        } else {
            Self::base_init(distance)
        }
    }


    fn base_init(distance: u32) -> Self {
        QPriority(INIT_PRIORITY, distance)
    }

    fn afl_init(distance: u32) -> Self {
        QPriority(AFL_INIT_PRIORITY, distance)
    }

    pub fn done() -> Self {
        QPriority(DONE_PRIORITY, INIT_DISTANCE)
    }

    pub fn is_done(&self) -> bool {
        self.0 == DONE_PRIORITY
    }
}

// Make the queue get smallest priority first.
impl Ord for QPriority {
    fn cmp(&self, other: &QPriority) -> Ordering {
        match self.0.cmp(&other.0) {
            Ordering::Greater => Ordering::Less,
            Ordering::Less => Ordering::Greater,
            Ordering::Equal => match self.1.cmp(&other.1) {
                Ordering::Greater => Ordering::Less,
                Ordering::Less => Ordering::Greater,
                Ordering::Equal => Ordering::Equal,
            }
        }
    }
}

impl PartialOrd for QPriority {
    fn partial_cmp(&self, other: &QPriority) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl fmt::Display for QPriority {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.0)
    }
}
