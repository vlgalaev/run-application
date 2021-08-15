from radexproAPI.dataloadIO import (load_traces, load_headers, load_header_names, get_header_id, get_arguments,
    save_traces, save_headers, ParallelPrint, _print)
import numpy as np
import sys

with ParallelPrint() as p:
    argv = sys.argv
    traces = load_traces()
    headers = load_headers()
    headerNames = load_header_names()
    parameters = get_arguments(argv[-1])
    _print("Data from RDX has received")

    traces *= 3.14
    headers[:,get_header_id("AAXFILT", headerNames)] *= 3
    _print("The computation has done")

    save_traces(traces)
    save_headers(headers)
    _print("Data has sent to RDX")

