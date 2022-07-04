################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ipc/abstract_client_reconnect_command.cpp \
../src/ipc/abstract_get_char_command.cpp \
../src/ipc/abstract_get_double_command.cpp \
../src/ipc/abstract_get_em_value_command.cpp \
../src/ipc/abstract_get_humidity_command.cpp \
../src/ipc/abstract_get_icm_value_command.cpp \
../src/ipc/abstract_get_relay_value_command.cpp \
../src/ipc/abstract_get_rgbw_command.cpp \
../src/ipc/abstract_get_temperature_command.cpp \
../src/ipc/abstract_get_valve_value_command.cpp \
../src/ipc/abstract_ipc_command.cpp \
../src/ipc/abstract_ipc_ctrl.cpp \
../src/ipc/abstract_ipc_socket_adapter.cpp \
../src/ipc/abstract_is_channel_connected_command.cpp \
../src/ipc/abstract_is_client_connected_command.cpp \
../src/ipc/abstract_is_device_connected_command.cpp \
../src/ipc/abstract_set_char_command.cpp \
../src/ipc/abstract_user_reconnect_command.cpp \
../src/ipc/client_reconnect_command.cpp \
../src/ipc/get_char_command.cpp \
../src/ipc/get_double_command.cpp \
../src/ipc/get_em_value_command.cpp \
../src/ipc/get_humidity_command.cpp \
../src/ipc/get_icm_value_command.cpp \
../src/ipc/get_relay_value_command.cpp \
../src/ipc/get_rgbw_command.cpp \
../src/ipc/get_temperature_command.cpp \
../src/ipc/get_valve_value_command.cpp \
../src/ipc/ipc_ctrl.cpp \
../src/ipc/is_channel_connected_command.cpp \
../src/ipc/is_client_connected_command.cpp \
../src/ipc/is_device_connected_command.cpp \
../src/ipc/set_char_command.cpp \
../src/ipc/user_reconnect_command.cpp 

CPP_DEPS += \
./src/ipc/abstract_client_reconnect_command.d \
./src/ipc/abstract_get_char_command.d \
./src/ipc/abstract_get_double_command.d \
./src/ipc/abstract_get_em_value_command.d \
./src/ipc/abstract_get_humidity_command.d \
./src/ipc/abstract_get_icm_value_command.d \
./src/ipc/abstract_get_relay_value_command.d \
./src/ipc/abstract_get_rgbw_command.d \
./src/ipc/abstract_get_temperature_command.d \
./src/ipc/abstract_get_valve_value_command.d \
./src/ipc/abstract_ipc_command.d \
./src/ipc/abstract_ipc_ctrl.d \
./src/ipc/abstract_ipc_socket_adapter.d \
./src/ipc/abstract_is_channel_connected_command.d \
./src/ipc/abstract_is_client_connected_command.d \
./src/ipc/abstract_is_device_connected_command.d \
./src/ipc/abstract_set_char_command.d \
./src/ipc/abstract_user_reconnect_command.d \
./src/ipc/client_reconnect_command.d \
./src/ipc/get_char_command.d \
./src/ipc/get_double_command.d \
./src/ipc/get_em_value_command.d \
./src/ipc/get_humidity_command.d \
./src/ipc/get_icm_value_command.d \
./src/ipc/get_relay_value_command.d \
./src/ipc/get_rgbw_command.d \
./src/ipc/get_temperature_command.d \
./src/ipc/get_valve_value_command.d \
./src/ipc/ipc_ctrl.d \
./src/ipc/is_channel_connected_command.d \
./src/ipc/is_client_connected_command.d \
./src/ipc/is_device_connected_command.d \
./src/ipc/set_char_command.d \
./src/ipc/user_reconnect_command.d 

OBJS += \
./src/ipc/abstract_client_reconnect_command.o \
./src/ipc/abstract_get_char_command.o \
./src/ipc/abstract_get_double_command.o \
./src/ipc/abstract_get_em_value_command.o \
./src/ipc/abstract_get_humidity_command.o \
./src/ipc/abstract_get_icm_value_command.o \
./src/ipc/abstract_get_relay_value_command.o \
./src/ipc/abstract_get_rgbw_command.o \
./src/ipc/abstract_get_temperature_command.o \
./src/ipc/abstract_get_valve_value_command.o \
./src/ipc/abstract_ipc_command.o \
./src/ipc/abstract_ipc_ctrl.o \
./src/ipc/abstract_ipc_socket_adapter.o \
./src/ipc/abstract_is_channel_connected_command.o \
./src/ipc/abstract_is_client_connected_command.o \
./src/ipc/abstract_is_device_connected_command.o \
./src/ipc/abstract_set_char_command.o \
./src/ipc/abstract_user_reconnect_command.o \
./src/ipc/client_reconnect_command.o \
./src/ipc/get_char_command.o \
./src/ipc/get_double_command.o \
./src/ipc/get_em_value_command.o \
./src/ipc/get_humidity_command.o \
./src/ipc/get_icm_value_command.o \
./src/ipc/get_relay_value_command.o \
./src/ipc/get_rgbw_command.o \
./src/ipc/get_temperature_command.o \
./src/ipc/get_valve_value_command.o \
./src/ipc/ipc_ctrl.o \
./src/ipc/is_channel_connected_command.o \
./src/ipc/is_client_connected_command.o \
./src/ipc/is_device_connected_command.o \
./src/ipc/set_char_command.o \
./src/ipc/user_reconnect_command.o 


# Each subdirectory must supply rules for building sources it contributes
src/ipc/%.o: ../src/ipc/%.cpp src/ipc/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-ipc

clean-src-2f-ipc:
	-$(RM) ./src/ipc/abstract_client_reconnect_command.d ./src/ipc/abstract_client_reconnect_command.o ./src/ipc/abstract_get_char_command.d ./src/ipc/abstract_get_char_command.o ./src/ipc/abstract_get_double_command.d ./src/ipc/abstract_get_double_command.o ./src/ipc/abstract_get_em_value_command.d ./src/ipc/abstract_get_em_value_command.o ./src/ipc/abstract_get_humidity_command.d ./src/ipc/abstract_get_humidity_command.o ./src/ipc/abstract_get_icm_value_command.d ./src/ipc/abstract_get_icm_value_command.o ./src/ipc/abstract_get_relay_value_command.d ./src/ipc/abstract_get_relay_value_command.o ./src/ipc/abstract_get_rgbw_command.d ./src/ipc/abstract_get_rgbw_command.o ./src/ipc/abstract_get_temperature_command.d ./src/ipc/abstract_get_temperature_command.o ./src/ipc/abstract_get_valve_value_command.d ./src/ipc/abstract_get_valve_value_command.o ./src/ipc/abstract_ipc_command.d ./src/ipc/abstract_ipc_command.o ./src/ipc/abstract_ipc_ctrl.d ./src/ipc/abstract_ipc_ctrl.o ./src/ipc/abstract_ipc_socket_adapter.d ./src/ipc/abstract_ipc_socket_adapter.o ./src/ipc/abstract_is_channel_connected_command.d ./src/ipc/abstract_is_channel_connected_command.o ./src/ipc/abstract_is_client_connected_command.d ./src/ipc/abstract_is_client_connected_command.o ./src/ipc/abstract_is_device_connected_command.d ./src/ipc/abstract_is_device_connected_command.o ./src/ipc/abstract_set_char_command.d ./src/ipc/abstract_set_char_command.o ./src/ipc/abstract_user_reconnect_command.d ./src/ipc/abstract_user_reconnect_command.o ./src/ipc/client_reconnect_command.d ./src/ipc/client_reconnect_command.o ./src/ipc/get_char_command.d ./src/ipc/get_char_command.o ./src/ipc/get_double_command.d ./src/ipc/get_double_command.o ./src/ipc/get_em_value_command.d ./src/ipc/get_em_value_command.o ./src/ipc/get_humidity_command.d ./src/ipc/get_humidity_command.o ./src/ipc/get_icm_value_command.d ./src/ipc/get_icm_value_command.o ./src/ipc/get_relay_value_command.d ./src/ipc/get_relay_value_command.o ./src/ipc/get_rgbw_command.d ./src/ipc/get_rgbw_command.o ./src/ipc/get_temperature_command.d ./src/ipc/get_temperature_command.o ./src/ipc/get_valve_value_command.d ./src/ipc/get_valve_value_command.o ./src/ipc/ipc_ctrl.d ./src/ipc/ipc_ctrl.o ./src/ipc/is_channel_connected_command.d ./src/ipc/is_channel_connected_command.o ./src/ipc/is_client_connected_command.d ./src/ipc/is_client_connected_command.o ./src/ipc/is_device_connected_command.d ./src/ipc/is_device_connected_command.o ./src/ipc/set_char_command.d ./src/ipc/set_char_command.o ./src/ipc/user_reconnect_command.d ./src/ipc/user_reconnect_command.o

.PHONY: clean-src-2f-ipc

