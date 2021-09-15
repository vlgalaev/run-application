import numpy as np
from time import sleep
import sys

import radexproAPI.dataloadIO as rdx
with rdx.ParallelPrint() as p:
    argv = sys.argv
    traces = rdx.load_traces()
    headers = rdx.load_headers()
    headerNames = rdx.load_header_names()
    parameters = rdx.get_arguments(argv[-1])
    p.report("Data from RDX has received")

    traces *= 3.14
    headers[:,rdx.get_header_id("AAXFILT", headerNames)] *= 3
    headers[:,rdx.get_header_id("dt", headerNames)] *= 2
    p.report("The computation has done")
    
    for i in range(0,100,10):
        sleep(3)
        p.depictWorkPercent(i)
    
    rdx.save_traces(traces)
    rdx.save_headers(headers)
    p.report("Data has sent to RDX")

