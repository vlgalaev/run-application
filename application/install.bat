@echo off 

REM create & activate the virtual environmet (venv) to compile the application
python -m venv venv
CALL .\venv\Scripts\activate.bat

REM setting up the venv and installing the dependencies
python -m pip install -U pip
pip install pyinstaller

pip install numpy
REM TODO: install dependencies

REM setting up environment variables locally
setlocal
PYTHONPATH = .\bin;.\radexporAPI
REM TODO: add new catalogs if it is needed
endlocal

REM compile the application
pyinstaller --onefile .\bin\main.py

REM deactivate the venv
CALL .\venv\Scripts\deactivate.bat

MOVE /Y .\dist\main.exe .\

rmdir .\venv\ /s /q
rmdir .\dist\ /s /q
rmdir .\build\ /s /q
rmdir .\bin\__pycache__\ /s /q
del .\main.spec /f