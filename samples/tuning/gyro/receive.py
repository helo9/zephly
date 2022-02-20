import serial
import time
from enum import Enum

SERIAL_PORT = "/dev/ttyACM2"
SERIAL_BAUD = 230400
SYNC_BYTE = b"\xaa"
MSG_CACHE_SIZE = 100
OUTPUT_FILENAME = "out.bin"

class States(Enum):
    NOT_SYNCHRONIZED = 1
    SYNCHRONIZATION1 = 2
    SYNCHRONIZATION2 = 3
    RECEIVE_LENGTH   = 4
    GETTING_DATA     = 5

def _start_timeout(timeout=None, print_stats=False):
    
    start = time.monotonic()

    if timeout is not None:
        def _is_timed_out():
            return (time.monotonic() - start) >= timeout

        return _is_timed_out
    else:
        return lambda: False

def  _print_stats(msg_rate, msg_length_mean, do_it=False):
    if not do_it:
        return

    print("writing, got mesages with rate {:4.2f} Hz and mean msg len {:2.2f} B".format(msg_rate, msg_length_mean))

def receive(timeout=None, print_stats=False):
    # cleanup output file
    with open(OUTPUT_FILENAME, "wb"):
        pass

    ser = serial.Serial(SERIAL_PORT, baudrate=SERIAL_BAUD)
    running = True
    state = States.NOT_SYNCHRONIZED
    msg_length = 0
    msg_buf = []

    is_timed_out = _start_timeout(timeout)
    last_ts = None

    while not is_timed_out():
        
        n = msg_length if state == States.GETTING_DATA else 1
        c = ser.read(n)

        match state:
            case States.NOT_SYNCHRONIZED | States.SYNCHRONIZATION1:
                if c == SYNC_BYTE:
                    state = States.SYNCHRONIZATION2
            case States.SYNCHRONIZATION2:
                if c == SYNC_BYTE:
                    state = States.RECEIVE_LENGTH
                else:
                    state = States.SYNCHRONIZATION1
            case States.RECEIVE_LENGTH:
                msg_length = int.from_bytes(c, "little")
                state = States.GETTING_DATA
            case States.GETTING_DATA:
                state = States.SYNCHRONIZATION1
                msg_buf.append(c)


        if len(msg_buf) > MSG_CACHE_SIZE:
            with open(OUTPUT_FILENAME, "ab") as outfile:           
                outfile.write(b"".join(msg_buf))

                now = time.monotonic_ns()
                if last_ts is not None:
                    msg_rate = MSG_CACHE_SIZE * 1e9 / (now-last_ts)

                    msg_length_mean = sum([len(msg) for msg in msg_buf]) / len(msg_buf)
                    
                    _print_stats(msg_rate, msg_length_mean, print_stats)
                last_ts = now

                
            msg_buf = []

if __name__=='__main__':
    receive(10, print_stats=True)
