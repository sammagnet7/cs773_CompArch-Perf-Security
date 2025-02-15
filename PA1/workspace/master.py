import subprocess
import sys

TERMINAL = "gnome-terminal --wait -- "


def open_terminal(command: str, workdir: str = ".", wait: bool = False):
    __command = TERMINAL + command
    print(__command)
    if wait:
        subprocess.run(__command, shell=True, cwd=workdir, check=False)
    else:
        subprocess.Popen(
            __command,
            shell=True,
            cwd=workdir,
        )

def open_sender_logs():
    command = "tail -f sender.log"
    open_terminal(command=command)


def open_receiver_logs():
    command = "tail -f receiver.log"
    open_terminal(command=command)


def occupancy(file_name: str = ""):
    command = f"python master.py {file_name};"
    open_terminal(command, workdir="task2b/py_scripts", wait=True)


def flush_reload(file_name: str = ""):
    command = f"python master.py {file_name};"
    open_terminal(command, workdir="task3a", wait=True)


def only_flushreload():
    flush_reload()


def only_occupancy():
    occupancy()


def occupancy_flushreaload():
    file_name = input("Enter shared file name: ")
    occupancy(file_name=file_name)
    flush_reload(file_name=file_name)

def print_help():
    print(f"usage: {sys.argv[0]} [2a|2b|3a|3b]")

def main():
    prompt = sys.argv[1]
    if prompt == "2a":
        only_flushreload()
    elif prompt == "2b":
        only_occupancy()
    elif prompt == "3a":
        occupancy_flushreaload()
    elif prompt == "3b":
        occupancy_flushreaload()
    elif prompt == "help":
        print_help()
    else:
        print(f"Invalid option: {prompt}")
        print(f"usage: {sys.argv[0]} [2a|2b|3a|3b]")


if __name__ == "__main__":
    main()
