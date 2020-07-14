"""
A more lightweight and compatible alternative to target pruning than ASAP.
"""
import sys
import os
import glob
import subprocess
import json
import csv
from statistics import quantiles
import logging

logging.basicConfig(stream=sys.stderr, level=logging.INFO)

SCRIPT_PATH = os.path.dirname(__file__)

TARGET_PERCENTILE = 50

bb_cmp_map = {}

def exec_and_collect_log(cmd, input_file):
    track_log_file = os.path.join(os.getcwd(), "profile_track.out")
    envs = {"ANGORA_TRACK_OUTPUT": f"{track_log_file}"}
    cmd = list(map(lambda e: input_file if e == "@@" else e, cmd))
    ret = subprocess.call(cmd, env=envs, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if ret != 0:
        logging.debug("Target program returned non-0 code")
    json_data = subprocess.check_output([f"{SCRIPT_PATH}/../bin/log_reader", track_log_file])
    data = json.loads(json_data)
    return data

def parse_bb_cmp_file(bb_cmp_file):
    global bb_cmp_map
    with open(bb_cmp_file) as fh:
        reader = csv.DictReader(fh)
        for row in reader:
            bb_cmp_map[int(row["bbId"])] = int(row["cmpId"])

def map_bbs_to_conds(bbs):
    return set(filter(None, map(lambda bb: bb_cmp_map.get(int(bb), None), bbs)))

def collect_func_diff_weights(diff_file):
    with open(diff_file, "rb") as fh:
        data = fh.read().decode().split("\n\n")
        func_counts = {}
        func_bbs = {}
        for entry in map(lambda s: s.strip(), data):
            count = 0
            if not entry.startswith("in "):
                continue
            lines = entry.splitlines()
            if len(lines) == 0:
                continue
            # Magic to extract function name
            func_name_line = lines[0].strip().split()
            func_name = lines[0].strip().split()[2][:-1] # Remove trailing :
            bbs = set()
            for line in lines:
                line = line.strip()
                if len(line) > 0 and line[0] == ">":
                    count += 1
                if line.startswith("in block"):
                    # Collect basic blocks belonging to function
                    bb_id = line.split()[5][1:-2]
                    bbs.add(bb_id)
            func_counts[func_name] = count
            func_bbs[func_name] = bbs

        sorted_counts = sorted(func_counts.items(), key=lambda e: e[1], reverse=True)
        count_percentiles = quantiles(map(lambda e: e[1], sorted_counts), n=100)
        filtered_funcs = list(filter(lambda e: e[1] > count_percentiles[TARGET_PERCENTILE], sorted_counts))
        bbs_to_target = set()
        for (func, count) in filtered_funcs:
            bbs_to_target |=func_bbs.get(func, set())


        promising_conds = map_bbs_to_conds(bbs_to_target)
        return promising_conds


def start():
    target_file = sys.argv[1]
    diff_file = sys.argv[2]
    bb_cmp_file = sys.argv[3]
    input_dir = sys.argv[4]
    track_bin_cmd = sys.argv[5:]

    parse_bb_cmp_file(bb_cmp_file)

    target_data = None
    with open(target_file, "rb") as fh:
        target_data = json.loads(fh.read())

    conds_to_filter = set()

    for input_file in glob.glob(f"{input_dir}/*"):
        logging.debug(f"Running '{input_file}'")
        logging.debug(f"CMD: {track_bin_cmd}")
        data = exec_and_collect_log(track_bin_cmd, input_file)
        for cond in data["cond_list"]:
            conds_to_filter.add(cond["cmpid"])


    num_targets = []
    num_targets.append(len(target_data["targets"]))

    for cid in conds_to_filter:
        try:
            target_data["targets"].remove(cid)
        except ValueError:
            pass # Do nothing

    logging.debug(f"Pruned {len(conds_to_filter)} targets: {conds_to_filter}")
    num_targets.append(len(target_data["targets"]))

    promising_conds = collect_func_diff_weights(diff_file)

    # Only keep promising (high-diff function) targets
    target_data["targets"] = list(filter(lambda c: c in promising_conds, target_data["targets"]))
    num_targets.append(len(target_data["targets"]))

    logging.debug(f"Target count per pruning step: {num_targets}")
    print(json.dumps(target_data, sort_keys=True, indent=4, separators=(',', ': ')))

def print_usage():
    print(f"Usage: {sys.argv[0]} TARGET_FILE DIFF_FILE BB_CMP_FILE INPUT_DIR [TRACK_BINARY_CMD..]")
    print()
    print("Produce TARGET_FILE (targets.json) and DIFF_FILE: `llvm-diff-parmesan -json myprog.bc myprog.asan.bc 2> myprog.diff`")
    print("Produce TRACK_BINARY with `USE_TRACK=1 angora-clang -o myprog.track mybrog.bc")


if __name__ == "__main__":
    if len(sys.argv) < 6:
        print_usage()
        sys.exit(1)
    start()
