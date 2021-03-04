################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/integration/mqtt/MqttClientIntegrationTest.cpp \
../src/test/integration/mqtt/MqttClientLibraryAdapterMock.cpp \
../src/test/integration/mqtt/MqttClientSettingsMock.cpp \
../src/test/integration/mqtt/MqttPublisherIntegrationTest.cpp \
../src/test/integration/mqtt/MqttSubscriberIntegrationTest.cpp \
../src/test/integration/mqtt/MqttUnpublisherIntegrationTest.cpp \
../src/test/integration/mqtt/MqttValueSetterMock.cpp 

OBJS += \
./src/test/integration/mqtt/MqttClientIntegrationTest.o \
./src/test/integration/mqtt/MqttClientLibraryAdapterMock.o \
./src/test/integration/mqtt/MqttClientSettingsMock.o \
./src/test/integration/mqtt/MqttPublisherIntegrationTest.o \
./src/test/integration/mqtt/MqttSubscriberIntegrationTest.o \
./src/test/integration/mqtt/MqttUnpublisherIntegrationTest.o \
./src/test/integration/mqtt/MqttValueSetterMock.o 

CPP_DEPS += \
./src/test/integration/mqtt/MqttClientIntegrationTest.d \
./src/test/integration/mqtt/MqttClientLibraryAdapterMock.d \
./src/test/integration/mqtt/MqttClientSettingsMock.d \
./src/test/integration/mqtt/MqttPublisherIntegrationTest.d \
./src/test/integration/mqtt/MqttSubscriberIntegrationTest.d \
./src/test/integration/mqtt/MqttUnpublisherIntegrationTest.d \
./src/test/integration/mqtt/MqttValueSetterMock.d 


# Each subdirectory must supply rules for building sources it contributes
src/test/integration/mqtt/%.o: ../src/test/integration/mqtt/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DSERVER_VERSION_23 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


