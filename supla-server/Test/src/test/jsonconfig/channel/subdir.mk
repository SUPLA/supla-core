################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/jsonconfig/channel/ActionTriggerConfigTest.cpp \
../src/test/jsonconfig/channel/AlexaConfigTest.cpp \
../src/test/jsonconfig/channel/AltWeeklyScheduleConfigTest.cpp \
../src/test/jsonconfig/channel/BinarySensorConfigTest.cpp \
../src/test/jsonconfig/channel/ChannelJSONConfigTest.cpp \
../src/test/jsonconfig/channel/ContainerConfigTest.cpp \
../src/test/jsonconfig/channel/ControllingTheGateConfigTest.cpp \
../src/test/jsonconfig/channel/ElectricityMeterConfigTest.cpp \
../src/test/jsonconfig/channel/FacadeBlindConfigTest.cpp \
../src/test/jsonconfig/channel/GeneralPurposeMeasurementConfigTest.cpp \
../src/test/jsonconfig/channel/GeneralPurposeMeterConfigTest.cpp \
../src/test/jsonconfig/channel/GoogleHomeConfigTest.cpp \
../src/test/jsonconfig/channel/HvacConfigTest.cpp \
../src/test/jsonconfig/channel/ImpulseCounterConfigTest.cpp \
../src/test/jsonconfig/channel/OcrConfigTest.cpp \
../src/test/jsonconfig/channel/PowerSwitchConfigTest.cpp \
../src/test/jsonconfig/channel/RollershutterConfigTest.cpp \
../src/test/jsonconfig/channel/TempHumConfigTest.cpp \
../src/test/jsonconfig/channel/WeeklyScheduleConfigTest.cpp 

CPP_DEPS += \
./src/test/jsonconfig/channel/ActionTriggerConfigTest.d \
./src/test/jsonconfig/channel/AlexaConfigTest.d \
./src/test/jsonconfig/channel/AltWeeklyScheduleConfigTest.d \
./src/test/jsonconfig/channel/BinarySensorConfigTest.d \
./src/test/jsonconfig/channel/ChannelJSONConfigTest.d \
./src/test/jsonconfig/channel/ContainerConfigTest.d \
./src/test/jsonconfig/channel/ControllingTheGateConfigTest.d \
./src/test/jsonconfig/channel/ElectricityMeterConfigTest.d \
./src/test/jsonconfig/channel/FacadeBlindConfigTest.d \
./src/test/jsonconfig/channel/GeneralPurposeMeasurementConfigTest.d \
./src/test/jsonconfig/channel/GeneralPurposeMeterConfigTest.d \
./src/test/jsonconfig/channel/GoogleHomeConfigTest.d \
./src/test/jsonconfig/channel/HvacConfigTest.d \
./src/test/jsonconfig/channel/ImpulseCounterConfigTest.d \
./src/test/jsonconfig/channel/OcrConfigTest.d \
./src/test/jsonconfig/channel/PowerSwitchConfigTest.d \
./src/test/jsonconfig/channel/RollershutterConfigTest.d \
./src/test/jsonconfig/channel/TempHumConfigTest.d \
./src/test/jsonconfig/channel/WeeklyScheduleConfigTest.d 

OBJS += \
./src/test/jsonconfig/channel/ActionTriggerConfigTest.o \
./src/test/jsonconfig/channel/AlexaConfigTest.o \
./src/test/jsonconfig/channel/AltWeeklyScheduleConfigTest.o \
./src/test/jsonconfig/channel/BinarySensorConfigTest.o \
./src/test/jsonconfig/channel/ChannelJSONConfigTest.o \
./src/test/jsonconfig/channel/ContainerConfigTest.o \
./src/test/jsonconfig/channel/ControllingTheGateConfigTest.o \
./src/test/jsonconfig/channel/ElectricityMeterConfigTest.o \
./src/test/jsonconfig/channel/FacadeBlindConfigTest.o \
./src/test/jsonconfig/channel/GeneralPurposeMeasurementConfigTest.o \
./src/test/jsonconfig/channel/GeneralPurposeMeterConfigTest.o \
./src/test/jsonconfig/channel/GoogleHomeConfigTest.o \
./src/test/jsonconfig/channel/HvacConfigTest.o \
./src/test/jsonconfig/channel/ImpulseCounterConfigTest.o \
./src/test/jsonconfig/channel/OcrConfigTest.o \
./src/test/jsonconfig/channel/PowerSwitchConfigTest.o \
./src/test/jsonconfig/channel/RollershutterConfigTest.o \
./src/test/jsonconfig/channel/TempHumConfigTest.o \
./src/test/jsonconfig/channel/WeeklyScheduleConfigTest.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/jsonconfig/channel/%.o: ../src/test/jsonconfig/channel/%.cpp src/test/jsonconfig/channel/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-jsonconfig-2f-channel

clean-src-2f-test-2f-jsonconfig-2f-channel:
	-$(RM) ./src/test/jsonconfig/channel/ActionTriggerConfigTest.d ./src/test/jsonconfig/channel/ActionTriggerConfigTest.o ./src/test/jsonconfig/channel/AlexaConfigTest.d ./src/test/jsonconfig/channel/AlexaConfigTest.o ./src/test/jsonconfig/channel/AltWeeklyScheduleConfigTest.d ./src/test/jsonconfig/channel/AltWeeklyScheduleConfigTest.o ./src/test/jsonconfig/channel/BinarySensorConfigTest.d ./src/test/jsonconfig/channel/BinarySensorConfigTest.o ./src/test/jsonconfig/channel/ChannelJSONConfigTest.d ./src/test/jsonconfig/channel/ChannelJSONConfigTest.o ./src/test/jsonconfig/channel/ContainerConfigTest.d ./src/test/jsonconfig/channel/ContainerConfigTest.o ./src/test/jsonconfig/channel/ControllingTheGateConfigTest.d ./src/test/jsonconfig/channel/ControllingTheGateConfigTest.o ./src/test/jsonconfig/channel/ElectricityMeterConfigTest.d ./src/test/jsonconfig/channel/ElectricityMeterConfigTest.o ./src/test/jsonconfig/channel/FacadeBlindConfigTest.d ./src/test/jsonconfig/channel/FacadeBlindConfigTest.o ./src/test/jsonconfig/channel/GeneralPurposeMeasurementConfigTest.d ./src/test/jsonconfig/channel/GeneralPurposeMeasurementConfigTest.o ./src/test/jsonconfig/channel/GeneralPurposeMeterConfigTest.d ./src/test/jsonconfig/channel/GeneralPurposeMeterConfigTest.o ./src/test/jsonconfig/channel/GoogleHomeConfigTest.d ./src/test/jsonconfig/channel/GoogleHomeConfigTest.o ./src/test/jsonconfig/channel/HvacConfigTest.d ./src/test/jsonconfig/channel/HvacConfigTest.o ./src/test/jsonconfig/channel/ImpulseCounterConfigTest.d ./src/test/jsonconfig/channel/ImpulseCounterConfigTest.o ./src/test/jsonconfig/channel/OcrConfigTest.d ./src/test/jsonconfig/channel/OcrConfigTest.o ./src/test/jsonconfig/channel/PowerSwitchConfigTest.d ./src/test/jsonconfig/channel/PowerSwitchConfigTest.o ./src/test/jsonconfig/channel/RollershutterConfigTest.d ./src/test/jsonconfig/channel/RollershutterConfigTest.o ./src/test/jsonconfig/channel/TempHumConfigTest.d ./src/test/jsonconfig/channel/TempHumConfigTest.o ./src/test/jsonconfig/channel/WeeklyScheduleConfigTest.d ./src/test/jsonconfig/channel/WeeklyScheduleConfigTest.o

.PHONY: clean-src-2f-test-2f-jsonconfig-2f-channel

