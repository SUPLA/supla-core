################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/doubles/ipc/ClientReconnectCommandMock.cpp \
../src/test/doubles/ipc/GetCharCommandMock.cpp \
../src/test/doubles/ipc/GetDoubleCommandMock.cpp \
../src/test/doubles/ipc/GetHumidityCommandMock.cpp \
../src/test/doubles/ipc/GetRGBWCommandMock.cpp \
../src/test/doubles/ipc/GetTemperatureCommandMock.cpp \
../src/test/doubles/ipc/IpcCtrlMock.cpp \
../src/test/doubles/ipc/IpcSocketAdapterMock.cpp \
../src/test/doubles/ipc/IsChannelConnectedCommandMock.cpp \
../src/test/doubles/ipc/IsClientConnectedCommandMock.cpp \
../src/test/doubles/ipc/IsDeviceConnectedCommandMock.cpp \
../src/test/doubles/ipc/UserReconnectCommandMock.cpp 

CPP_DEPS += \
./src/test/doubles/ipc/ClientReconnectCommandMock.d \
./src/test/doubles/ipc/GetCharCommandMock.d \
./src/test/doubles/ipc/GetDoubleCommandMock.d \
./src/test/doubles/ipc/GetHumidityCommandMock.d \
./src/test/doubles/ipc/GetRGBWCommandMock.d \
./src/test/doubles/ipc/GetTemperatureCommandMock.d \
./src/test/doubles/ipc/IpcCtrlMock.d \
./src/test/doubles/ipc/IpcSocketAdapterMock.d \
./src/test/doubles/ipc/IsChannelConnectedCommandMock.d \
./src/test/doubles/ipc/IsClientConnectedCommandMock.d \
./src/test/doubles/ipc/IsDeviceConnectedCommandMock.d \
./src/test/doubles/ipc/UserReconnectCommandMock.d 

OBJS += \
./src/test/doubles/ipc/ClientReconnectCommandMock.o \
./src/test/doubles/ipc/GetCharCommandMock.o \
./src/test/doubles/ipc/GetDoubleCommandMock.o \
./src/test/doubles/ipc/GetHumidityCommandMock.o \
./src/test/doubles/ipc/GetRGBWCommandMock.o \
./src/test/doubles/ipc/GetTemperatureCommandMock.o \
./src/test/doubles/ipc/IpcCtrlMock.o \
./src/test/doubles/ipc/IpcSocketAdapterMock.o \
./src/test/doubles/ipc/IsChannelConnectedCommandMock.o \
./src/test/doubles/ipc/IsClientConnectedCommandMock.o \
./src/test/doubles/ipc/IsDeviceConnectedCommandMock.o \
./src/test/doubles/ipc/UserReconnectCommandMock.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/doubles/ipc/%.o: ../src/test/doubles/ipc/%.cpp src/test/doubles/ipc/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-doubles-2f-ipc

clean-src-2f-test-2f-doubles-2f-ipc:
	-$(RM) ./src/test/doubles/ipc/ClientReconnectCommandMock.d ./src/test/doubles/ipc/ClientReconnectCommandMock.o ./src/test/doubles/ipc/GetCharCommandMock.d ./src/test/doubles/ipc/GetCharCommandMock.o ./src/test/doubles/ipc/GetDoubleCommandMock.d ./src/test/doubles/ipc/GetDoubleCommandMock.o ./src/test/doubles/ipc/GetHumidityCommandMock.d ./src/test/doubles/ipc/GetHumidityCommandMock.o ./src/test/doubles/ipc/GetRGBWCommandMock.d ./src/test/doubles/ipc/GetRGBWCommandMock.o ./src/test/doubles/ipc/GetTemperatureCommandMock.d ./src/test/doubles/ipc/GetTemperatureCommandMock.o ./src/test/doubles/ipc/IpcCtrlMock.d ./src/test/doubles/ipc/IpcCtrlMock.o ./src/test/doubles/ipc/IpcSocketAdapterMock.d ./src/test/doubles/ipc/IpcSocketAdapterMock.o ./src/test/doubles/ipc/IsChannelConnectedCommandMock.d ./src/test/doubles/ipc/IsChannelConnectedCommandMock.o ./src/test/doubles/ipc/IsClientConnectedCommandMock.d ./src/test/doubles/ipc/IsClientConnectedCommandMock.o ./src/test/doubles/ipc/IsDeviceConnectedCommandMock.d ./src/test/doubles/ipc/IsDeviceConnectedCommandMock.o ./src/test/doubles/ipc/UserReconnectCommandMock.d ./src/test/doubles/ipc/UserReconnectCommandMock.o

.PHONY: clean-src-2f-test-2f-doubles-2f-ipc

