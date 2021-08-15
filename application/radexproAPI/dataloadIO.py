from threading import Thread
from traceback import print_tb
from time import sleep
from queue import Queue
import struct
import numpy as np
import re
import sys

def consumer(q):
    while True:
        try:
            data = q.get()
        except:
            pass
        else:
            if data[0] == () and data[1] == {}:
                break
            if data[1].get('file') is None or data[1]['file'] == sys.stdout:
                sleep(1)
                data[1]['flush'] = True
                print(*data[0], **data[1])
            else:
                data[1]['flush'] = False
                print(*data[0], **data[1])

def parallel(func):
    q = Queue()
    thr = Thread(target=consumer, args=(q,))
    thr.start()

    def wrapper(*args, **kwargs):
        if kwargs.get('flush') is None:
            pass
        else:
            del kwargs['flush']
        q.put((args, kwargs))

    return wrapper

@parallel
def _print(*args, **kwargs):
    pass

class ParallelPrint:
    def __init__(self):
        pass
                    
    def __enter__(self):
        pass
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        if exc_type:
            print_tb(exc_tb)
            print(f"{exc_type.__name__} : {exc_val}", file=sys.stderr)
        _print()
        return True

def load_traces(filename='traces'):
    with open(filename, 'rb') as file:
        b = file.read(8)
        n_tr = struct.unpack('l', b[0:4])[0]
        ns = struct.unpack('l', b[4:8])[0]
        traces = np.fromfile(file, dtype=np.single, count=n_tr * ns)
    traces = traces.reshape((n_tr, ns), order='C')
    return traces

def load_headers(filename='headers'):
    with open(filename, 'rb') as file:
        b = file.read(8)
        n_tr = struct.unpack('l', b[0:4])[0]
        nf = struct.unpack('l', b[4:8])[0]
        headers = np.fromfile(file, dtype=np.double, count=n_tr * nf)
    headers = headers.reshape((n_tr, nf), order='C')
    return headers

def load_header_names(filename='headerNames.txt'):
    header_names = []
    with open(filename, 'r') as file:
        for line in file:
            header_names.append(line.strip())
    return np.array(header_names)

def get_header_id(headerName, header_names=None):
    if header_names is None:
        header_names = load_header_names()
    if headerName not in header_names:
        raise ValueError(f"The passed header name ({headerName}) doesn't exist.")
    return np.arange(len(header_names))[header_names == headerName][0]

def get_arguments(arguments=sys.argv[-1]):
    params = {}
    pattern = r"-{2}([A-Za-z_]+\w*)\s*=\s*(\w+)"
    for arg in arguments.split(' '):
        match = re.fullmatch(pattern, arg.strip())
        if match is not None:
            params[match.group(1)] = match.group(2)
    return params

def save_traces(traces, filename='traces'):
    with open(filename, 'wb') as file:
        file.write(struct.pack('ll', *traces.shape))
        file.write(traces.astype(np.float32).tobytes())

def save_headers(headers, filename='headers'):
    with open(filename, 'wb') as file:
        file.write(struct.pack('ll', *headers.shape))
        file.write(headers.astype(np.float64).tobytes())
