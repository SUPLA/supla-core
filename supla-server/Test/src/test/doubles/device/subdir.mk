################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/doubles/device/DeviceDaoMock.cpp \
../src/test/doubles/device/DeviceStub.cpp \
../src/test/doubles/device/GateValueGetterStub.cpp \
../src/test/doubles/device/RegisterDeviceMock.cpp \
../src/test/doubles/device/ValueGetterMock.cpp \
../src/test/doubles/device/ValueGetterStub.cpp 

CPP_DEPS += \
./src/test/doubles/device/DeviceDaoMock.d \
./src/test/doubles/device/DeviceStub.d \
./src/test/doubles/device/GateValueGetterStub.d \
./src/test/doubles/device/RegisterDeviceMock.d \
./src/test/doubles/device/ValueGetterMock.d \
./src/test/doubles/device/ValueGetterStub.d 

OBJS += \
./src/test/doubles/device/DeviceDaoMock.o \
./src/test/doubles/device/DeviceStub.o \
./src/test/doubles/device/GateValueGetterStub.o \
./src/test/doubles/device/RegisterDeviceMock.o \
./src/test/doubles/device/ValueGetterMock.o \
./src/test/doubles/device/ValueGetterStub.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/doubles/device/%.o: ../src/test/doubles/device/%.cpp src/test/doubles/device/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-doubles-2f-device

clean-src-2f-test-2f-doubles-2f-device:
	-$(RM) ./src/test/doubles/device/DeviceDaoMock.d ./src/test/doubles/device/DeviceDaoMock.o ./src/test/doubles/device/DeviceStub.d ./src/test/doubles/device/DeviceStub.o ./src/test/doubles/device/GateValueGetterStub.d ./src/test/doubles/device/GateValueGetterStub.o ./src/test/doubles/device/RegisterDeviceMock.d ./src/test/doubles/device/RegisterDeviceMock.o ./src/test/doubles/device/ValueGetterMock.d ./src/test/doubles/device/ValueGetterMock.o ./src/test/doubles/device/ValueGetterStub.d ./src/test/doubles/device/ValueGetterStub.o

.PHONY: clean-src-2f-test-2f-doubles-2f-device

