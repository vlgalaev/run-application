# Example Application

One of the main purposes of GeoHazard-Solver project is to create 
some instruments which let to perform cluster analysis of seismic data
in [RadExPro](https://radexpro.com/ru/). This applications is one of them.

## Application Description

It allows you to multiply each trace by 3.14 and multiply `AAXFILT` header by 2

It is created to run under RadExPro Software using the 
**Run Application** RDX module.


## Dependencies

RadExPro Module **Run Application**
[Python 3](https://www.python.org/downloads/release/python-396/) assembled
with packages:

| library | version | link |
| ------ | ------ | ------ |
| numpy | 1.21.0 | https://numpy.org/doc/stable/index.html |


## Installing

Run **install.bat** by either clicking on it twice with left button of the mouse 
or entering the following instruction into cmd at the folder of **install.bat**
`.\install.bat`


## Executing the application

Choose the executable **main.exe** in **Run Application**
module and then run RDX flow with the module. The application execution 
starts as soon as the module starts and it ends as soon as the module ends.



