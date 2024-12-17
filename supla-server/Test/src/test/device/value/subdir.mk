################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/device/value/ChannelBinarySensorValueTest.cpp \
../src/test/device/value/ChannelContainerValueTest.cpp \
../src/test/device/value/ChannelDgfValueTest.cpp \
../src/test/device/value/ChannelEmValueTest.cpp \
../src/test/device/value/ChannelFbValueTest.cpp \
../src/test/device/value/ChannelFloatingPointSensorValueTest.cpp \
../src/test/device/value/ChannelGateValueTest.cpp \
../src/test/device/value/ChannelGeneralPurposeBaseValueTest.cpp \
../src/test/device/value/ChannelHpThermostatValueTest.cpp \
../src/test/device/value/ChannelHvacValueTest.cpp \
../src/test/device/value/ChannelHvacValueWithTempHumTest.cpp \
../src/test/device/value/ChannelIcValueTest.cpp \
../src/test/device/value/ChannelOnOffValueTest.cpp \
../src/test/device/value/ChannelOpenClosedValueTest.cpp \
../src/test/device/value/ChannelRgbwValueTest.cpp \
../src/test/device/value/ChannelRsValueTest.cpp \
../src/test/device/value/ChannelTempHumValueTest.cpp \
../src/test/device/value/ChannelValueTest.cpp \
../src/test/device/value/ChannelValveValueTest.cpp 

CPP_DEPS += \
./src/test/device/value/ChannelBinarySensorValueTest.d \
./src/test/device/value/ChannelContainerValueTest.d \
./src/test/device/value/ChannelDgfValueTest.d \
./src/test/device/value/ChannelEmValueTest.d \
./src/test/device/value/ChannelFbValueTest.d \
./src/test/device/value/ChannelFloatingPointSensorValueTest.d \
./src/test/device/value/ChannelGateValueTest.d \
./src/test/device/value/ChannelGeneralPurposeBaseValueTest.d \
./src/test/device/value/ChannelHpThermostatValueTest.d \
./src/test/device/value/ChannelHvacValueTest.d \
./src/test/device/value/ChannelHvacValueWithTempHumTest.d \
./src/test/device/value/ChannelIcValueTest.d \
./src/test/device/value/ChannelOnOffValueTest.d \
./src/test/device/value/ChannelOpenClosedValueTest.d \
./src/test/device/value/ChannelRgbwValueTest.d \
./src/test/device/value/ChannelRsValueTest.d \
./src/test/device/value/ChannelTempHumValueTest.d \
./src/test/device/value/ChannelValueTest.d \
./src/test/device/value/ChannelValveValueTest.d 

OBJS += \
./src/test/device/value/ChannelBinarySensorValueTest.o \
./src/test/device/value/ChannelContainerValueTest.o \
./src/test/device/value/ChannelDgfValueTest.o \
./src/test/device/value/ChannelEmValueTest.o \
./src/test/device/value/ChannelFbValueTest.o \
./src/test/device/value/ChannelFloatingPointSensorValueTest.o \
./src/test/device/value/ChannelGateValueTest.o \
./src/test/device/value/ChannelGeneralPurposeBaseValueTest.o \
./src/test/device/value/ChannelHpThermostatValueTest.o \
./src/test/device/value/ChannelHvacValueTest.o \
./src/test/device/value/ChannelHvacValueWithTempHumTest.o \
./src/test/device/value/ChannelIcValueTest.o \
./src/test/device/value/ChannelOnOffValueTest.o \
./src/test/device/value/ChannelOpenClosedValueTest.o \
./src/test/device/value/ChannelRgbwValueTest.o \
./src/test/device/value/ChannelRsValueTest.o \
./src/test/device/value/ChannelTempHumValueTest.o \
./src/test/device/value/ChannelValueTest.o \
./src/test/device/value/ChannelValveValueTest.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/device/value/%.o: ../src/test/device/value/%.cpp src/test/device/value/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-device-2f-value

clean-src-2f-test-2f-device-2f-value:
	-$(RM) ./src/test/device/value/ChannelBinarySensorValueTest.d ./src/test/device/value/ChannelBinarySensorValueTest.o ./src/test/device/value/ChannelContainerValueTest.d ./src/test/device/value/ChannelContainerValueTest.o ./src/test/device/value/ChannelDgfValueTest.d ./src/test/device/value/ChannelDgfValueTest.o ./src/test/device/value/ChannelEmValueTest.d ./src/test/device/value/ChannelEmValueTest.o ./src/test/device/value/ChannelFbValueTest.d ./src/test/device/value/ChannelFbValueTest.o ./src/test/device/value/ChannelFloatingPointSensorValueTest.d ./src/test/device/value/ChannelFloatingPointSensorValueTest.o ./src/test/device/value/ChannelGateValueTest.d ./src/test/device/value/ChannelGateValueTest.o ./src/test/device/value/ChannelGeneralPurposeBaseValueTest.d ./src/test/device/value/ChannelGeneralPurposeBaseValueTest.o ./src/test/device/value/ChannelHpThermostatValueTest.d ./src/test/device/value/ChannelHpThermostatValueTest.o ./src/test/device/value/ChannelHvacValueTest.d ./src/test/device/value/ChannelHvacValueTest.o ./src/test/device/value/ChannelHvacValueWithTempHumTest.d ./src/test/device/value/ChannelHvacValueWithTempHumTest.o ./src/test/device/value/ChannelIcValueTest.d ./src/test/device/value/ChannelIcValueTest.o ./src/test/device/value/ChannelOnOffValueTest.d ./src/test/device/value/ChannelOnOffValueTest.o ./src/test/device/value/ChannelOpenClosedValueTest.d ./src/test/device/value/ChannelOpenClosedValueTest.o ./src/test/device/value/ChannelRgbwValueTest.d ./src/test/device/value/ChannelRgbwValueTest.o ./src/test/device/value/ChannelRsValueTest.d ./src/test/device/value/ChannelRsValueTest.o ./src/test/device/value/ChannelTempHumValueTest.d ./src/test/device/value/ChannelTempHumValueTest.o ./src/test/device/value/ChannelValueTest.d ./src/test/device/value/ChannelValueTest.o ./src/test/device/value/ChannelValveValueTest.d ./src/test/device/value/ChannelValveValueTest.o

.PHONY: clean-src-2f-test-2f-device-2f-value

