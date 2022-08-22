################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/device/call_handler/abstract_device_channel_value_changed.cpp \
../src/device/call_handler/device_channel_value_changed.cpp \
../src/device/call_handler/device_channel_value_changed_b.cpp \
../src/device/call_handler/register_device.cpp \
../src/device/call_handler/register_device_b.cpp \
../src/device/call_handler/register_device_c.cpp \
../src/device/call_handler/register_device_d.cpp \
../src/device/call_handler/register_device_e.cpp 

CPP_DEPS += \
./src/device/call_handler/abstract_device_channel_value_changed.d \
./src/device/call_handler/device_channel_value_changed.d \
./src/device/call_handler/device_channel_value_changed_b.d \
./src/device/call_handler/register_device.d \
./src/device/call_handler/register_device_b.d \
./src/device/call_handler/register_device_c.d \
./src/device/call_handler/register_device_d.d \
./src/device/call_handler/register_device_e.d 

OBJS += \
./src/device/call_handler/abstract_device_channel_value_changed.o \
./src/device/call_handler/device_channel_value_changed.o \
./src/device/call_handler/device_channel_value_changed_b.o \
./src/device/call_handler/register_device.o \
./src/device/call_handler/register_device_b.o \
./src/device/call_handler/register_device_c.o \
./src/device/call_handler/register_device_d.o \
./src/device/call_handler/register_device_e.o 


# Each subdirectory must supply rules for building sources it contributes
src/device/call_handler/%.o: ../src/device/call_handler/%.cpp src/device/call_handler/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-device-2f-call_handler

clean-src-2f-device-2f-call_handler:
	-$(RM) ./src/device/call_handler/abstract_device_channel_value_changed.d ./src/device/call_handler/abstract_device_channel_value_changed.o ./src/device/call_handler/device_channel_value_changed.d ./src/device/call_handler/device_channel_value_changed.o ./src/device/call_handler/device_channel_value_changed_b.d ./src/device/call_handler/device_channel_value_changed_b.o ./src/device/call_handler/register_device.d ./src/device/call_handler/register_device.o ./src/device/call_handler/register_device_b.d ./src/device/call_handler/register_device_b.o ./src/device/call_handler/register_device_c.d ./src/device/call_handler/register_device_c.o ./src/device/call_handler/register_device_d.d ./src/device/call_handler/register_device_d.o ./src/device/call_handler/register_device_e.d ./src/device/call_handler/register_device_e.o

.PHONY: clean-src-2f-device-2f-call_handler

