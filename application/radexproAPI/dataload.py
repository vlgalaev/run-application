import struct
import numpy as np
import re

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
   
   
def load_headerNames(filename='headerNames.txt'):
    headerNames = []
    with open(filename, 'r') as file:
        for line in file:
            headerNames.append(line.strip())
    return np.array(headerNames)
    
    
def get_headerID(headerName, headerNames=None):
    if headerNames is None:
        headerNames = load_headerNames()
    if headerName not in headerNames:
        raise ValueError(f"The passed header name ({headerName}) doesn't exist.")
    return np.arange(len(headerNames))[headerNames == headerName][0]
  
  
def get_arguments(appArgv=[]):
    params = {}
    pattern = r"-{2}([A-Za-z_]+\w*)\s*=\s*(\w+)"
    for arg in appArgv[1:]:
        match = re.fullmatch(pattern, arg.strip())
        if match is not None:
            params[match.group(1)] = match.group(2)
    return params
    