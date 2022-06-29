################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/doubles/ipc/GetCharCommandMock.cpp \
../src/test/doubles/ipc/IpcCtrlMock.cpp \
../src/test/doubles/ipc/IpcSocketAdapterMock.cpp 

CPP_DEPS += \
./src/test/doubles/ipc/GetCharCommandMock.d \
./src/test/doubles/ipc/IpcCtrlMock.d \
./src/test/doubles/ipc/IpcSocketAdapterMock.d 

OBJS += \
./src/test/doubles/ipc/GetCharCommandMock.o \
./src/test/doubles/ipc/IpcCtrlMock.o \
./src/test/doubles/ipc/IpcSocketAdapterMock.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/doubles/ipc/%.o: ../src/test/doubles/ipc/%.cpp src/test/doubles/ipc/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-doubles-2f-ipc

clean-src-2f-test-2f-doubles-2f-ipc:
	-$(RM) ./src/test/doubles/ipc/GetCharCommandMock.d ./src/test/doubles/ipc/GetCharCommandMock.o ./src/test/doubles/ipc/IpcCtrlMock.d ./src/test/doubles/ipc/IpcCtrlMock.o ./src/test/doubles/ipc/IpcSocketAdapterMock.d ./src/test/doubles/ipc/IpcSocketAdapterMock.o

.PHONY: clean-src-2f-test-2f-doubles-2f-ipc

