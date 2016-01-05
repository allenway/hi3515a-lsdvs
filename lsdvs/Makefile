#!/bin/sh
##################################################
# author : sven
# date   : 2013-09-30
##################################################

#TARGET_VERSION = GDB

ifdef MA3359
CROSS	=  arm-arago-linux-gnueabi-
FIT_MCU	=	-DMCU_MA3359
endif

ifdef HI3515
CROSS	=  arm-hismall-linux-
FIT_MCU	=	-DMCU_HI3515
endif

#start add HI3515A
#add at 20130916
ifdef HI3515A
CROSS = arm-hisiv100nptl-linux-
FIT_MCU	=	-DMCU_HI3515A
endif
#end add HI3515A

export CC		:=	$(CROSS)gcc
export CPP		:=	$(CROSS)g++
export AR		:=	$(CROSS)ar
export ARFLAGS  := 	rcs

# for hi3512 only
ifeq ($(CROSS),arm-hismall-linux-)
export SDK_DIR:= /hisi/hi3515sdk1080
export KERNEL_PATH:=$(SDK_DIR)/source/os/linux-2.6.24/include
export PUB_PATH:=$(SDK_DIR)/pub
export PUBINC_PATH:=$(SDK_DIR)/pub/include
export MPP_PATH:=$(SDK_DIR)/mpp
export MPPINC_PATH:=$(SDK_DIR)/mpp/include	
#export MPPINC_PATH:=$(PWD)/../mppinc	
export LIB_PATH:=$(MPP_PATH)/lib
export HI3515_LIBS := 	$(LIB_PATH)/libmpi.a \
				$(LIB_PATH)/lib_VoiceEngine.a \
			 	$(LIB_PATH)/lib_amr_spc.a \
				$(LIB_PATH)/lib_amr_fipop.a \
				$(LIB_PATH)/lib_aec.a \
				$(LIB_PATH)/lib_aacdec.a \
				$(LIB_PATH)/lib_aacenc.a \
				$(LIB_PATH)/libloadbmp.a \
				$(LIB_PATH)/libtde.a \
				$(LIB_PATH)/libresampler.a
EX_LIBS += $(HI3515_LIBS)
endif


#add for HI3515A
#add at 20130916
ifeq ($(CROSS),arm-hisiv100nptl-linux-)
export SDK_DIR:=/opt/work/Hi3515a/Hi3520D_SDK_V1.0.1.0
export KERNEL_PATH:=$(SDK_DIR)/source/os/linux-2.6.24/include
#export PUB_PATH:=$(SDK_DIR)/pub
export PUBINC_PATH:=$(SDK_DIR)/pub/include
export MPP_PATH:=$(SDK_DIR)/mpp
export MPPINC_PATH:=$(SDK_DIR)/mpp/include	
export LIB_PATH:=$(MPP_PATH)/lib
export HI3515A_LIBS := 	$(LIB_PATH)/libmpi.a \
				$(LIB_PATH)/libVoiceEngine.a \
				$(LIB_PATH)/libaec.a \
				$(LIB_PATH)/libtde.a \
				$(LIB_PATH)/libjpeg.a \
				$(LIB_PATH)/libhdmi.a \
				$(LIB_PATH)/libanr.a \
				$(LIB_PATH)/libresampler.a
EX_LIBS += $(HI3515A_LIBS)
endif
#end of HI3515A


ifeq ($(TARGET_VERSION),GDB)
	export	CPPFLAGS_V	=	$(FIT_MCU) -Wall -g 
	LINKFLAGS_V	=	-lstdc++ -lpthread -lm
else
	export	CPPFLAGS_V	=	$(FIT_MCU) -Wall -O2
	LINKFLAGS_V	=	-lstdc++ -lpthread -lm -s
endif

#extern libs
ELIBS_PATH			:=	$(PWD)/libs
#ELIBS				:=	$(wildcard $(ELIBS_PATH)/*.a)

#final target
export TARGET = rhadvs
ALL: $(TARGET)

#common
export COMMON_PATH	:=	$(PWD)/../common
COMMON_SRCS			:=	$(wildcard $(COMMON_PATH)/*.cpp)
COMMON_INCS			:=	$(wildcard $(COMMON_PATH)/*.h)
COMMON				:=	$(COMMON_PATH)/$(notdir $(COMMON_PATH))$(TARGET).a
$(COMMON): $(COMMON_SRCS) $(COMMON_INCS)
	$(MAKE) -C $(COMMON_PATH) SUB_PATH=$(COMMON_PATH)

#public
export PUBLIC_PATH	:=	$(PWD)/../public
PUBLIC_SRCS			:=	$(wildcard $(PUBLIC_PATH)/*.cpp)
PUBLIC_INCS			:=	$(wildcard $(PUBLIC_PATH)/*.h)
PUBLIC				:=	$(PUBLIC_PATH)/$(notdir $(PUBLIC_PATH))$(TARGET).a
$(PUBLIC): $(PUBLIC_SRCS) $(PUBLIC_INCS)
	$(MAKE) -C $(PUBLIC_PATH) SUB_PATH=$(PUBLIC_PATH)
	
#test
export TEST_PATH	:=	$(PWD)/test
TEST_SRCS			:=	$(wildcard $(TEST_PATH)/*.cpp)
TEST_INCS			:=	$(wildcard $(TEST_PATH)/*.h)
TEST				:=	$(TEST_PATH)/$(notdir $(TEST_PATH)).a
$(TEST): $(TEST_SRCS) $(TEST_INCS)
	$(MAKE) -C $(TEST_PATH) SUB_PATH=$(TEST_PATH)
	
#main
MAIN_SRCS			:=	$(wildcard *.cpp)
MAIN_OBJS			:=	$(MAIN_SRCS:%.cpp=%.o)	
MAIN_SRCSC			:=	$(wildcard *.c)
MAIN_OBJSC			:=	$(MAIN_SRCSC:%.c=%.o)	

#procon
export PROCON_PATH	:=	$(PWD)/procon
PROCON_SRCS			:=	$(wildcard $(PROCON_PATH)/*.cpp)
PROCON_INCS			:=	$(wildcard $(PROCON_PATH)/*.h)
PROCON				:=	$(PROCON_PATH)/$(notdir $(PROCON_PATH)).a
$(PROCON): $(PROCON_SRCS) $(PROCON_INCS)
	$(MAKE) -C $(PROCON_PATH) SUB_PATH=$(PROCON_PATH)

#record
export RECORD_PATH	:=	$(PWD)/record
RECORD_SRCS			:=	$(wildcard $(RECORD_PATH)/*.cpp)
RECORD_INCS			:=	$(wildcard $(RECORD_PATH)/*.h)
RECORD				:=	$(RECORD_PATH)/$(notdir $(RECORD_PATH)).a
$(RECORD): $(RECORD_SRCS) $(RECORD_INCS)
	$(MAKE) -C $(RECORD_PATH) SUB_PATH=$(RECORD_PATH)
	
#hdd
export HDD_PATH		:=	$(PWD)/hdd
HDD_SRCS			:=	$(wildcard $(HDD_PATH)/*.cpp)
HDD_INCS			:=	$(wildcard $(HDD_PATH)/*.h)
HDD					:=	$(HDD_PATH)/$(notdir $(HDD_PATH)).a
$(HDD): $(HDD_SRCS) $(HDD_INCS)
	$(MAKE) -C $(HDD_PATH) SUB_PATH=$(HDD_PATH)

#param
export PARAM_PATH	:=	$(PWD)/param
PARAM_SRCS			:=	$(wildcard $(PARAM_PATH)/*.cpp)
PARAM_INCS			:=	$(wildcard $(PARAM_PATH)/*.h)
PARAM				:=	$(PARAM_PATH)/$(notdir $(PARAM_PATH)).a
$(PARAM): $(PARAM_SRCS) $(PARAM_INCS)
	$(MAKE) -C $(PARAM_PATH) SUB_PATH=$(PARAM_PATH)
	
#ntp
export NTP_PATH	:=	$(PWD)/ntp
NTP_SRCS			:=	$(wildcard $(NTP_PATH)/*.cpp)
NTP_INCS			:=	$(wildcard $(NTP_PATH)/*.h)
NTP					:=	$(NTP_PATH)/$(notdir $(NTP_PATH)).a
$(NTP): $(NTP_SRCS) $(NTP_INCS)
	$(MAKE) -C $(NTP_PATH) SUB_PATH=$(NTP_PATH)	

#email
export EMAIL_PATH	:=	$(PWD)/email
EMAIL_SRCS			:=	$(wildcard $(EMAIL_PATH)/*.cpp)
EMAIL_INCS			:=	$(wildcard $(EMAIL_PATH)/*.h)
EMAIL				:=	$(EMAIL_PATH)/$(notdir $(EMAIL_PATH)).a
$(EMAIL): $(EMAIL_SRCS) $(EMAIL_INCS)
	$(MAKE) -C $(EMAIL_PATH) SUB_PATH=$(EMAIL_PATH)	

#alarm
export ALARM_PATH	:=	$(PWD)/alarm
ALARM_SRCS			:=	$(wildcard $(ALARM_PATH)/*.cpp)
ALARM_INCS			:=	$(wildcard $(ALARM_PATH)/*.h)
ALARM				:=	$(ALARM_PATH)/$(notdir $(ALARM_PATH)).a
$(ALARM): $(ALARM_SRCS) $(ALARM_INCS)
	$(MAKE) -C $(ALARM_PATH) SUB_PATH=$(ALARM_PATH)	

#serial
export SERIAL_PATH	:=	$(PWD)/serial
SERIAL_SRCS			:=	$(wildcard $(SERIAL_PATH)/*.cpp)
SERIAL_INCS			:=	$(wildcard $(SERIAL_PATH)/*.h)
SERIAL				:=	$(SERIAL_PATH)/$(notdir $(SERIAL_PATH)).a
$(SERIAL): $(SERIAL_SRCS) $(SERIAL_INCS)
	$(MAKE) -C $(SERIAL_PATH) SUB_PATH=$(SERIAL_PATH)	

#threeg
export THREEG_PATH	:=	$(PWD)/threeg
THREEG_SRCS			:=	$(wildcard $(THREEG_PATH)/*.cpp)
THREEG_INCS			:=	$(wildcard $(THREEG_PATH)/*.h)
THREEG				:=	$(THREEG_PATH)/$(notdir $(THREEG_PATH)).a
$(THREEG): $(THREEG_SRCS) $(THREEG_INCS)
	$(MAKE) -C $(THREEG_PATH) SUB_PATH=$(THREEG_PATH)

#wifi
export WIFI_PATH	:=	$(PWD)/wifi
WIFI_SRCS			:=	$(wildcard $(WIFI_PATH)/*.cpp)
WIFI_INCS			:=	$(wildcard $(WIFI_PATH)/*.h)
WIFI				:=	$(WIFI_PATH)/$(notdir $(WIFI_PATH)).a
$(WIFI): $(WIFI_SRCS) $(WIFI_INCS)
	$(MAKE) -C $(WIFI_PATH) SUB_PATH=$(WIFI_PATH)

#ftp
export FTP_PATH	:=	$(PWD)/ftp
FTP_SRCS			:=	$(wildcard $(FTP_PATH)/*.cpp)
FTP_INCS			:=	$(wildcard $(FTP_PATH)/*.h)
FTP					:=	$(FTP_PATH)/$(notdir $(FTP_PATH)).a
$(FTP): $(FTP_SRCS) $(FTP_INCS)
	$(MAKE) -C $(FTP_PATH) SUB_PATH=$(FTP_PATH)

#ptz
export PTZ_PATH	:=	$(PWD)/ptz
PTZ_SRCS			:=	$(wildcard $(PTZ_PATH)/*.cpp)
PTZ_INCS			:=	$(wildcard $(PTZ_PATH)/*.h)
PTZ				:=	$(PTZ_PATH)/$(notdir $(PTZ_PATH)).a
$(PTZ): $(PTZ_SRCS) $(PTZ_INCS)
	$(MAKE) -C $(PTZ_PATH) SUB_PATH=$(PTZ_PATH)

#norflash
export NORFLASH_PATH	:=	$(PWD)/flash
NORFLASH_SRCS			:=	$(wildcard $(NORFLASH_PATH)/*.cpp)
NORFLASH_INCS			:=	$(wildcard $(NORFLASH_PATH)/*.h)
NORFLASH				:=	$(NORFLASH_PATH)/$(notdir $(NORFLASH_PATH)).a
$(NORFLASH):$(NORFLASH_SRCS) $(NORFLASH_INCS)
	$(MAKE) -C $(NORFLASH_PATH) SUB_PATH=$(NORFLASH_PATH)

#update
export UPDATE_PATH	:=	$(PWD)/update
UPDATE_SRCS			:=	$(wildcard $(UPDATE_PATH)/*.cpp)
UPDATE_INCS			:=	$(wildcard $(UPDATE_PATH)/*.h)
UPDATE				:=	$(UPDATE_PATH)/$(notdir $(UPDATE_PATH)).a
$(UPDATE): $(UPDATE_SRCS) $(UPDATE_INCS)
	$(MAKE) -C $(UPDATE_PATH) SUB_PATH=$(UPDATE_PATH)

#net
export NET_PATH	:=	$(PWD)/net
NET_SRCS			:=	$(wildcard $(NET_PATH)/*.cpp)
NET_INCS			:=	$(wildcard $(NET_PATH)/*.h)
NET				:=	$(NET_PATH)/$(notdir $(NET_PATH)).a
$(NET): $(NET_SRCS) $(NET_INCS)
	$(MAKE) -C $(NET_PATH) SUB_PATH=$(NET_PATH)

#lspp
export LSPP_PATH	:=	$(PWD)/lspp
LSPP_SRCS			:=	$(wildcard $(LSPP_PATH)/*.cpp)
LSPP_INCS			:=	$(wildcard $(LSPP_PATH)/*.h)
LSPP				:=	$(LSPP_PATH)/$(notdir $(LSPP_PATH)).a
$(LSPP): $(LSPP_SRCS) $(LSPP_INCS)
	$(MAKE) -C $(LSPP_PATH) SUB_PATH=$(LSPP_PATH)

#mpi
export MPI_PATH	:=	$(PWD)/mpi
MPI_SRCS			:=	$(wildcard $(MPI_PATH)/*.cpp)
MPI_SRCSC			:=	$(wildcard $(MPI_PATH)/*.c)
MPI_INCS			:=	$(wildcard $(MPI_PATH)/*.h)
MPI				:=	$(MPI_PATH)/$(notdir $(MPI_PATH)).a
$(MPI): $(MPI_SRCS) $(MPI_SRCSC) $(MPI_INCS)
	$(MAKE) -C $(MPI_PATH) SUB_PATH=$(MPI_PATH)

#datetime
export DATETIME_PATH	:=	$(PWD)/datetime
DATETIME_SRCS			:=	$(wildcard $(DATETIME_PATH)/*.cpp)
DATETIME_INCS			:=	$(wildcard $(DATETIME_PATH)/*.h)
DATETIME				:=	$(DATETIME_PATH)/$(notdir $(DATETIME_PATH)).a
$(DATETIME): $(DATETIME_SRCS) $(DATETIME_INCS)
	$(MAKE) -C $(DATETIME_PATH) SUB_PATH=$(DATETIME_PATH)

#system
export SYSTEM_PATH	:=	$(PWD)/system
SYSTEM_SRCS			:=	$(wildcard $(SYSTEM_PATH)/*.cpp)
SYSTEM_INCS			:=	$(wildcard $(SYSTEM_PATH)/*.h)
SYSTEM				:=	$(SYSTEM_PATH)/$(notdir $(SYSTEM_PATH)).a
$(SYSTEM): $(SYSTEM_SRCS) $(SYSTEM_INCS)
	$(MAKE) -C $(SYSTEM_PATH) SUB_PATH=$(SYSTEM_PATH)

#piccap
export PICCAP_PATH	:=	$(PWD)/piccap
PICCAP_SRCS			:=	$(wildcard $(PICCAP_PATH)/*.cpp)
PICCAP_INCS			:=	$(wildcard $(PICCAP_PATH)/*.h)
PICCAP				:=	$(PICCAP_PATH)/$(notdir $(PICCAP_PATH)).a
$(PICCAP): $(PICCAP_SRCS) $(PICCAP_INCS)
	$(MAKE) -C $(PICCAP_PATH) SUB_PATH=$(PICCAP_PATH)

#icmp
export ICMP_PATH	:=	$(PWD)/icmp
ICMP_SRCS			:=	$(wildcard $(ICMP_PATH)/*.cpp)
ICMP_INCS			:=	$(wildcard $(ICMP_PATH)/*.h)
ICMP				:=	$(ICMP_PATH)/$(notdir $(ICMP_PATH)).a
$(ICMP): $(ICMP_SRCS) $(ICMP_INCS)
	$(MAKE) -C $(ICMP_PATH) SUB_PATH=$(ICMP_PATH)
	
#ddns
export DDNS_PATH	:=	$(PWD)/ddns
DDNS_SRCS			:=	$(wildcard $(DDNS_PATH)/*.cpp)
DDNS_INCS			:=	$(wildcard $(DDNS_PATH)/*.h)
DDNS				:=	$(DDNS_PATH)/$(notdir $(DDNS_PATH)).a
$(DDNS): $(DDNS_SRCS) $(DDNS_INCS)
	$(MAKE) -C $(DDNS_PATH) SUB_PATH=$(DDNS_PATH)	
	
#dtu
export DTU_PATH	:=	$(PWD)/dtu
DTU_SRCS		:=	$(wildcard $(DTU_PATH)/*.cpp)
DTU_INCS		:=	$(wildcard $(DTU_PATH)/*.h)
DTU				:=	$(DTU_PATH)/$(notdir $(DTU_PATH)).a
$(DTU): $(DTU_SRCS) $(DTU_INCS)
	$(MAKE) -C $(DTU_PATH) SUB_PATH=$(DTU_PATH)	

#ipc
export IPC_PATH	:=	$(PWD)/ipc
IPC_SRCS		:=	$(wildcard $(IPC_PATH)/*.cpp)
IPC_SRCSC		:=	$(wildcard $(IPC_PATH)/*.c)
IPC_INCS		:=	$(wildcard $(IPC_PATH)/*.h)
IPC				:=	$(IPC_PATH)/$(notdir $(IPC_PATH)).a
$(IPC): $(IPC_SRCS) $(IPC_INCS) $(IPC_SRCSC)
	$(MAKE) -C $(IPC_PATH) SUB_PATH=$(IPC_PATH)

#fit
export FIT_PATH	:=	$(PWD)/fit
FIT_SRCS		:=	$(wildcard $(FIT_PATH)/*.cpp)
FIT_INCS		:=	$(wildcard $(FIT_PATH)/*.h)
FIT				:=	$(FIT_PATH)/$(notdir $(FIT_PATH)).a
$(FIT): $(FIT_SRCS) $(FIT_INCS) $(FIT_SRCSC)
	$(MAKE) -C $(FIT_PATH) SUB_PATH=$(FIT_PATH)

#hyaline
export HYALINE_PATH	:=	$(PWD)/hyaline
HYALINE_SRCS		:=	$(wildcard $(HYALINE_PATH)/*.cpp)
HYALINE_INCS		:=	$(wildcard $(HYALINE_PATH)/*.h)
HYALINE				:=	$(HYALINE_PATH)/$(notdir $(HYALINE_PATH)).a
$(HYALINE): $(HYALINE_SRCS) $(HYALINE_INCS) $(HYALINE_SRCSC)
	$(MAKE) -C $(HYALINE_PATH) SUB_PATH=$(HYALINE_PATH)
	
#driver
export DRIVER_PATH	:=	$(PWD)/driver
DRIVER_SRCS		:=	$(wildcard $(DRIVER_PATH)/*.cpp)
DRIVER_INCS		:=	$(wildcard $(DRIVER_PATH)/*.h)
DRIVER				:=	$(DRIVER_PATH)/$(notdir $(DRIVER_PATH)).a
$(DRIVER): $(DRIVER_SRCS) $(DRIVER_INCS) $(DRIVER_SRCSC)
	$(MAKE) -C $(DRIVER_PATH) SUB_PATH=$(DRIVER_PATH)	

#bhdcp
export BHDCP_PATH	:=	$(PWD)/bhdcp
BHDCP_SRCS		:=	$(wildcard $(BHDCP_PATH)/*.cpp)
BHDCP_INCS		:=	$(wildcard $(BHDCP_PATH)/*.h)
BHDCP				:=	$(BHDCP_PATH)/$(notdir $(BHDCP_PATH)).a
$(BHDCP): $(BHDCP_SRCS) $(BHDCP_INCS) $(BHDCP_SRCSC)
	$(MAKE) -C $(BHDCP_PATH) SUB_PATH=$(BHDCP_PATH)
	
#log
export LOG_PATH	:=	$(PWD)/log
LOG_SRCS			:=	$(wildcard $(LOG_PATH)/*.cpp)
LOG_INCS			:=	$(wildcard $(LOG_PATH)/*.h)
LOG					:=	$(LOG_PATH)/$(notdir $(LOG_PATH)).a
$(LOG): $(LOG_SRCS) $(LOG_INCS)
	$(MAKE) -C $(LOG_PATH) SUB_PATH=$(LOG_PATH)	

#gpio
export GPIO_PATH	:=	$(PWD)/gpio
GPIO_SRCS		:=	$(wildcard $(GPIO_PATH)/*.cpp)
GPIO_INCS		:=	$(wildcard $(GPIO_PATH)/*.h)
GPIO				:=	$(GPIO_PATH)/$(notdir $(GPIO_PATH)).a
$(GPIO): $(GPIO_SRCS) $(GPIO_INCS) $(GPIO_SRCS)
	$(MAKE) -C $(GPIO_PATH) SUB_PATH=$(GPIO_PATH)

#broadcast
export BROADCAST_PATH	:=	$(PWD)/broadcast
BROADCAST_SRCS			:=	$(wildcard $(BROADCAST_PATH)/*.cpp)
BROADCAST_INCS			:=	$(wildcard $(BROADCAST_PATH)/*.h)
BROADCAST					:=	$(BROADCAST_PATH)/$(notdir $(BROADCAST_PATH)).a
$(BROADCAST): $(BROADCAST_SRCS) $(BROADCAST_INCS)
	$(MAKE) -C $(BROADCAST_PATH) SUB_PATH=$(BROADCAST_PATH)	

#wdog
export WDOG_PATH	:=	$(PWD)/wdog
WDOG_SRCS		:=	$(wildcard $(WDOG_PATH)/*.cpp)
WDOG_INCS		:=	$(wildcard $(WDOG_PATH)/*.h)
WDOG				:=	$(WDOG_PATH)/$(notdir $(WDOG_PATH)).a
$(WDOG): $(WDOG_SRCS) $(WDOG_INCS) $(WDOG_SRCS)
	$(MAKE) -C $(WDOG_PATH) SUB_PATH=$(WDOG_PATH)
	
#reboot
export REBOOT_PATH	:=	$(PWD)/reboot
REBOOT_SRCS		:=	$(wildcard $(REBOOT_PATH)/*.cpp)
REBOOT_INCS		:=	$(wildcard $(REBOOT_PATH)/*.h)
REBOOT				:=	$(REBOOT_PATH)/$(notdir $(REBOOT_PATH)).a
$(REBOOT): $(REBOOT_SRCS) $(REBOOT_INCS) $(REBOOT_SRCS)
	$(MAKE) -C $(REBOOT_PATH) SUB_PATH=$(REBOOT_PATH)
	
#rtsp
export RTSP_PATH	:=	$(PWD)/rtsp
RTSP_SRCS			:=	$(wildcard $(RTSP_PATH)/*.cpp)
RTSP_SRCSC			:=	$(wildcard $(RTSP_PATH)/*.c)
RTSP_INCS			:=	$(wildcard $(RTSP_PATH)/*.h)
RTSP				:=	$(RTSP_PATH)/$(notdir $(RTSP_PATH)).a
$(RTSP): $(RTSP_SRCS) $(RTSP_SRCSC) $(RTSP_INCS)
	$(MAKE) -C $(RTSP_PATH) SUB_PATH=$(RTSP_PATH)
	
##filling
#for make cleanall and dep
PATH_MOUDELS	+=  $(TEST_PATH) $(RECORD_PATH) $(HDD_PATH) $(PUBLIC_PATH) $(COMMON_PATH) \
					$(PROCON_PATH) $(PARAM_PATH) $(NTP_PATH) $(EMAIL_PATH) $(ALARM_PATH) \
					$(SERIAL_PATH) $(THREEG_PATH) $(WIFI_PATH) $(FTP_PATH) $(PTZ_PATH) \
					$(NORFLASH_PATH) $(UPDATE_PATH) $(NET_PATH) $(LSPP_PATH) $(MPI_PATH) \
					$(DATETIME_PATH) $(SYSTEM_PATH) $(PICCAP_PATH) $(ICMP_PATH) $(DDNS_PATH) \
					$(DTU_PATH)	$(IPC_PATH) $(FIT_PATH) $(HYALINE_PATH) $(DRIVER_PATH) \
					$(BHDCP_PATH) $(LOG_PATH) $(BROADCAST_PATH) $(GPIO_PATH) $(WDOG_PATH) \
					$(REBOOT_PATH) $(RTSP_PATH)
						
#for src dep this makefile
DEP_MOUDELS		+=	$(MAIN_SRCS) $(MAIN_OBJSC) $(COMMON_SRCS) $(PUBLIC_SRCS) $(TEST_SRCS) $(PROCON_SRCS) \
					$(RECORD_SRCS) $(HDD_SRCS) $(PARAM_SRC) $(NTP_SRC) $(EMAIL_SRC) \
					$(ALARM_SRCS) $(SERIAL_SRCS) $(THREEG_SRCS) $(WIFI_SRCS) $(FTP_SRCS) \
					$(PTZ_SRCS) $(NORFLASH_SRCS) $(UPDATE_SRCS) $(NET_SRCS) $(LSPP_SRCS) \
					$(MPI_SRCS) $(MPI_SRCSC) $(DATETIME_SRCS) $(SYSTEM_SRCS) $(PICCAP_SRCS) \
					$(ICMP_SRCS) $(DDNS_SRCS) $(DTU_SRCS) $(IPC_SRCS) $(IPC_SRCSC) $(FIT_SRCS) \
					$(HYALINE_SRCS) $(DRIVER_SRCS) $(BHDCP_SRCS) $(LOG_SRCS) $(BROADCAST_SRCS) \
					$(GPIO_SRCS) $(WDOG_SRCS) $(REBOOT_SRCS) $(RTSP_SRCS) $(RTSP_SRCSC)

#for this makefile include
INC_PATH		:=	-I$(TEST_PATH) -I$(PUBLIC_PATH) -I$(COMMON_PATH) \
					-I$(PROCON_PATH) -I$(RECORD_PATH) -I$(HDD_PATH) -I$(PARAM_PATH) \
					-I$(NTP_PATH) -I$(EMAIL_PATH) -I$(ALARM_PATH) -I$(SERIAL_PATH) \
					-I$(THREEG_PATH) -I$(WIFI_PATH) -I$(FTP_PATH) -I$(PTZ_PATH) \
					-I$(NORFLASH_PATH) -I$(UPDATE_PATH) -I$(NET_PATH) -I$(LSPP_PATH) \
					-I$(MPI_PATH) -I$(PUBINC_PATH) -I$(MPPINC_PATH) -I$(DATETIME_PATH) \
					-I$(SYSTEM_PATH) -I$(PICCAP_PATH) -I$(ICMP_PATH) -I$(DDNS_PATH) \
					-I$(DTU_PATH) -I$(IPC_PATH) -I$(FIT_PATH) -I$(HYALINE_PATH) -I$(DRIVER_PATH) \
					-I$(BHDCP_PATH) -I$(LOG_PATH) -I$(BROADCAST_PATH) -I$(GPIO_PATH) -I$(WDOG_PATH) \
                    -I$(REBOOT_PATH) -I$(RTSP_PATH)
##all libs
ifneq ($(CROSS),arm-hisiv100nptl-linux-)
MPI=
endif

FIRS_LIBS = $(TEST) $(GPIO) $(WDOG) $(BHDCP) $(RECORD) $(PICCAP) $(FIT) $(MPI) $(SYSTEM) \
			$(IPC) $(HYALINE) $(RTSP) \
			$(HDD) $(ALARM) $(PROCON) \
			$(SERIAL) $(THREEG) $(WIFI) $(FTP) $(PTZ) $(DRIVER)\
			$(BROADCAST) $(UPDATE) $(REBOOT) $(NET) $(NTP) $(DATETIME) $(ICMP) $(DDNS) $(DTU)	\
			$(EMAIL)  $(PARAM) $(NORFLASH) $(PUBLIC) $(COMMON) $(ELIBS) \
			$(LOG)

## this dir
CPPFLAGS		:=	$(CPPFLAGS_V)
CPPFLAGS		+=	$(INC_PATH)
LINKFLAGS		:=	$(LINKFLAGS_V)

$(TARGET): $(MAIN_OBJS) $(FIRS_LIBS) $(EX_LIBS)
#ifdef MA3359
#	cp ./procon/procon_3359.a ./procon/procon.a
#endif
#ifdef HI3515
#	cp ./procon/procon_3515.a ./procon/procon.a
#endif
	$(CPP) -o $(TARGET) $^ $(LINKFLAGS)
#	cp $(TARGET) /yfnfs

%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) -o $@ $< 
%.o: %.c
	$(CPP) -c $(CPPFLAGS) -o $@ $< 

.PHONY: clean cleanall dep 

clean:
	rm -fr $(MAIN_OBJS) $(TARGET) *.dep
	
cleanall: clean
	@for loop in $(PATH_MOUDELS);\
	do \
		$(MAKE) -C $$loop clean SUB_PATH=$$loop; \
	done

dep:
	@for loop in $(PATH_MOUDELS);\
	do \
		$(MAKE) -C $$loop dep; \
	done
	
	(for i in $(DEP_MOUDELS);do  dirname $$i|xargs echo -n;echo -n "/";$(CPP) -MM $(INC_PATH) $$i;done) > Makefile.dep

### Dependencies:
ifeq (Makefile.dep, $(wildcard Makefile.dep))
include Makefile.dep
endif
