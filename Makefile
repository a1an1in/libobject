#
#=====================================================================================
#  Filename:    Makefile
#  Description: This is top Makefile, it uses with src dir' Makefiles and mk's Makefile
#         	functions.
#  Usage:       Add the code dir that you want to compile to TARGET_DIRS value below,
#               then open an shell and change dir to top Makefile's dir and key make.
#  Version:     2.0
#  Created:     11/05/2015 11:27:03 PM
#  Author:      alan lin , a1an1in@sina.com
#  Company:     vigor
#=====================================================================================
#
#TARGET file name
TARGET = test
SHARE_TARGET = libobject.so
STATIC_TARGET = libobject.a

#compile tools
export CC = gcc
export LD = ld
export MACROS+=$(MACRO)
export PROJECT_PATH = $(shell pwd)
export LIBS = -lpthread -lm -lrt -lSDL2 -lSDL2_ttf -L /usr/local/lib -lcutils
export INCLUDES = -I /usr/local/include/libcutils
export INCLUDES += -I $(PROJECT_PATH)/src/include

#include makefile funtions
include ./mk/Makefile.funcs

#there can't be blank space after '\' character.
all :
	@$(call build_obj);\
	$(call check_output_dir);\
	$(CC) -o $(PROJECT_PATH)/bin/$(TARGET) $$BUILD_IN_FILES $$LIBS;\
	if [ $$? -ne 0 ]; then \
		echo "Building $(TARGET) is failed !"; \
		exit 12; \
	else\
		echo $(CC) -o $(PROJECT_PATH)/bin/$(TARGET) $$BUILD_IN_FILES $$LIBS ;\
		echo ;\
		echo $(TARGET) has been created!;\
		echo ;\
	fi;
clean:
	$(call make_clean)
	rm -rf bin/$(TARGET)
cp:
	make
	cp -rf bin/$(TARGET) /var/ftp
so:
	@$(call build_obj);\
	$(call check_output_dir);\
	$(CC) -shared -o $(PROJECT_PATH)/bin/$(SHARE_TARGET) $$BUILD_IN_FILES $$LIBS ;\
	if [ $$? -ne 0 ]; then \
		echo "Building $(SHARE_TARGET) is failed !"; \
		exit 12; \
	else\
		echo $(CC) -shared -o $(PROJECT_PATH)/bin/$(SHARE_TARGET) $$BUILD_IN_FILES $$LIBS;\
		echo ;\
		echo $(TARGET) has been created!;\
		echo ;\
	fi;
ar:
	@$(call build_obj);\
	$(call check_output_dir);\
	$(AR) cqs  $(PROJECT_PATH)/bin/$(STATIC_TARGET) $$BUILD_IN_FILES;\
	if [ $$? -ne 0 ]; then \
		echo "Building $(STATIC_TARGET) is failed !"; \
		exit 12; \
	else\
		echo $(AR) cqs  $(PROJECT_PATH)/bin/$(STATIC_TARGET) $$BUILD_IN_FILES;\
		echo ;\
		echo $(TARGET) has been created!;\
		echo ;\
	fi;
install:
	cp -rf ./src/include /usr/local/include/libobject
	cp -rf $(PROJECT_PATH)/bin/$(SHARE_TARGET) /usr/local/lib

