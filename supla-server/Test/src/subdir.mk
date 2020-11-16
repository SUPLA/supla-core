################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
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

CPP_SRCS += \
../src/accept_loop.cpp \
../src/cdbase.cpp \
../src/cdcontainer.cpp \
../src/database.cpp \
../src/datalogger.cpp \
../src/db.cpp \
../src/dcpair.cpp \
../src/ipcctrl.cpp \
../src/objcontainer.cpp \
../src/objcontaineritem.cpp \
../src/serverconnection.cpp \
../src/voiceassistantclient.cpp 

OBJS += \
./src/accept_loop.o \
./src/cdbase.o \
./src/cdcontainer.o \
./src/cfg.o \
./src/database.o \
./src/datalogger.o \
./src/db.o \
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
./src/srpc.o \
./src/sslcrypto.o \
./src/sthread.o \
./src/supla-socket.o \
./src/svrcfg.o \
./src/tools.o \
./src/voiceassistantclient.o 

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

CPP_DEPS += \
./src/accept_loop.d \
./src/cdbase.d \
./src/cdcontainer.d \
./src/database.d \
./src/datalogger.d \
./src/db.d \
./src/dcpair.d \
./src/ipcctrl.d \
./src/objcontainer.d \
./src/objcontaineritem.d \
./src/serverconnection.d \
./src/voiceassistantclient.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DSERVER_VERSION_23 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -D__DEBUG=1 -DSERVER_VERSION_23 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I$(SSLDIR)/include -I../src/client -I../src/user -I../src/device -I../src -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all  -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


