import sys
from tabulate import tabulate

LOG_FILE_PATH = "./results/logs/{mode}/{trace}.txt"
GRAPH_IMG_PATH = "./results/graphs/{mode}/{type}.png"
mode_2_dir = {
    "base": "base",
    "way": "way_part",
    "static": "static_set_part",
    "dynamic": "dynamic_set_part",
}


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
    log_file = LOG_FILE_PATH.format(mode=mode_2_dir[mode], trace=trace)
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
        "ipc_total": 0,
        "eviction_core0": 0,
        "eviction_core1": 0,
        "eviction_total": 0,
    }
    selected_core = ""
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
    obj["ipc_total"] = obj["ipc_core0"] + obj["ipc_core1"]
    total_instructions = 50000000 * 2
    obj["eviction_total"] = (
        (obj["eviction_core0"] + obj["eviction_core1"]) / total_instructions * 1000
    )  # 1K/50M*2
    return obj


def parse_data(mode: str):
    datas = []
    for trace in [
        "mcf-mcf",
        "mcf-perl",
        # "perl-mcf",
        "perl-perl",
    ]:
        datas.append(extract_data(mode=mode, trace=trace))
    return datas


def compare_data(parsed_data, modes):
    indexed_data = {}
    for data in parsed_data:
        _mode = data["mode"]
        trace = data["trace"]
        ipc_total = data["ipc_total"]
        mpki_avg = data["mpki_avg"]
        eviction_total = data["eviction_total"]
        indexed_data.update(
            {
                _mode
                + "-"
                + trace: {
                    "ipc_total": ipc_total,
                    "mpki_avg": mpki_avg,
                    "eviction_total": eviction_total,
                }
            }
        )
    comparison_data = []
    other_modes = modes
    other_modes.remove("base")
    for mode in modes:
        for trace in [
            "mcf-mcf",
            "mcf-perl",
            # "perl-mcf",
            "perl-perl",
        ]:
            base_ipc_total = indexed_data["base" + "-" + trace]["ipc_total"]
            base_mpki_avg = indexed_data["base" + "-" + trace]["mpki_avg"]
            base_eviction_total = indexed_data["base" + "-" + trace]["eviction_total"]
            new_ipc_total = indexed_data[mode + "-" + trace]["ipc_total"]
            new_mpki_avg = indexed_data[mode + "-" + trace]["mpki_avg"]
            new_eviction_total = indexed_data[mode + "-" + trace]["eviction_total"]
            normalized_ipc = round(new_ipc_total / base_ipc_total, 2)
            comparison_data.append(
                {
                    "mode": mode,
                    "trace": trace,
                    "normalized_ipc": normalized_ipc,
                    "base_mpki": base_mpki_avg,
                    "new_mpki": new_mpki_avg,
                    "base_evictions": base_eviction_total,
                    "new_eviction": new_eviction_total,
                }
            )
    return comparison_data


import matplotlib.pyplot as plt
import numpy as np


import matplotlib.pyplot as plt
import numpy as np


def draw_graph(title, ylabel, labels, base_values, new_values, file_name):
    x = np.arange(len(labels))
    width = 0.35  # Bar width

    color_blue = "#4F81BD"
    color_red = "#C0504D"

    fig, ax = plt.subplots()

    if base_values:
        bars1 = ax.bar(
            x - width / 2,
            base_values,
            width,
            label="Non Secure System",
            color=color_blue,
        )
        bars2 = ax.bar(
            x + width / 2,
            new_values,
            width,
            label="Secure System",
            color=color_red,
        )

        # Place text labels at the correct top position of each bar
        for bar in bars1:
            bar.set_path_effects([])
            bar.set_linestyle("solid")
            bar.set_capstyle("round")

            height = bar.get_height()
            ax.text(
                bar.get_x() + bar.get_width() / 2,
                height * 1.02,  # Small offset to avoid overlap
                f"{height:.2f}",
                ha="center",
                color=color_blue,
                fontsize=11,
                fontweight="bold",
            )

        for bar in bars2:
            bar.set_path_effects([])
            bar.set_linestyle("solid")
            bar.set_capstyle("round")

            height = bar.get_height()
            ax.text(
                bar.get_x() + bar.get_width() / 2,
                height * 1.02,
                f"{height:.2f}",
                ha="center",
                color=color_red,
                fontsize=11,
                fontweight="bold",
            )

    else:
        bars = ax.bar(
            x, new_values, color=color_blue, width=0.5, label="Normalized IPC"
        )

        # Place text labels on top of the bars
        for bar in bars:
            bar.set_path_effects([])
            bar.set_linestyle("solid")
            bar.set_capstyle("round")

            height = bar.get_height()
            ax.text(
                bar.get_x() + bar.get_width() / 2,
                height + 0.01,  # Fixed small offset for IPC
                f"{height:.2f}",
                ha="center",
                color=color_blue,
                fontsize=11,
                fontweight="bold",
            )

    ax.set_xlabel("Trace")
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    ax.set_xticks(x)
    ax.set_xticklabels(labels, rotation=0)
    ax.legend()

    plt.grid(True, linestyle="-", alpha=0.7, axis="y")
    plt.savefig(file_name, bbox_inches="tight")
    plt.close(fig)
    print(f"Saved {file_name}")


def process_and_plot(data):
    modes = set(row["mode"] for row in data)  # Get unique modes

    for mode in modes:
        mode_data = [row for row in data if row["mode"] == mode]  # Filter data by mode
        traces = [row["trace"] for row in mode_data]

        # Extracting values
        ipc_values = [row["normalized_ipc"] for row in mode_data]
        base_mpki = [row["base_mpki"] for row in mode_data]
        new_mpki = [row["new_mpki"] for row in mode_data]
        base_evictions = [
            row["base_evictions"] / 1000 for row in mode_data
        ]  # Convert to per K instructions
        new_evictions = [row["new_eviction"] / 1000 for row in mode_data]

        # Generating 3 plots per mode
        draw_graph(
            f"Normalized IPC ({mode} partition)",
            "Normalized IPC",
            traces,
            None,
            ipc_values,
            GRAPH_IMG_PATH.format(mode=mode_2_dir[mode], type="ipc"),
        )
        draw_graph(
            f"LLC MPKI ({mode} partition)",
            "LLC MPKI",
            traces,
            base_mpki,
            new_mpki,
            GRAPH_IMG_PATH.format(mode=mode_2_dir[mode], type="mpki"),
        )
        draw_graph(
            f"Self Evictions Per Kilo Instructions ({mode} partition)",
            "Evictions (K)",
            traces,
            base_evictions,
            new_evictions,
            GRAPH_IMG_PATH.format(mode=mode_2_dir[mode], type="evictions"),
        )


def main(_modes: str):
    if _modes == "all":
        _modes = "base,way,static,dynamic"
    modes = _modes.split(",")
    parsed_data = []
    for mode in modes:
        parsed_data.extend(parse_data(mode=mode))
    print("Parsed Data:::")
    print(tabulate(parsed_data, headers="keys", tablefmt="grid"))

    if len(modes) == 1 or "base" not in modes:
        return

    comparison_data = compare_data(parsed_data=parsed_data, modes=modes)
    print("Comparison data:::")
    print(tabulate(comparison_data, headers="keys", tablefmt="grid"))

    process_and_plot(comparison_data)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <base|way|static|dynamic|all>")
        exit(1)
    arg_modes = sys.argv[1]
    main(_modes=arg_modes)
