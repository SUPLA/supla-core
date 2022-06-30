################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/ipc/IpcCommandTest.cpp \
../src/test/ipc/IpcCtrlTest.cpp 

CPP_DEPS += \
./src/test/ipc/IpcCommandTest.d \
./src/test/ipc/IpcCtrlTest.d 

OBJS += \
./src/test/ipc/IpcCommandTest.o \
./src/test/ipc/IpcCtrlTest.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/ipc/%.o: ../src/test/ipc/%.cpp src/test/ipc/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-ipc

clean-src-2f-test-2f-ipc:
	-$(RM) ./src/test/ipc/IpcCommandTest.d ./src/test/ipc/IpcCommandTest.o ./src/test/ipc/IpcCtrlTest.d ./src/test/ipc/IpcCtrlTest.o

.PHONY: clean-src-2f-test-2f-ipc

