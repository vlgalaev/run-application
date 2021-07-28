from radexproAPI.dataload import load_traces, load_headers, load_headerNames, get_headerID
from radexproAPI.datasave import save_traces, save_headers
import numpy as np
import sys


traces = load_traces()
headers = load_headers()
headerNames = load_headerNames()


traces *= 3.14
headers[:,get_headerID("AAXFILT", headerNames)] *= 2


save_traces(traces)
save_headers(headers)


