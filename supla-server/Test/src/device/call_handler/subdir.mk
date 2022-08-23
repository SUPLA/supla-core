################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/device/call_handler/abstract_device_channel_value_changed.cpp \
../src/device/call_handler/abstract_device_srpc_call_handler.cpp \
../src/device/call_handler/action_trigger.cpp \
../src/device/call_handler/channel_set_value_result.cpp \
../src/device/call_handler/channel_state_result.cpp \
../src/device/call_handler/device_calcfg_result.cpp \
../src/device/call_handler/device_channel_extendedvalue_changed.cpp \
../src/device/call_handler/device_channel_value_changed.cpp \
../src/device/call_handler/device_channel_value_changed_b.cpp \
../src/device/call_handler/device_channel_value_changed_c.cpp \
../src/device/call_handler/get_channel_config.cpp \
../src/device/call_handler/get_channel_functions.cpp \
../src/device/call_handler/get_firmware_update_url.cpp \
../src/device/call_handler/register_device.cpp \
../src/device/call_handler/register_device_b.cpp \
../src/device/call_handler/register_device_c.cpp \
../src/device/call_handler/register_device_d.cpp \
../src/device/call_handler/register_device_e.cpp 

CPP_DEPS += \
./src/device/call_handler/abstract_device_channel_value_changed.d \
./src/device/call_handler/abstract_device_srpc_call_handler.d \
./src/device/call_handler/action_trigger.d \
./src/device/call_handler/channel_set_value_result.d \
./src/device/call_handler/channel_state_result.d \
./src/device/call_handler/device_calcfg_result.d \
./src/device/call_handler/device_channel_extendedvalue_changed.d \
./src/device/call_handler/device_channel_value_changed.d \
./src/device/call_handler/device_channel_value_changed_b.d \
./src/device/call_handler/device_channel_value_changed_c.d \
./src/device/call_handler/get_channel_config.d \
./src/device/call_handler/get_channel_functions.d \
./src/device/call_handler/get_firmware_update_url.d \
./src/device/call_handler/register_device.d \
./src/device/call_handler/register_device_b.d \
./src/device/call_handler/register_device_c.d \
./src/device/call_handler/register_device_d.d \
./src/device/call_handler/register_device_e.d 

OBJS += \
./src/device/call_handler/abstract_device_channel_value_changed.o \
./src/device/call_handler/abstract_device_srpc_call_handler.o \
./src/device/call_handler/action_trigger.o \
./src/device/call_handler/channel_set_value_result.o \
./src/device/call_handler/channel_state_result.o \
./src/device/call_handler/device_calcfg_result.o \
./src/device/call_handler/device_channel_extendedvalue_changed.o \
./src/device/call_handler/device_channel_value_changed.o \
./src/device/call_handler/device_channel_value_changed_b.o \
./src/device/call_handler/device_channel_value_changed_c.o \
./src/device/call_handler/get_channel_config.o \
./src/device/call_handler/get_channel_functions.o \
./src/device/call_handler/get_firmware_update_url.o \
./src/device/call_handler/register_device.o \
./src/device/call_handler/register_device_b.o \
./src/device/call_handler/register_device_c.o \
./src/device/call_handler/register_device_d.o \
./src/device/call_handler/register_device_e.o 


# Each subdirectory must supply rules for building sources it contributes
src/device/call_handler/%.o: ../src/device/call_handler/%.cpp src/device/call_handler/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-device-2f-call_handler

clean-src-2f-device-2f-call_handler:
	-$(RM) ./src/device/call_handler/abstract_device_channel_value_changed.d ./src/device/call_handler/abstract_device_channel_value_changed.o ./src/device/call_handler/abstract_device_srpc_call_handler.d ./src/device/call_handler/abstract_device_srpc_call_handler.o ./src/device/call_handler/action_trigger.d ./src/device/call_handler/action_trigger.o ./src/device/call_handler/channel_set_value_result.d ./src/device/call_handler/channel_set_value_result.o ./src/device/call_handler/channel_state_result.d ./src/device/call_handler/channel_state_result.o ./src/device/call_handler/device_calcfg_result.d ./src/device/call_handler/device_calcfg_result.o ./src/device/call_handler/device_channel_extendedvalue_changed.d ./src/device/call_handler/device_channel_extendedvalue_changed.o ./src/device/call_handler/device_channel_value_changed.d ./src/device/call_handler/device_channel_value_changed.o ./src/device/call_handler/device_channel_value_changed_b.d ./src/device/call_handler/device_channel_value_changed_b.o ./src/device/call_handler/device_channel_value_changed_c.d ./src/device/call_handler/device_channel_value_changed_c.o ./src/device/call_handler/get_channel_config.d ./src/device/call_handler/get_channel_config.o ./src/device/call_handler/get_channel_functions.d ./src/device/call_handler/get_channel_functions.o ./src/device/call_handler/get_firmware_update_url.d ./src/device/call_handler/get_firmware_update_url.o ./src/device/call_handler/register_device.d ./src/device/call_handler/register_device.o ./src/device/call_handler/register_device_b.d ./src/device/call_handler/register_device_b.o ./src/device/call_handler/register_device_c.d ./src/device/call_handler/register_device_c.o ./src/device/call_handler/register_device_d.d ./src/device/call_handler/register_device_d.o ./src/device/call_handler/register_device_e.d ./src/device/call_handler/register_device_e.o

.PHONY: clean-src-2f-device-2f-call_handler

