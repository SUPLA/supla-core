################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/suplasinglecall.cpp 

C_SRCS += \
../src/cfg.c \
../src/eh.c \
../src/ini.c \
../src/lck.c \
../src/log.c \
../src/proto.c \
../src/safearray.c \
../src/srpc.c \
../src/sthread.c \
../src/supla-client.c \
../src/supla-socket.c \
../src/tools.c 

CPP_DEPS += \
./src/suplasinglecall.d 

C_DEPS += \
./src/cfg.d \
./src/eh.d \
./src/ini.d \
./src/lck.d \
./src/log.d \
./src/proto.d \
./src/safearray.d \
./src/srpc.d \
./src/sthread.d \
./src/supla-client.d \
./src/supla-socket.d \
./src/tools.d 

OBJS += \
./src/cfg.o \
./src/eh.o \
./src/ini.o \
./src/lck.o \
./src/log.o \
./src/proto.o \
./src/safearray.o \
./src/srpc.o \
./src/sthread.o \
./src/supla-client.o \
./src/supla-socket.o \
./src/suplasinglecall.o \
./src/tools.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	$(GCC) $(PARAMS) -DNOMYSQL=1 -O3 -Wall -fsigned-char  -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.cpp src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ $(PARAMS) -std=c++0x -DNOMYSQL=1 -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/cfg.d ./src/cfg.o ./src/eh.d ./src/eh.o ./src/ini.d ./src/ini.o ./src/lck.d ./src/lck.o ./src/log.d ./src/log.o ./src/proto.d ./src/proto.o ./src/safearray.d ./src/safearray.o ./src/srpc.d ./src/srpc.o ./src/sthread.d ./src/sthread.o ./src/supla-client.d ./src/supla-client.o ./src/supla-socket.d ./src/supla-socket.o ./src/suplasinglecall.d ./src/suplasinglecall.o ./src/tools.d ./src/tools.o

.PHONY: clean-src

