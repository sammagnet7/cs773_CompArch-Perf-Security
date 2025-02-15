import subprocess

TERMINAL = " gnome-terminal -- "


def open_terminal(command: str):
    command = TERMINAL + command
    subprocess.Popen(command, shell=True)


def open_sender_logs():
    command = "tail -f sender.log"
    open_terminal(command=command)


def open_receiver_logs():
    command = "tail -f receiver.log"
    open_terminal(command=command)


def occupancy():
    command = "cd ../task3a/py_scripts; python master.py;"
    open_terminal(command)


def flush_reload():
    command = "cd ../task2a/; python master.py;"
    open_terminal(command)


def main():
    # send_sharedfile_occupany()
    # send_contentfile_flushreload()
    # open_sender_logs()
    pass


if __name__ == "__main__":
    main()
