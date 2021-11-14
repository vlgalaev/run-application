@echo off 

REM create & activate the virtual environmet (venv) to compile the application
IF NOT EXIST .\venv\Scripts\activate.bat python -m venv venv
CALL .\venv\Scripts\activate.bat

REM setting up the venv and installing the dependencies
python -m pip install -U pip
IF NOT EXIST .\venv\Lib\site-packages\PyInstaller\__init__.py pip install pyinstaller

IF NOT EXIST .\venv\Lib\site-packages\numpy\__init__.py pip install numpy
REM TODO: install dependencies

REM compile the application
pyinstaller --onefile .\bin\main.py -p .\radexproAPI

REM deactivate the venv
CALL .\venv\Scripts\deactivate.bat

MOVE /Y .\dist\main.exe .\

rmdir .\dist\ /s /q
rmdir .\build\ /s /q
rmdir .\bin\__pycache__\ /s /q
del .\main.spec /f
