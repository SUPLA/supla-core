################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/mqtt/MqttChannelAndStateRemovedTopicsProviderTest.cpp \
../src/test/mqtt/MqttChannelMessageProviderTest.cpp \
../src/test/mqtt/MqttClientTest.cpp \
../src/test/mqtt/MqttDataSourceTest.cpp \
../src/test/mqtt/MqttDeviceMessageProviderTest.cpp \
../src/test/mqtt/MqttDsContextTest.cpp \
../src/test/mqtt/MqttIniSettingsTest.cpp \
../src/test/mqtt/MqttMessageProviderTest.cpp \
../src/test/mqtt/MqttStateMessageProviderTest.cpp \
../src/test/mqtt/MqttSubscriberTest.cpp \
../src/test/mqtt/MqttTopicCmpTest.cpp \
../src/test/mqtt/MqttUserMessageProviderTest.cpp 

CPP_DEPS += \
./src/test/mqtt/MqttChannelAndStateRemovedTopicsProviderTest.d \
./src/test/mqtt/MqttChannelMessageProviderTest.d \
./src/test/mqtt/MqttClientTest.d \
./src/test/mqtt/MqttDataSourceTest.d \
./src/test/mqtt/MqttDeviceMessageProviderTest.d \
./src/test/mqtt/MqttDsContextTest.d \
./src/test/mqtt/MqttIniSettingsTest.d \
./src/test/mqtt/MqttMessageProviderTest.d \
./src/test/mqtt/MqttStateMessageProviderTest.d \
./src/test/mqtt/MqttSubscriberTest.d \
./src/test/mqtt/MqttTopicCmpTest.d \
./src/test/mqtt/MqttUserMessageProviderTest.d 

OBJS += \
./src/test/mqtt/MqttChannelAndStateRemovedTopicsProviderTest.o \
./src/test/mqtt/MqttChannelMessageProviderTest.o \
./src/test/mqtt/MqttClientTest.o \
./src/test/mqtt/MqttDataSourceTest.o \
./src/test/mqtt/MqttDeviceMessageProviderTest.o \
./src/test/mqtt/MqttDsContextTest.o \
./src/test/mqtt/MqttIniSettingsTest.o \
./src/test/mqtt/MqttMessageProviderTest.o \
./src/test/mqtt/MqttStateMessageProviderTest.o \
./src/test/mqtt/MqttSubscriberTest.o \
./src/test/mqtt/MqttTopicCmpTest.o \
./src/test/mqtt/MqttUserMessageProviderTest.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/mqtt/%.o: ../src/test/mqtt/%.cpp src/test/mqtt/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-mqtt

clean-src-2f-test-2f-mqtt:
	-$(RM) ./src/test/mqtt/MqttChannelAndStateRemovedTopicsProviderTest.d ./src/test/mqtt/MqttChannelAndStateRemovedTopicsProviderTest.o ./src/test/mqtt/MqttChannelMessageProviderTest.d ./src/test/mqtt/MqttChannelMessageProviderTest.o ./src/test/mqtt/MqttClientTest.d ./src/test/mqtt/MqttClientTest.o ./src/test/mqtt/MqttDataSourceTest.d ./src/test/mqtt/MqttDataSourceTest.o ./src/test/mqtt/MqttDeviceMessageProviderTest.d ./src/test/mqtt/MqttDeviceMessageProviderTest.o ./src/test/mqtt/MqttDsContextTest.d ./src/test/mqtt/MqttDsContextTest.o ./src/test/mqtt/MqttIniSettingsTest.d ./src/test/mqtt/MqttIniSettingsTest.o ./src/test/mqtt/MqttMessageProviderTest.d ./src/test/mqtt/MqttMessageProviderTest.o ./src/test/mqtt/MqttStateMessageProviderTest.d ./src/test/mqtt/MqttStateMessageProviderTest.o ./src/test/mqtt/MqttSubscriberTest.d ./src/test/mqtt/MqttSubscriberTest.o ./src/test/mqtt/MqttTopicCmpTest.d ./src/test/mqtt/MqttTopicCmpTest.o ./src/test/mqtt/MqttUserMessageProviderTest.d ./src/test/mqtt/MqttUserMessageProviderTest.o

.PHONY: clean-src-2f-test-2f-mqtt

