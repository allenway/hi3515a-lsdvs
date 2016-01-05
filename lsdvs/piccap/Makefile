#!/bin/sh
##################################################
# author : sven
# date   : 2013-09-30
##################################################

#local dir
INC_PATH	:=  -I$(COMMON_PATH) -I$(PUBLIC_PATH) -I$(MPI_PATH) -I$(PARAM_PATH) -I$(PROCON_PATH) \
				-I$(HDD_PATH) -I$(IPC_PATH) -I$(FIT_PATH)
CPPFLAGS	:=	$(CPPFLAGS_V) $(INC_PATH) 
SRCS		:=	$(wildcard *.cpp)
OBJS		:=	$(SRCS:%.cpp=%.o)
SRCSC		:=	$(wildcard *.c)
OBJSC		:=	$(SRCSC:%.c=%.o)

LOCAL_TARGET := $(notdir $(SUB_PATH)).a
$(LOCAL_TARGET): $(OBJS) $(OBJSC)
	$(AR) $(ARFLAGS) $(LOCAL_TARGET) $(OBJS)

%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) -o $@ $< 
%.o: %.c
	$(CC) -c $(CPPFLAGS) -o $@ $< 	
	
.PHONY: clean dep

clean:
	rm -f $(OBJS) $(LOCAL_TARGET) *.dep
	
dep:
	(for i in *.cpp;do echo -n "    ";$(CPP) -MM $(INC_PATH) $$i;done) > Makefile.dep
	
### Dependencies:
ifeq (Makefile.dep, $(wildcard Makefile.dep))
include Makefile.dep
endif
