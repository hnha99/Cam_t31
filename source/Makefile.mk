CFLAGS  += -I$(PROJECT_DIR)/source
VPATH += $(PROJECT_DIR)/source

-include $(PROJECT_DIR)/source/video/Makefile.mk

OBJS += $(OBJSDIR)/main.o