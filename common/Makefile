#!/bin/sh
##################################################
# author : ÁºÇÅ½­
# date   : 2012-01-30
##################################################

#this dir
INC_PATH	:=
CPPFLAGS	:=	$(CPPFLAGS_V) $(INC_PATH)
SRCS		:=	$(wildcard *.cpp)
OBJS		:=	$(SRCS:%.cpp=%.o)

THIS_TARGET := $(notdir $(SUB_PATH))$(TARGET).a
$(THIS_TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $(THIS_TARGET) $^
	rm -f $(OBJS)

%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) -o $@ $< 
	
.PHONY: clean dep

clean:
	rm -f $(OBJS) $(THIS_TARGET) *.dep
	
dep:
	(for i in *.cpp;do echo -n "    ";$(CPP) -MM -I$(INC_PATH) -I./ $$i;done) > Makefile.dep
	
### Dependencies:
ifeq (Makefile.dep, $(wildcard Makefile.dep))
include Makefile.dep
endif
