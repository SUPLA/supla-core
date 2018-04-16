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
../src/sthread.c \
../src/supla-socket.c \
../src/svrcfg.c \
../src/tools.c 

CPP_SRCS += \
../src/accept_loop.cpp \
../src/cdcommon.cpp \
../src/database.cpp \
../src/datalogger.cpp \
../src/db.cpp \
../src/ipcctrl.cpp \
../src/objcontainer.cpp \
../src/objcontaineritem.cpp \
../src/serverconnection.cpp \
../src/supla-server.cpp 

OBJS += \
./src/accept_loop.o \
./src/cdcommon.o \
./src/cfg.o \
./src/database.o \
./src/datalogger.o \
./src/db.o \
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
./src/sthread.o \
./src/supla-server.o \
./src/supla-socket.o \
./src/svrcfg.o \
./src/tools.o 

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
./src/sthread.d \
./src/supla-socket.d \
./src/svrcfg.d \
./src/tools.d 

CPP_DEPS += \
./src/accept_loop.d \
./src/cdcommon.d \
./src/database.d \
./src/datalogger.d \
./src/db.d \
./src/ipcctrl.d \
./src/objcontainer.d \
./src/objcontaineritem.d \
./src/serverconnection.d \
./src/supla-server.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -I/usr/include/mysql -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O3 -Wall -c -fmessage-length=0 -fstack-protector-all -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -D__BCRYPT=1 -I$(SSLDIR)/include -I../src -I../src/user -I../src/device -I../src/client -O3 -Wall -c -fmessage-length=0 -fstack-protector-all -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


