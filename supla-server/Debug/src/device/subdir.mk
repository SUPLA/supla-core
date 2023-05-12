################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/device/abstract_channel_property_getter.cpp \
../src/device/abstract_device_dao.cpp \
../src/device/channel_address.cpp \
../src/device/channel_electricity_measurement.cpp \
../src/device/channel_fragment.cpp \
../src/device/channel_property_getter.cpp \
../src/device/channel_thermostat_measurement.cpp \
../src/device/device.cpp \
../src/device/device_dao.cpp \
../src/device/devicechannel.cpp \
../src/device/devicechannels.cpp 

CPP_DEPS += \
./src/device/abstract_channel_property_getter.d \
./src/device/abstract_device_dao.d \
./src/device/channel_address.d \
./src/device/channel_electricity_measurement.d \
./src/device/channel_fragment.d \
./src/device/channel_property_getter.d \
./src/device/channel_thermostat_measurement.d \
./src/device/device.d \
./src/device/device_dao.d \
./src/device/devicechannel.d \
./src/device/devicechannels.d 

OBJS += \
./src/device/abstract_channel_property_getter.o \
./src/device/abstract_device_dao.o \
./src/device/channel_address.o \
./src/device/channel_electricity_measurement.o \
./src/device/channel_fragment.o \
./src/device/channel_property_getter.o \
./src/device/channel_thermostat_measurement.o \
./src/device/device.o \
./src/device/device_dao.o \
./src/device/devicechannel.o \
./src/device/devicechannels.o 


# Each subdirectory must supply rules for building sources it contributes
src/device/%.o: ../src/device/%.cpp src/device/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-device

clean-src-2f-device:
	-$(RM) ./src/device/abstract_channel_property_getter.d ./src/device/abstract_channel_property_getter.o ./src/device/abstract_device_dao.d ./src/device/abstract_device_dao.o ./src/device/channel_address.d ./src/device/channel_address.o ./src/device/channel_electricity_measurement.d ./src/device/channel_electricity_measurement.o ./src/device/channel_fragment.d ./src/device/channel_fragment.o ./src/device/channel_property_getter.d ./src/device/channel_property_getter.o ./src/device/channel_thermostat_measurement.d ./src/device/channel_thermostat_measurement.o ./src/device/device.d ./src/device/device.o ./src/device/device_dao.d ./src/device/device_dao.o ./src/device/devicechannel.d ./src/device/devicechannel.o ./src/device/devicechannels.d ./src/device/devicechannels.o

.PHONY: clean-src-2f-device

