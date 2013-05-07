
PRJ=$(OO_SDK_HOME)
SETTINGS=$(PRJ)/settings
include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

EXTENSION_VERSION=$(shell cat VERSION)
EXTENSION_ID=mytools.calc.CppWatchingWindow
EXTENSION_NAME=WatchingWindow
EXTENSION_DISPLAY_NAME=WatchingWindow
IMPLE_NAME=mytools.calc.CppWatchingWindow
MANAGER_FACTORY_IMPLE_NAME=bookmarks.BookmarksPopupManagerFactory
OPTIONS_IMPLE_NAME=mytools.config.CppWatchingWindowOptions

IN_TASKPANE=false

ifeq "$(IN_TASKPANE)" "true"
TASKPANE_DEF=-DTASKPANE
else
TASKPANE_DEF=
endif

ifeq "$(SDKVERSION)" "3.4"
VERSION_DEF=
IN_TASKPANE=false
ifneq "$(PLATFORM)" "windows"
EXTENSION_PLATFORM=$(PLATFORM)_x86
else
ifneq "$(PLATFORM)" "linux"
ifneq "$(PROCTYPE)" "i386"
EXTENSION_PLATFORM=$(PLATFORM)_x86
else
ifneq "$(PROCTYPE)" "x86_64"
EXTENSION_PLATFORM=$(PLATFORM)_x86_64
endif
endif
endif
endif

else
VERSION_DEF=-DAOO4
# since AOO4
include $(SETTINGS)/platform.mk
endif


COMP_NAME=ww
COMP_IMPL_NAME=$(COMP_NAME).uno.$(SHAREDLIB_EXT)

OPTIONS_COMP_NAME=wwop
OPTIONS_COMP_IMPL_NAME=$(OPTIONS_COMP_NAME).uno.$(SHAREDLIB_EXT)

ifeq "$(OS)" "WIN"
ORIGIN_NAME=%%origin%%
CC_FLAGS+= /O2 
else
ORIGIN_NAME=%origin%
CC_FLAGS=-c -Os -fpic
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS) 
ifeq "$(SDKVERSION)" "3.4"
CC_FLAGS+= -fvisibility=hidden
else
COMP_LINK_FLAGS+= -Wl,--version-script,$(SETTINGS)/component.uno.map 
endif
endif

SRC=./src
OUT=.
CONFIG_DIR=./config
BUILD_DIR=./build
GEN_DIR=./gen

OUT_COMP_INC=$(OUT_INC)/$(COMP_NAME)
OUT_COMP_GEN=$(OUT_MISC)/$(COMP_NAME)
OUT_COMP_SLO=$(OUT_SLO)/$(COMP_NAME)

CXXFILES = efac.cpp services.cpp element.cpp model.cpp view.cpp row.cpp listeners.cpp config.cpp tools.cpp 
OBJFILES = $(patsubst %.cpp,$(OUT_SLO)/%.$(OBJ_EXT),$(CXXFILES))

OPTIONS_CXXFILES=options.cpp config.cpp tools.cpp
OPTIONS_OBJFILES=$(patsubst %.cpp,$(OUT_SLO)/%.$(OBJ_EXT),$(OPTIONS_CXXFILES))

IDL_LOC_INC=./inc
CC_INCLUDES=-I. -I$(IDL_LOC_INC) -I$(OO_SDK_HOME)/include 

MANIFEST=$(BUILD_DIR)/META-INF/manifest.xml
DESCRIPTION=$(BUILD_DIR)/description.xml
COMP_DIR=$(BUILD_DIR)/libs
COMP_REGISTRATION=$(COMP_DIR)/registration.components

UNO_PKG_NAME=.$(PS)files$(PS)$(EXTENSION_NAME)-$(EXTENSION_VERSION)-$(subst _,-,$(EXTENSION_PLATFORM)).$(UNOOXT_EXT)

#CC=ccache gcc

.PHONY: ALL
ALL : CppWatchingWindow

CppWatchingWindow : $(UNO_PKG_NAME)

include $(SETTINGS)/stdtarget.mk

PAKCAGE_CONTENTS=META-INF/* description.xml README LICENSE libs/* descriptions/* *.xcs *.xcu dialogs/* icons/* iconsh/* resources/*


$(UNO_PKG_NAME) : $(COMP_DIR)/$(COMP_IMPL_NAME) $(COMP_DIR)/$(OPTIONS_COMP_IMPL_NAME) $(MANIFEST) $(COMP_REGISTRATION) 
	$(COPY) README $(subst /,$(PS),$(BUILD_DIR)/README)
	$(COPY) LICENSE $(subst /,$(PS),$(BUILD_DIR)/LICENSE)
	$(COPY) -r $(CONFIG_DIR)/* $(BUILD_DIR)
	$(COPY) -r $(GEN_DIR)/* $(BUILD_DIR)
	$(COPY) -r dialogs $(BUILD_DIR)
	$(COPY) -r icons $(BUILD_DIR)
	$(COPY) -r iconsh $(BUILD_DIR)
	cd $(BUILD_DIR) && $(SDK_ZIP) -9 -r -o ../$(UNO_PKG_NAME) $(PAKCAGE_CONTENTS)


$(OUT_SLO)/%.$(OBJ_EXT) : $(SRC)/%.cpp $(SDKTYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $(CC_FLAGS) $< $(CC_INCLUDES) $(CC_DEFINES) $(VERSION_DEF) $(TASKPANE_DEF)


ifeq "$(OS)" "WIN"
LINK_OUT_FLAG=/OUT:
MATH_LIB=-lm
ADDITIONAL_LIBS=msvcrt.lib kernel32.lib
else
LINK_OUT_FLAG=-o 
MATH_LIB=
ADDITIONAL_LIBS=-Wl,--as-needed -ldl -lpthread -lm -Wl,--no-as-needed -Wl,-Bdynamic
endif

# ToDo
LINK_LIBS=-L $(OFFICE_BASE_PROGRAM_PATH) -L $(PRJ)$(PS)lib

$(COMP_DIR)/$(COMP_IMPL_NAME) : $(OBJFILES)
	-$(MKDIR) $(subst /,$(PS),$(COMP_DIR))
	$(LINK) $(COMP_LINK_FLAGS) $(LINK_OUT_FLAG)$(COMP_DIR)/$(COMP_IMPL_NAME) $(OBJFILES) $(COJBFILES) $(LINK_LIBS) $(MATH_LIB) $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STC++LIB) $(CPPUHELPERDYLIB) $(CPPUDYLIB) $(SALDYLIB) $(SALHELPERLIB) $(ADDITIONAL_LIBS)

$(COMP_DIR)/$(OPTIONS_COMP_IMPL_NAME) : $(OPTIONS_OBJFILES)
	-$(MKDIR) $(subst /,$(PS),$(COMP_DIR))
	$(LINK) $(COMP_LINK_FLAGS) $(LINK_OUT_FLAG)$(COMP_DIR)/$(OPTIONS_COMP_IMPL_NAME) $(OPTIONS_OBJFILES) $(COJBFILES) $(LINK_LIBS) $(MATH_LIB) $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STC++LIB) $(CPPUHELPERDYLIB) $(CPPUDYLIB) $(SALDYLIB) $(SALHELPERLIB) $(ADDITIONAL_LIBS)


$(MANIFEST) : 
	@-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) > $@
	@echo $(OSEP)manifest:manifest$(CSEP) >> $@
	@echo $(OSEP)manifest:file-entry manifest:full-path="$(QM)libs/registration.components$(QM)"  >> $@
	@echo manifest:media-type="$(QM)application/vnd.sun.star.uno-components;platform=$(UNOPKG_PLATFORM)$(QM)"/$(CSEP)  >> $@
ifeq "$(IN_TASKPANE)" "true"
	@echo $(OSEP)manifest:file-entry manifest:full-path="$(QM)CalcWindowState.xcu$(QM)" >> $@
else
	@echo $(OSEP)manifest:file-entry manifest:full-path="$(QM)Sidebar.xcu$(QM)" >> $@
endif
	@echo manifest:media-type="$(QM)application/vnd.sun.star.configuration-data$(QM)"/$(CSEP) >> $@
	@echo $(OSEP)manifest:file-entry manifest:full-path="$(QM)Factories.xcu$(QM)" >> $@
	@echo manifest:media-type="$(QM)application/vnd.sun.star.configuration-data$(QM)"/$(CSEP) >> $@
	@echo $(OSEP)manifest:file-entry manifest:full-path="$(QM)OptionsDialog.xcu$(QM)" >> $@
	@echo manifest:media-type="$(QM)application/vnd.sun.star.configuration-data$(QM)"/$(CSEP) >> $@
	@echo $(OSEP)manifest:file-entry manifest:full-path="$(QM)WatchingWindow.xcs$(QM)" >> $@
	@echo manifest:media-type="$(QM)application/vnd.sun.star.configuration-schema$(QM)"/$(CSEP) >> $@
	@echo $(OSEP)manifest:file-entry manifest:full-path="$(QM)WatchingWindow.xcu$(QM)" >> $@
	@echo manifest:media-type="$(QM)application/vnd.sun.star.configuration-data$(QM)"/$(CSEP) >> $@
	@echo $(OSEP)/manifest:manifest$(CSEP) >> $@

$(COMP_REGISTRATION) : 
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) >> $@
	@echo $(OSEP)components xmlns="$(QM)http://openoffice.org/2010/uno-components$(QM)"$(CSEP) >> $@
	@echo $(OSEP)component loader="$(QM)com.sun.star.loader.SharedLibrary$(QM)" uri="$(QM)$(COMP_IMPL_NAME)$(QM)"$(CSEP) >> $@
	@echo $(OSEP)implementation name="$(QM)$(IMPLE_NAME)$(QM)"$(CSEP) >> $@
	@echo $(OSEP)service name="$(QM)$(IMPLE_NAME)$(QM)"/$(CSEP) >> $@
	@echo $(OSEP)/implementation$(CSEP) >> $@
	@echo $(OSEP)/component$(CSEP) >> $@
	@echo $(OSEP)component loader="$(QM)com.sun.star.loader.SharedLibrary$(QM)" uri="$(QM)$(OPTIONS_COMP_IMPL_NAME)$(QM)"$(CSEP) >> $@
	@echo $(OSEP)implementation name="$(QM)$(OPTIONS_IMPLE_NAME)$(QM)"$(CSEP) >> $@
	@echo $(OSEP)service name="$(QM)$(OPTIONS_IMPLE_NAME)$(QM)"/$(CSEP) >> $@
	@echo $(OSEP)/implementation$(CSEP) >> $@
	@echo $(OSEP)/component$(CSEP) >> $@
	@echo $(OSEP)/components$(CSEP) >> $@

clean : 
	@- $(RM) $(subst \\,\,$(subst /,$(PS),$(MANIFEST)))
	@- $(RM) $(subst \\,\,$(subst /,$(PS),$(DESCRIPTION)))
	@- $(DELRECURSIVE) $(subst \,$(PS),$(OUT_SLO))
	@- $(RM) $(subst \\,\,$(subst /,$(PS),$(COMP_DIR)/$(COMP_IMPL_NAME)))
	@- $(RM) $(subst \\,\,$(subst /,$(PS),$(COMP_DIR)/$(OPTIONS_COMP_IMPL_NAME)))
	@- $(RM) $(subst /,$(PS),$(BUILD_DIR)/LICENSE)
	@- $(RM) $(subst /,$(PS),$(BUILD_DIR)/README)
	@- $(RM) $(UNO_PKG_NAME)
#	@- $(DELRECURSIVE) $(BUILD_DIR)

