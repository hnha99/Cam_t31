.PHONY: all, clean ,create

TARGET= app

CROSSCOMPILE = mips-linux-gnu-
PROJECT_DIR  = $(PWD)
OBJSDIR=  $(PROJECT_DIR)/build
CC   = 	  $(CROSSCOMPILE)gcc  
CXX  = 	  $(CROSSCOMPILE)g++ 

# add color
RED?= "\033[0;31;1m"
GREEN?= "\033[0;32;3m"
BLUE?= "\033[3;36m"
YELLOW?= "033[0;33;1m"
NONE?="\033[0m"
GOTOY?= "\033[%dC"

CFLAGS += -Wall -g -pthread -lrt -lm -ldl

include $(PROJECT_DIR)/source/Makefile.mk

CFLAGS +=-I$(PROJECT_DIR)/dependence/include/t31/imp

LALIBS += $(PROJECT_DIR)/dependence/lib/uclibc/libimp.a
LALIBS += $(PROJECT_DIR)/dependence/lib/uclibc/libsysutils.a
LALIBS += $(PROJECT_DIR)/dependence/lib/uclibc/libalog.a

LDFLAGS =

all: create $(OBJSDIR)/$(TARGET)

create:
	@mkdir -p $(OBJSDIR)

$(OBJSDIR)/$(TARGET) : $(OBJS)
	@echo ---------- BUILD PROJECT ----------
	@$(CC) $^ -o $@ $(CFLAGS) $(LALIBS)
	@echo $(GREEN)"--Compiling executable file '$(OBJSDIR)/$(TARGET)'"$(NONE) $(BLUE)"OK"$(NONE)

$(OBJSDIR)/%.o:%.c $(HDRS)
	@$(CC) -c $< -o $@ $(CFLAGS) 
	@echo $(GREEN)"--Compiling '$<'--"$(NONE) $(BLUE)"OK"$(NONE)


$(OBJSDIR)/%.o:%.cpp $(HDRS)
	@$(CXX) -c $< -o $@ $(CFLAGS) 
	@echo $(GREEN)"--Compiling '$<'--"$(NONE) $(BLUE)"OK"$(NONE)


clean: 
	rm -rf $(OBJSDIR)
	@echo $(GREEN)"--Remove '$(OBJSDIR)'--"$(NONE) $(BLUE)"OK"$(NONE)
	@echo ""

	