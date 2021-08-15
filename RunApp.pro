MODULE_TITLE = "Run Application"
GROUP = "Custom Modules"
MAIN_FUNC_NAME = "runRunApp"
#MODULE_TITLE_SUFFIX_ADDER = getSuffix

include(../../COMMON/Module.pri)
#include($${RADEX_SRC_DIR}COMMON/BoostSupport.pri)
#include($${RADEX_SRC_DIR}COMMON/MklThreadSupport.pri)
#include($${RADEX_SRC_DIR}COMMON/OpenMpSupport.pri)

HEADERS = headers/RunAppAPI.h \
          headers/RunAppModule.h \
          headers/RunAppParameters.h \
          headers/RunAppParametersDialog.h \
		  headers/AppProcess.h


SOURCES = sources/RunAppAPI.cpp \
          sources/RunAppModule.cpp \
          sources/RunAppParameters.cpp \
          sources/RunAppParametersDialog.cpp \
		  sources/AppProcess.cpp

FORMS = forms/RunAppParametersDialog.ui

LIBS += -lConfControl \
        -lDataman -ldmproc -lDMPFlowDef \
        -lDpiAwareness -lHeaderNoValue \
        -lReLogManager -lReplicaTable -lMaskProcessor
