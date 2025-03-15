import sys
from tabulate import tabulate

LOG_FILE_PATH = "./results/logs/{mode}/{trace}.txt"


def grep_lines(search_strings: list[str], filename: str = None, content: str = None):
    if not content:
        try:
            with open(filename, "r") as file:
                content = file.readlines()
        except FileNotFoundError:
            print(f"Error: File '{filename}' not found.")
            exit(1)
        except Exception as e:
            print(f"Error: {e}")
            exit(1)
    return "\n".join(
        [
            line.strip()
            for line in content
            if any(search_string in line for search_string in search_strings)
        ]
    )


def extract_data(mode: str, trace: str):
    log_file = LOG_FILE_PATH.format(mode=mode, trace=trace)
    mpki_lines_all = grep_lines(
        filename=log_file,
        search_strings=[
            "LLC TOTAL",
            "CPU 1 cumulative IPC",
            "CPU 0 cumulative IPC",
            "Self-eviction count",
        ],
    )
    mpki_lines = mpki_lines_all.split("\n")[4:]
    # print("\n".join(mpki_lines))
    obj = {
        "mode": mode,
        "trace": trace,
        "mpki_core0": 0,
        "mpki_core1": 0,
        "mpki_avg": 0,
        "ipc_core0": 0,
        "ipc_core1": 0,
        "ipc_avg": 0,
        "eviction_core0": 0,
        "eviction_core1": 0,
    }
    selected_cpu = ""
    for line in mpki_lines:
        if "Self-eviction" in line:
            cur_core = line.split("CPU ")[1].split()[0]
            eviction_count = int(line.split(":")[-1])
            obj["eviction_core" + cur_core] = eviction_count
        elif line.startswith("CPU"):
            selected_core = line.split("CPU ")[1].split()[0]
            ipc = float(line.split("cumulative IPC:")[1].split()[0])
            obj["ipc_core" + selected_core] = ipc
        else:
            mpki = float(line.split("MPKI:")[1].split()[0])
            obj["mpki_core" + selected_core] = mpki
    obj["mpki_avg"] = (obj["mpki_core0"] + obj["mpki_core1"]) / 2
    obj["ipc_avg"] = (obj["ipc_core0"] + obj["ipc_core1"]) / 2
    # print(obj)
    return obj


def base(mode: str):
    datas = []
    for trace in [
        "mcf-mcf",
        "mcf-perl",
        # "perl-mcf",
        "perl-perl",
    ]:
        datas.append(extract_data(mode=mode, trace=trace))
    return datas


def main(mode: str):
    ret = []
    if mode != "all":
        return base(mode=mode)
    ret.append(base(mode="base"))
    ret.append(base(mode="way"))
    ret.append(base(mode="set"))
    ret.append(base(mode="dynamic"))
    return ret


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <base|way|set|dynamic|all>")
        exit(1)
    arg_mode = sys.argv[1]
    datas = main(mode=arg_mode)
    print(tabulate(datas, headers="keys", tablefmt="grid"))
