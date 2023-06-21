################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/device/call_handler/abstract_device_channel_value_changed.cpp \
../src/device/call_handler/abstract_device_srpc_call_handler.cpp \
../src/device/call_handler/abstract_register_device.cpp \
../src/device/call_handler/action_trigger.cpp \
../src/device/call_handler/call_handler_collection.cpp \
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
../src/device/call_handler/register_device_a.cpp \
../src/device/call_handler/register_device_b.cpp \
../src/device/call_handler/register_device_c.cpp \
../src/device/call_handler/register_device_d.cpp \
../src/device/call_handler/register_device_e.cpp \
../src/device/call_handler/register_push_notification.cpp \
../src/device/call_handler/send_push_notification.cpp \
../src/device/call_handler/set_channel_config.cpp \
../src/device/call_handler/set_channel_config_result.cpp \
../src/device/call_handler/set_device_config.cpp \
../src/device/call_handler/set_device_config_multipart_call.cpp \
../src/device/call_handler/set_device_config_result.cpp 

CPP_DEPS += \
./src/device/call_handler/abstract_device_channel_value_changed.d \
./src/device/call_handler/abstract_device_srpc_call_handler.d \
./src/device/call_handler/abstract_register_device.d \
./src/device/call_handler/action_trigger.d \
./src/device/call_handler/call_handler_collection.d \
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
./src/device/call_handler/register_device_a.d \
./src/device/call_handler/register_device_b.d \
./src/device/call_handler/register_device_c.d \
./src/device/call_handler/register_device_d.d \
./src/device/call_handler/register_device_e.d \
./src/device/call_handler/register_push_notification.d \
./src/device/call_handler/send_push_notification.d \
./src/device/call_handler/set_channel_config.d \
./src/device/call_handler/set_channel_config_result.d \
./src/device/call_handler/set_device_config.d \
./src/device/call_handler/set_device_config_multipart_call.d \
./src/device/call_handler/set_device_config_result.d 

OBJS += \
./src/device/call_handler/abstract_device_channel_value_changed.o \
./src/device/call_handler/abstract_device_srpc_call_handler.o \
./src/device/call_handler/abstract_register_device.o \
./src/device/call_handler/action_trigger.o \
./src/device/call_handler/call_handler_collection.o \
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
./src/device/call_handler/register_device_a.o \
./src/device/call_handler/register_device_b.o \
./src/device/call_handler/register_device_c.o \
./src/device/call_handler/register_device_d.o \
./src/device/call_handler/register_device_e.o \
./src/device/call_handler/register_push_notification.o \
./src/device/call_handler/send_push_notification.o \
./src/device/call_handler/set_channel_config.o \
./src/device/call_handler/set_channel_config_result.o \
./src/device/call_handler/set_device_config.o \
./src/device/call_handler/set_device_config_multipart_call.o \
./src/device/call_handler/set_device_config_result.o 


# Each subdirectory must supply rules for building sources it contributes
src/device/call_handler/%.o: ../src/device/call_handler/%.cpp src/device/call_handler/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O2 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-device-2f-call_handler

clean-src-2f-device-2f-call_handler:
	-$(RM) ./src/device/call_handler/abstract_device_channel_value_changed.d ./src/device/call_handler/abstract_device_channel_value_changed.o ./src/device/call_handler/abstract_device_srpc_call_handler.d ./src/device/call_handler/abstract_device_srpc_call_handler.o ./src/device/call_handler/abstract_register_device.d ./src/device/call_handler/abstract_register_device.o ./src/device/call_handler/action_trigger.d ./src/device/call_handler/action_trigger.o ./src/device/call_handler/call_handler_collection.d ./src/device/call_handler/call_handler_collection.o ./src/device/call_handler/channel_set_value_result.d ./src/device/call_handler/channel_set_value_result.o ./src/device/call_handler/channel_state_result.d ./src/device/call_handler/channel_state_result.o ./src/device/call_handler/device_calcfg_result.d ./src/device/call_handler/device_calcfg_result.o ./src/device/call_handler/device_channel_extendedvalue_changed.d ./src/device/call_handler/device_channel_extendedvalue_changed.o ./src/device/call_handler/device_channel_value_changed.d ./src/device/call_handler/device_channel_value_changed.o ./src/device/call_handler/device_channel_value_changed_b.d ./src/device/call_handler/device_channel_value_changed_b.o ./src/device/call_handler/device_channel_value_changed_c.d ./src/device/call_handler/device_channel_value_changed_c.o ./src/device/call_handler/get_channel_config.d ./src/device/call_handler/get_channel_config.o ./src/device/call_handler/get_channel_functions.d ./src/device/call_handler/get_channel_functions.o ./src/device/call_handler/get_firmware_update_url.d ./src/device/call_handler/get_firmware_update_url.o ./src/device/call_handler/register_device.d ./src/device/call_handler/register_device.o ./src/device/call_handler/register_device_a.d ./src/device/call_handler/register_device_a.o ./src/device/call_handler/register_device_b.d ./src/device/call_handler/register_device_b.o ./src/device/call_handler/register_device_c.d ./src/device/call_handler/register_device_c.o ./src/device/call_handler/register_device_d.d ./src/device/call_handler/register_device_d.o ./src/device/call_handler/register_device_e.d ./src/device/call_handler/register_device_e.o ./src/device/call_handler/register_push_notification.d ./src/device/call_handler/register_push_notification.o ./src/device/call_handler/send_push_notification.d ./src/device/call_handler/send_push_notification.o ./src/device/call_handler/set_channel_config.d ./src/device/call_handler/set_channel_config.o ./src/device/call_handler/set_channel_config_result.d ./src/device/call_handler/set_channel_config_result.o ./src/device/call_handler/set_device_config.d ./src/device/call_handler/set_device_config.o ./src/device/call_handler/set_device_config_multipart_call.d ./src/device/call_handler/set_device_config_multipart_call.o ./src/device/call_handler/set_device_config_result.d ./src/device/call_handler/set_device_config_result.o

.PHONY: clean-src-2f-device-2f-call_handler

