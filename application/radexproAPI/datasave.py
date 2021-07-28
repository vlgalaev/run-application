import struct
import numpy as np

def save_traces(traces, filename='traces'):
    with open(filename, 'wb') as file:
        file.write(struct.pack('ll', *traces.shape))
        file.write(traces.astype(np.float32).tobytes())

def save_headers(headers, filename='headers'):
    with open(filename, 'wb') as file:
        file.write(struct.pack('ll', *headers.shape))
        file.write(headers.astype(np.float64).tobytes())
