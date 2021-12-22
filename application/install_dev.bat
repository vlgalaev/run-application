@echo off 

REM create & activate the virtual environmet (venv) to compile the application
IF NOT EXIST .\venv\Scripts\activate.bat python -m venv venv
CALL .\venv\Scripts\activate.bat

REM setting up the venv and installing the dependencies
python -m pip install -U pip
pip install -U pyinstaller

pip install -U numpy
REM TODO: install dependencies

set PYTHONPATH=.\bin;.\radexproAPI

REM compile the application
pyinstaller --onefile .\bin\main.py

REM deactivate the venv
CALL .\venv\Scripts\deactivate.bat

MOVE /Y .\dist\main.exe .\

rmdir .\dist\ /s /q
rmdir .\build\ /s /q
rmdir .\bin\__pycache__\ /s /q
del .\main.spec /f
