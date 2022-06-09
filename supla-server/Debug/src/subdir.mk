################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/abstract_srpc_adapter.cpp \
../src/accept_loop.cpp \
../src/caller.cpp \
../src/cdbase.cpp \
../src/cdcontainer.cpp \
../src/database.cpp \
../src/datalogger.cpp \
../src/dbcommon.cpp \
../src/dcpair.cpp \
../src/ipcctrl.cpp \
../src/objcontainer.cpp \
../src/objcontaineritem.cpp \
../src/serverconnection.cpp \
../src/serverstatus.cpp \
../src/srpc_adapter.cpp \
../src/supla-server.cpp \
../src/svrdb.cpp \
../src/voiceassistantclient.cpp 

C_SRCS += \
../src/cfg.c \
../src/eh.c \
../src/ini.c \
../src/ipcsocket.c \
../src/lck.c \
../src/log.c \
../src/proto.c \
../src/safearray.c \
../src/srpc.c \
../src/sslcrypto.c \
../src/sthread.c \
../src/supla-socket.c \
../src/svrcfg.c \
../src/tools.c 

CPP_DEPS += \
./src/abstract_srpc_adapter.d \
./src/accept_loop.d \
./src/caller.d \
./src/cdbase.d \
./src/cdcontainer.d \
./src/database.d \
./src/datalogger.d \
./src/dbcommon.d \
./src/dcpair.d \
./src/ipcctrl.d \
./src/objcontainer.d \
./src/objcontaineritem.d \
./src/serverconnection.d \
./src/serverstatus.d \
./src/srpc_adapter.d \
./src/supla-server.d \
./src/svrdb.d \
./src/voiceassistantclient.d 

C_DEPS += \
./src/cfg.d \
./src/eh.d \
./src/ini.d \
./src/ipcsocket.d \
./src/lck.d \
./src/log.d \
./src/proto.d \
./src/safearray.d \
./src/srpc.d \
./src/sslcrypto.d \
./src/sthread.d \
./src/supla-socket.d \
./src/svrcfg.d \
./src/tools.d 

OBJS += \
./src/abstract_srpc_adapter.o \
./src/accept_loop.o \
./src/caller.o \
./src/cdbase.o \
./src/cdcontainer.o \
./src/cfg.o \
./src/database.o \
./src/datalogger.o \
./src/dbcommon.o \
./src/dcpair.o \
./src/eh.o \
./src/ini.o \
./src/ipcctrl.o \
./src/ipcsocket.o \
./src/lck.o \
./src/log.o \
./src/objcontainer.o \
./src/objcontaineritem.o \
./src/proto.o \
./src/safearray.o \
./src/serverconnection.o \
./src/serverstatus.o \
./src/srpc.o \
./src/srpc_adapter.o \
./src/sslcrypto.o \
./src/sthread.o \
./src/supla-server.o \
./src/supla-socket.o \
./src/svrcfg.o \
./src/svrdb.o \
./src/tools.o \
./src/voiceassistantclient.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	$(CC) -D__OPENSSL_TOOLS=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__DEBUG=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(SSLDIR)/include -I../src/mqtt -I../src/user -I../src/device -I../src/client -I../src -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all  -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/abstract_srpc_adapter.d ./src/abstract_srpc_adapter.o ./src/accept_loop.d ./src/accept_loop.o ./src/caller.d ./src/caller.o ./src/cdbase.d ./src/cdbase.o ./src/cdcontainer.d ./src/cdcontainer.o ./src/cfg.d ./src/cfg.o ./src/database.d ./src/database.o ./src/datalogger.d ./src/datalogger.o ./src/dbcommon.d ./src/dbcommon.o ./src/dcpair.d ./src/dcpair.o ./src/eh.d ./src/eh.o ./src/ini.d ./src/ini.o ./src/ipcctrl.d ./src/ipcctrl.o ./src/ipcsocket.d ./src/ipcsocket.o ./src/lck.d ./src/lck.o ./src/log.d ./src/log.o ./src/objcontainer.d ./src/objcontainer.o ./src/objcontaineritem.d ./src/objcontaineritem.o ./src/proto.d ./src/proto.o ./src/safearray.d ./src/safearray.o ./src/serverconnection.d ./src/serverconnection.o ./src/serverstatus.d ./src/serverstatus.o ./src/srpc.d ./src/srpc.o ./src/srpc_adapter.d ./src/srpc_adapter.o ./src/sslcrypto.d ./src/sslcrypto.o ./src/sthread.d ./src/sthread.o ./src/supla-server.d ./src/supla-server.o ./src/supla-socket.d ./src/supla-socket.o ./src/svrcfg.d ./src/svrcfg.o ./src/svrdb.d ./src/svrdb.o ./src/tools.d ./src/tools.o ./src/voiceassistantclient.d ./src/voiceassistantclient.o

.PHONY: clean-src

