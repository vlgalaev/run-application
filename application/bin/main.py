from radexproAPI.dataload import load_traces, load_headers, load_headerNames, get_headerID, get_arguments
from radexproAPI.datasave import save_traces, save_headers
import numpy as np
import sys


try:
    try:
        traces = load_traces()
        headers = load_headers()
        headerNames = load_headerNames()
    except BaseException:
        sys.exit(1)
        
        
    try:
        parameters = get_arguments(sys.argv)
    except Exception as exc:
        sys.exit(3)
        
    '''
    try:
        tr_scalar = float(parameters['scalar_for_traces'])
        h_scalar = float(parameters['scalar_for_header'])
    except KeyError:
        sys.exit(4)
    traces *= float(parameters['scalar_for_traces'])
    headers[:,get_headerID("AAXFILT", headerNames)] *= float(parameters['scalar_for_header'])
    '''
    traces *= 3.14
    headers[:,get_headerID("AAXFILT", headerNames)] *= 3
    
    
    try:
        save_traces(traces)
        save_headers(headers)
    except BaseException:
        sys.exit(1)
except MemoryError:
    sys.exit(2)

    