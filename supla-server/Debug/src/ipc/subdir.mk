################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ipc/abstract_action_cg_command.cpp \
../src/ipc/abstract_action_command.cpp \
../src/ipc/abstract_alexa_cred_changed_command.cpp \
../src/ipc/abstract_before_channel_function_change_command.cpp \
../src/ipc/abstract_before_device_delete_command.cpp \
../src/ipc/abstract_client_reconnect_command.cpp \
../src/ipc/abstract_device_set_time_command.cpp \
../src/ipc/abstract_enter_cfg_mode_command.cpp \
../src/ipc/abstract_execute_scene_command.cpp \
../src/ipc/abstract_get_char_command.cpp \
../src/ipc/abstract_get_digiglass_value_command.cpp \
../src/ipc/abstract_get_double_command.cpp \
../src/ipc/abstract_get_em_value_command.cpp \
../src/ipc/abstract_get_facade_blind_value_command.cpp \
../src/ipc/abstract_get_gpm_value_command.cpp \
../src/ipc/abstract_get_humidity_command.cpp \
../src/ipc/abstract_get_hvac_value_command.cpp \
../src/ipc/abstract_get_ic_value_command.cpp \
../src/ipc/abstract_get_pn_limit_command.cpp \
../src/ipc/abstract_get_relay_value_command.cpp \
../src/ipc/abstract_get_rgbw_command.cpp \
../src/ipc/abstract_get_scene_summary_command.cpp \
../src/ipc/abstract_get_status_command.cpp \
../src/ipc/abstract_get_temperature_command.cpp \
../src/ipc/abstract_get_valve_value_command.cpp \
../src/ipc/abstract_gh_cred_changed_command.cpp \
../src/ipc/abstract_identify_device_command.cpp \
../src/ipc/abstract_identify_subdevice_command.cpp \
../src/ipc/abstract_interrupt_and_execute_scene_command.cpp \
../src/ipc/abstract_interrupt_scene_command.cpp \
../src/ipc/abstract_ipc_command.cpp \
../src/ipc/abstract_ipc_ctrl.cpp \
../src/ipc/abstract_ipc_socket_adapter.cpp \
../src/ipc/abstract_is_channel_connected_command.cpp \
../src/ipc/abstract_is_client_connected_command.cpp \
../src/ipc/abstract_is_device_connected_command.cpp \
../src/ipc/abstract_on_channel_config_changed_command.cpp \
../src/ipc/abstract_on_channel_deleted_command.cpp \
../src/ipc/abstract_on_device_config_changed_command.cpp \
../src/ipc/abstract_on_device_deleted_command.cpp \
../src/ipc/abstract_on_mqtt_settings_changed_command.cpp \
../src/ipc/abstract_on_scene_added_command.cpp \
../src/ipc/abstract_on_scene_changed_command.cpp \
../src/ipc/abstract_on_scene_removed_command.cpp \
../src/ipc/abstract_on_state_webhook_changed_command.cpp \
../src/ipc/abstract_on_user_settings_changed_command.cpp \
../src/ipc/abstract_on_vbt_changed_command.cpp \
../src/ipc/abstract_pair_subdevice_command.cpp \
../src/ipc/abstract_recalibrate_command.cpp \
../src/ipc/abstract_reset_counters_command.cpp \
../src/ipc/abstract_restart_device_command.cpp \
../src/ipc/abstract_restart_subdevice_command.cpp \
../src/ipc/abstract_send_push_command.cpp \
../src/ipc/abstract_set_cg_char_command.cpp \
../src/ipc/abstract_set_cg_rgbw_command.cpp \
../src/ipc/abstract_set_char_command.cpp \
../src/ipc/abstract_set_digiglass_value_command.cpp \
../src/ipc/abstract_set_rgbw_command.cpp \
../src/ipc/abstract_take_ocr_photo_command.cpp \
../src/ipc/abstract_user_reconnect_command.cpp \
../src/ipc/action_cg_command.cpp \
../src/ipc/action_command.cpp \
../src/ipc/alexa_cred_changed_command.cpp \
../src/ipc/before_channel_function_change_command.cpp \
../src/ipc/before_device_delete_command.cpp \
../src/ipc/client_reconnect_command.cpp \
../src/ipc/device_set_time_command.cpp \
../src/ipc/enter_cfg_mode_command.cpp \
../src/ipc/execute_scene_command.cpp \
../src/ipc/get_char_command.cpp \
../src/ipc/get_digiglass_value_command.cpp \
../src/ipc/get_double_command.cpp \
../src/ipc/get_em_value_command.cpp \
../src/ipc/get_facade_blind_value_command.cpp \
../src/ipc/get_gpm_value_command.cpp \
../src/ipc/get_humidity_command.cpp \
../src/ipc/get_hvac_value_command.cpp \
../src/ipc/get_ic_value_command.cpp \
../src/ipc/get_pn_limit_command.cpp \
../src/ipc/get_relay_value_command.cpp \
../src/ipc/get_rgbw_command.cpp \
../src/ipc/get_scene_summary_command.cpp \
../src/ipc/get_status_command.cpp \
../src/ipc/get_temperature_command.cpp \
../src/ipc/get_valve_value_command.cpp \
../src/ipc/gh_cred_changed_command.cpp \
../src/ipc/identify_device_command.cpp \
../src/ipc/identify_subdevice_command.cpp \
../src/ipc/interrupt_and_execute_scene_command.cpp \
../src/ipc/interrupt_scene_command.cpp \
../src/ipc/ipc_ctrl.cpp \
../src/ipc/ipc_socket_adapter.cpp \
../src/ipc/is_channel_connected_command.cpp \
../src/ipc/is_client_connected_command.cpp \
../src/ipc/is_device_connected_command.cpp \
../src/ipc/on_channel_config_changed_command.cpp \
../src/ipc/on_channel_deleted_command.cpp \
../src/ipc/on_device_config_changed_command.cpp \
../src/ipc/on_device_deleted_command.cpp \
../src/ipc/on_mqtt_settings_changed_command.cpp \
../src/ipc/on_scene_added_command.cpp \
../src/ipc/on_scene_changed_command.cpp \
../src/ipc/on_scene_removed_command.cpp \
../src/ipc/on_state_webhook_changed_command.cpp \
../src/ipc/on_user_settings_changed_command.cpp \
../src/ipc/on_vbt_changed_command.cpp \
../src/ipc/pair_subdevice_command.cpp \
../src/ipc/recalibrate_command.cpp \
../src/ipc/reset_counters_command.cpp \
../src/ipc/restart_device_command.cpp \
../src/ipc/restart_subdevice_command.cpp \
../src/ipc/send_push_command.cpp \
../src/ipc/set_cg_char_command.cpp \
../src/ipc/set_cg_rgbw_command.cpp \
../src/ipc/set_char_command.cpp \
../src/ipc/set_digiglass_value_command.cpp \
../src/ipc/set_rgbw_command.cpp \
../src/ipc/take_ocr_photo_command.cpp \
../src/ipc/user_reconnect_command.cpp 

C_SRCS += \
../src/ipc/ipcsocket.c 

CPP_DEPS += \
./src/ipc/abstract_action_cg_command.d \
./src/ipc/abstract_action_command.d \
./src/ipc/abstract_alexa_cred_changed_command.d \
./src/ipc/abstract_before_channel_function_change_command.d \
./src/ipc/abstract_before_device_delete_command.d \
./src/ipc/abstract_client_reconnect_command.d \
./src/ipc/abstract_device_set_time_command.d \
./src/ipc/abstract_enter_cfg_mode_command.d \
./src/ipc/abstract_execute_scene_command.d \
./src/ipc/abstract_get_char_command.d \
./src/ipc/abstract_get_digiglass_value_command.d \
./src/ipc/abstract_get_double_command.d \
./src/ipc/abstract_get_em_value_command.d \
./src/ipc/abstract_get_facade_blind_value_command.d \
./src/ipc/abstract_get_gpm_value_command.d \
./src/ipc/abstract_get_humidity_command.d \
./src/ipc/abstract_get_hvac_value_command.d \
./src/ipc/abstract_get_ic_value_command.d \
./src/ipc/abstract_get_pn_limit_command.d \
./src/ipc/abstract_get_relay_value_command.d \
./src/ipc/abstract_get_rgbw_command.d \
./src/ipc/abstract_get_scene_summary_command.d \
./src/ipc/abstract_get_status_command.d \
./src/ipc/abstract_get_temperature_command.d \
./src/ipc/abstract_get_valve_value_command.d \
./src/ipc/abstract_gh_cred_changed_command.d \
./src/ipc/abstract_identify_device_command.d \
./src/ipc/abstract_identify_subdevice_command.d \
./src/ipc/abstract_interrupt_and_execute_scene_command.d \
./src/ipc/abstract_interrupt_scene_command.d \
./src/ipc/abstract_ipc_command.d \
./src/ipc/abstract_ipc_ctrl.d \
./src/ipc/abstract_ipc_socket_adapter.d \
./src/ipc/abstract_is_channel_connected_command.d \
./src/ipc/abstract_is_client_connected_command.d \
./src/ipc/abstract_is_device_connected_command.d \
./src/ipc/abstract_on_channel_config_changed_command.d \
./src/ipc/abstract_on_channel_deleted_command.d \
./src/ipc/abstract_on_device_config_changed_command.d \
./src/ipc/abstract_on_device_deleted_command.d \
./src/ipc/abstract_on_mqtt_settings_changed_command.d \
./src/ipc/abstract_on_scene_added_command.d \
./src/ipc/abstract_on_scene_changed_command.d \
./src/ipc/abstract_on_scene_removed_command.d \
./src/ipc/abstract_on_state_webhook_changed_command.d \
./src/ipc/abstract_on_user_settings_changed_command.d \
./src/ipc/abstract_on_vbt_changed_command.d \
./src/ipc/abstract_pair_subdevice_command.d \
./src/ipc/abstract_recalibrate_command.d \
./src/ipc/abstract_reset_counters_command.d \
./src/ipc/abstract_restart_device_command.d \
./src/ipc/abstract_restart_subdevice_command.d \
./src/ipc/abstract_send_push_command.d \
./src/ipc/abstract_set_cg_char_command.d \
./src/ipc/abstract_set_cg_rgbw_command.d \
./src/ipc/abstract_set_char_command.d \
./src/ipc/abstract_set_digiglass_value_command.d \
./src/ipc/abstract_set_rgbw_command.d \
./src/ipc/abstract_take_ocr_photo_command.d \
./src/ipc/abstract_user_reconnect_command.d \
./src/ipc/action_cg_command.d \
./src/ipc/action_command.d \
./src/ipc/alexa_cred_changed_command.d \
./src/ipc/before_channel_function_change_command.d \
./src/ipc/before_device_delete_command.d \
./src/ipc/client_reconnect_command.d \
./src/ipc/device_set_time_command.d \
./src/ipc/enter_cfg_mode_command.d \
./src/ipc/execute_scene_command.d \
./src/ipc/get_char_command.d \
./src/ipc/get_digiglass_value_command.d \
./src/ipc/get_double_command.d \
./src/ipc/get_em_value_command.d \
./src/ipc/get_facade_blind_value_command.d \
./src/ipc/get_gpm_value_command.d \
./src/ipc/get_humidity_command.d \
./src/ipc/get_hvac_value_command.d \
./src/ipc/get_ic_value_command.d \
./src/ipc/get_pn_limit_command.d \
./src/ipc/get_relay_value_command.d \
./src/ipc/get_rgbw_command.d \
./src/ipc/get_scene_summary_command.d \
./src/ipc/get_status_command.d \
./src/ipc/get_temperature_command.d \
./src/ipc/get_valve_value_command.d \
./src/ipc/gh_cred_changed_command.d \
./src/ipc/identify_device_command.d \
./src/ipc/identify_subdevice_command.d \
./src/ipc/interrupt_and_execute_scene_command.d \
./src/ipc/interrupt_scene_command.d \
./src/ipc/ipc_ctrl.d \
./src/ipc/ipc_socket_adapter.d \
./src/ipc/is_channel_connected_command.d \
./src/ipc/is_client_connected_command.d \
./src/ipc/is_device_connected_command.d \
./src/ipc/on_channel_config_changed_command.d \
./src/ipc/on_channel_deleted_command.d \
./src/ipc/on_device_config_changed_command.d \
./src/ipc/on_device_deleted_command.d \
./src/ipc/on_mqtt_settings_changed_command.d \
./src/ipc/on_scene_added_command.d \
./src/ipc/on_scene_changed_command.d \
./src/ipc/on_scene_removed_command.d \
./src/ipc/on_state_webhook_changed_command.d \
./src/ipc/on_user_settings_changed_command.d \
./src/ipc/on_vbt_changed_command.d \
./src/ipc/pair_subdevice_command.d \
./src/ipc/recalibrate_command.d \
./src/ipc/reset_counters_command.d \
./src/ipc/restart_device_command.d \
./src/ipc/restart_subdevice_command.d \
./src/ipc/send_push_command.d \
./src/ipc/set_cg_char_command.d \
./src/ipc/set_cg_rgbw_command.d \
./src/ipc/set_char_command.d \
./src/ipc/set_digiglass_value_command.d \
./src/ipc/set_rgbw_command.d \
./src/ipc/take_ocr_photo_command.d \
./src/ipc/user_reconnect_command.d 

C_DEPS += \
./src/ipc/ipcsocket.d 

OBJS += \
./src/ipc/abstract_action_cg_command.o \
./src/ipc/abstract_action_command.o \
./src/ipc/abstract_alexa_cred_changed_command.o \
./src/ipc/abstract_before_channel_function_change_command.o \
./src/ipc/abstract_before_device_delete_command.o \
./src/ipc/abstract_client_reconnect_command.o \
./src/ipc/abstract_device_set_time_command.o \
./src/ipc/abstract_enter_cfg_mode_command.o \
./src/ipc/abstract_execute_scene_command.o \
./src/ipc/abstract_get_char_command.o \
./src/ipc/abstract_get_digiglass_value_command.o \
./src/ipc/abstract_get_double_command.o \
./src/ipc/abstract_get_em_value_command.o \
./src/ipc/abstract_get_facade_blind_value_command.o \
./src/ipc/abstract_get_gpm_value_command.o \
./src/ipc/abstract_get_humidity_command.o \
./src/ipc/abstract_get_hvac_value_command.o \
./src/ipc/abstract_get_ic_value_command.o \
./src/ipc/abstract_get_pn_limit_command.o \
./src/ipc/abstract_get_relay_value_command.o \
./src/ipc/abstract_get_rgbw_command.o \
./src/ipc/abstract_get_scene_summary_command.o \
./src/ipc/abstract_get_status_command.o \
./src/ipc/abstract_get_temperature_command.o \
./src/ipc/abstract_get_valve_value_command.o \
./src/ipc/abstract_gh_cred_changed_command.o \
./src/ipc/abstract_identify_device_command.o \
./src/ipc/abstract_identify_subdevice_command.o \
./src/ipc/abstract_interrupt_and_execute_scene_command.o \
./src/ipc/abstract_interrupt_scene_command.o \
./src/ipc/abstract_ipc_command.o \
./src/ipc/abstract_ipc_ctrl.o \
./src/ipc/abstract_ipc_socket_adapter.o \
./src/ipc/abstract_is_channel_connected_command.o \
./src/ipc/abstract_is_client_connected_command.o \
./src/ipc/abstract_is_device_connected_command.o \
./src/ipc/abstract_on_channel_config_changed_command.o \
./src/ipc/abstract_on_channel_deleted_command.o \
./src/ipc/abstract_on_device_config_changed_command.o \
./src/ipc/abstract_on_device_deleted_command.o \
./src/ipc/abstract_on_mqtt_settings_changed_command.o \
./src/ipc/abstract_on_scene_added_command.o \
./src/ipc/abstract_on_scene_changed_command.o \
./src/ipc/abstract_on_scene_removed_command.o \
./src/ipc/abstract_on_state_webhook_changed_command.o \
./src/ipc/abstract_on_user_settings_changed_command.o \
./src/ipc/abstract_on_vbt_changed_command.o \
./src/ipc/abstract_pair_subdevice_command.o \
./src/ipc/abstract_recalibrate_command.o \
./src/ipc/abstract_reset_counters_command.o \
./src/ipc/abstract_restart_device_command.o \
./src/ipc/abstract_restart_subdevice_command.o \
./src/ipc/abstract_send_push_command.o \
./src/ipc/abstract_set_cg_char_command.o \
./src/ipc/abstract_set_cg_rgbw_command.o \
./src/ipc/abstract_set_char_command.o \
./src/ipc/abstract_set_digiglass_value_command.o \
./src/ipc/abstract_set_rgbw_command.o \
./src/ipc/abstract_take_ocr_photo_command.o \
./src/ipc/abstract_user_reconnect_command.o \
./src/ipc/action_cg_command.o \
./src/ipc/action_command.o \
./src/ipc/alexa_cred_changed_command.o \
./src/ipc/before_channel_function_change_command.o \
./src/ipc/before_device_delete_command.o \
./src/ipc/client_reconnect_command.o \
./src/ipc/device_set_time_command.o \
./src/ipc/enter_cfg_mode_command.o \
./src/ipc/execute_scene_command.o \
./src/ipc/get_char_command.o \
./src/ipc/get_digiglass_value_command.o \
./src/ipc/get_double_command.o \
./src/ipc/get_em_value_command.o \
./src/ipc/get_facade_blind_value_command.o \
./src/ipc/get_gpm_value_command.o \
./src/ipc/get_humidity_command.o \
./src/ipc/get_hvac_value_command.o \
./src/ipc/get_ic_value_command.o \
./src/ipc/get_pn_limit_command.o \
./src/ipc/get_relay_value_command.o \
./src/ipc/get_rgbw_command.o \
./src/ipc/get_scene_summary_command.o \
./src/ipc/get_status_command.o \
./src/ipc/get_temperature_command.o \
./src/ipc/get_valve_value_command.o \
./src/ipc/gh_cred_changed_command.o \
./src/ipc/identify_device_command.o \
./src/ipc/identify_subdevice_command.o \
./src/ipc/interrupt_and_execute_scene_command.o \
./src/ipc/interrupt_scene_command.o \
./src/ipc/ipc_ctrl.o \
./src/ipc/ipc_socket_adapter.o \
./src/ipc/ipcsocket.o \
./src/ipc/is_channel_connected_command.o \
./src/ipc/is_client_connected_command.o \
./src/ipc/is_device_connected_command.o \
./src/ipc/on_channel_config_changed_command.o \
./src/ipc/on_channel_deleted_command.o \
./src/ipc/on_device_config_changed_command.o \
./src/ipc/on_device_deleted_command.o \
./src/ipc/on_mqtt_settings_changed_command.o \
./src/ipc/on_scene_added_command.o \
./src/ipc/on_scene_changed_command.o \
./src/ipc/on_scene_removed_command.o \
./src/ipc/on_state_webhook_changed_command.o \
./src/ipc/on_user_settings_changed_command.o \
./src/ipc/on_vbt_changed_command.o \
./src/ipc/pair_subdevice_command.o \
./src/ipc/recalibrate_command.o \
./src/ipc/reset_counters_command.o \
./src/ipc/restart_device_command.o \
./src/ipc/restart_subdevice_command.o \
./src/ipc/send_push_command.o \
./src/ipc/set_cg_char_command.o \
./src/ipc/set_cg_rgbw_command.o \
./src/ipc/set_char_command.o \
./src/ipc/set_digiglass_value_command.o \
./src/ipc/set_rgbw_command.o \
./src/ipc/take_ocr_photo_command.o \
./src/ipc/user_reconnect_command.o 


# Each subdirectory must supply rules for building sources it contributes
src/ipc/%.o: ../src/ipc/%.cpp src/ipc/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ipc/%.o: ../src/ipc/%.c src/ipc/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	$(CC) -D__OPENSSL_TOOLS=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__DEBUG=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(SSLDIR)/include -I../src/mqtt -I../src/user -I../src/device -I../src/client -I../src -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all  -D_FORTIFY_SOURCE=2 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-ipc

clean-src-2f-ipc:
	-$(RM) ./src/ipc/abstract_action_cg_command.d ./src/ipc/abstract_action_cg_command.o ./src/ipc/abstract_action_command.d ./src/ipc/abstract_action_command.o ./src/ipc/abstract_alexa_cred_changed_command.d ./src/ipc/abstract_alexa_cred_changed_command.o ./src/ipc/abstract_before_channel_function_change_command.d ./src/ipc/abstract_before_channel_function_change_command.o ./src/ipc/abstract_before_device_delete_command.d ./src/ipc/abstract_before_device_delete_command.o ./src/ipc/abstract_client_reconnect_command.d ./src/ipc/abstract_client_reconnect_command.o ./src/ipc/abstract_device_set_time_command.d ./src/ipc/abstract_device_set_time_command.o ./src/ipc/abstract_enter_cfg_mode_command.d ./src/ipc/abstract_enter_cfg_mode_command.o ./src/ipc/abstract_execute_scene_command.d ./src/ipc/abstract_execute_scene_command.o ./src/ipc/abstract_get_char_command.d ./src/ipc/abstract_get_char_command.o ./src/ipc/abstract_get_digiglass_value_command.d ./src/ipc/abstract_get_digiglass_value_command.o ./src/ipc/abstract_get_double_command.d ./src/ipc/abstract_get_double_command.o ./src/ipc/abstract_get_em_value_command.d ./src/ipc/abstract_get_em_value_command.o ./src/ipc/abstract_get_facade_blind_value_command.d ./src/ipc/abstract_get_facade_blind_value_command.o ./src/ipc/abstract_get_gpm_value_command.d ./src/ipc/abstract_get_gpm_value_command.o ./src/ipc/abstract_get_humidity_command.d ./src/ipc/abstract_get_humidity_command.o ./src/ipc/abstract_get_hvac_value_command.d ./src/ipc/abstract_get_hvac_value_command.o ./src/ipc/abstract_get_ic_value_command.d ./src/ipc/abstract_get_ic_value_command.o ./src/ipc/abstract_get_pn_limit_command.d ./src/ipc/abstract_get_pn_limit_command.o ./src/ipc/abstract_get_relay_value_command.d ./src/ipc/abstract_get_relay_value_command.o ./src/ipc/abstract_get_rgbw_command.d ./src/ipc/abstract_get_rgbw_command.o ./src/ipc/abstract_get_scene_summary_command.d ./src/ipc/abstract_get_scene_summary_command.o ./src/ipc/abstract_get_status_command.d ./src/ipc/abstract_get_status_command.o ./src/ipc/abstract_get_temperature_command.d ./src/ipc/abstract_get_temperature_command.o ./src/ipc/abstract_get_valve_value_command.d ./src/ipc/abstract_get_valve_value_command.o ./src/ipc/abstract_gh_cred_changed_command.d ./src/ipc/abstract_gh_cred_changed_command.o ./src/ipc/abstract_identify_device_command.d ./src/ipc/abstract_identify_device_command.o ./src/ipc/abstract_identify_subdevice_command.d ./src/ipc/abstract_identify_subdevice_command.o ./src/ipc/abstract_interrupt_and_execute_scene_command.d ./src/ipc/abstract_interrupt_and_execute_scene_command.o ./src/ipc/abstract_interrupt_scene_command.d ./src/ipc/abstract_interrupt_scene_command.o ./src/ipc/abstract_ipc_command.d ./src/ipc/abstract_ipc_command.o ./src/ipc/abstract_ipc_ctrl.d ./src/ipc/abstract_ipc_ctrl.o ./src/ipc/abstract_ipc_socket_adapter.d ./src/ipc/abstract_ipc_socket_adapter.o ./src/ipc/abstract_is_channel_connected_command.d ./src/ipc/abstract_is_channel_connected_command.o ./src/ipc/abstract_is_client_connected_command.d ./src/ipc/abstract_is_client_connected_command.o ./src/ipc/abstract_is_device_connected_command.d ./src/ipc/abstract_is_device_connected_command.o ./src/ipc/abstract_on_channel_config_changed_command.d ./src/ipc/abstract_on_channel_config_changed_command.o ./src/ipc/abstract_on_channel_deleted_command.d ./src/ipc/abstract_on_channel_deleted_command.o ./src/ipc/abstract_on_device_config_changed_command.d ./src/ipc/abstract_on_device_config_changed_command.o ./src/ipc/abstract_on_device_deleted_command.d ./src/ipc/abstract_on_device_deleted_command.o ./src/ipc/abstract_on_mqtt_settings_changed_command.d ./src/ipc/abstract_on_mqtt_settings_changed_command.o ./src/ipc/abstract_on_scene_added_command.d ./src/ipc/abstract_on_scene_added_command.o ./src/ipc/abstract_on_scene_changed_command.d ./src/ipc/abstract_on_scene_changed_command.o ./src/ipc/abstract_on_scene_removed_command.d ./src/ipc/abstract_on_scene_removed_command.o ./src/ipc/abstract_on_state_webhook_changed_command.d ./src/ipc/abstract_on_state_webhook_changed_command.o ./src/ipc/abstract_on_user_settings_changed_command.d ./src/ipc/abstract_on_user_settings_changed_command.o ./src/ipc/abstract_on_vbt_changed_command.d ./src/ipc/abstract_on_vbt_changed_command.o ./src/ipc/abstract_pair_subdevice_command.d ./src/ipc/abstract_pair_subdevice_command.o ./src/ipc/abstract_recalibrate_command.d ./src/ipc/abstract_recalibrate_command.o ./src/ipc/abstract_reset_counters_command.d ./src/ipc/abstract_reset_counters_command.o ./src/ipc/abstract_restart_device_command.d ./src/ipc/abstract_restart_device_command.o ./src/ipc/abstract_restart_subdevice_command.d ./src/ipc/abstract_restart_subdevice_command.o ./src/ipc/abstract_send_push_command.d ./src/ipc/abstract_send_push_command.o ./src/ipc/abstract_set_cg_char_command.d ./src/ipc/abstract_set_cg_char_command.o ./src/ipc/abstract_set_cg_rgbw_command.d ./src/ipc/abstract_set_cg_rgbw_command.o ./src/ipc/abstract_set_char_command.d ./src/ipc/abstract_set_char_command.o ./src/ipc/abstract_set_digiglass_value_command.d ./src/ipc/abstract_set_digiglass_value_command.o ./src/ipc/abstract_set_rgbw_command.d ./src/ipc/abstract_set_rgbw_command.o ./src/ipc/abstract_take_ocr_photo_command.d ./src/ipc/abstract_take_ocr_photo_command.o ./src/ipc/abstract_user_reconnect_command.d ./src/ipc/abstract_user_reconnect_command.o ./src/ipc/action_cg_command.d ./src/ipc/action_cg_command.o ./src/ipc/action_command.d ./src/ipc/action_command.o ./src/ipc/alexa_cred_changed_command.d ./src/ipc/alexa_cred_changed_command.o ./src/ipc/before_channel_function_change_command.d ./src/ipc/before_channel_function_change_command.o ./src/ipc/before_device_delete_command.d ./src/ipc/before_device_delete_command.o ./src/ipc/client_reconnect_command.d ./src/ipc/client_reconnect_command.o ./src/ipc/device_set_time_command.d ./src/ipc/device_set_time_command.o ./src/ipc/enter_cfg_mode_command.d ./src/ipc/enter_cfg_mode_command.o
	-$(RM) ./src/ipc/execute_scene_command.d ./src/ipc/execute_scene_command.o ./src/ipc/get_char_command.d ./src/ipc/get_char_command.o ./src/ipc/get_digiglass_value_command.d ./src/ipc/get_digiglass_value_command.o ./src/ipc/get_double_command.d ./src/ipc/get_double_command.o ./src/ipc/get_em_value_command.d ./src/ipc/get_em_value_command.o ./src/ipc/get_facade_blind_value_command.d ./src/ipc/get_facade_blind_value_command.o ./src/ipc/get_gpm_value_command.d ./src/ipc/get_gpm_value_command.o ./src/ipc/get_humidity_command.d ./src/ipc/get_humidity_command.o ./src/ipc/get_hvac_value_command.d ./src/ipc/get_hvac_value_command.o ./src/ipc/get_ic_value_command.d ./src/ipc/get_ic_value_command.o ./src/ipc/get_pn_limit_command.d ./src/ipc/get_pn_limit_command.o ./src/ipc/get_relay_value_command.d ./src/ipc/get_relay_value_command.o ./src/ipc/get_rgbw_command.d ./src/ipc/get_rgbw_command.o ./src/ipc/get_scene_summary_command.d ./src/ipc/get_scene_summary_command.o ./src/ipc/get_status_command.d ./src/ipc/get_status_command.o ./src/ipc/get_temperature_command.d ./src/ipc/get_temperature_command.o ./src/ipc/get_valve_value_command.d ./src/ipc/get_valve_value_command.o ./src/ipc/gh_cred_changed_command.d ./src/ipc/gh_cred_changed_command.o ./src/ipc/identify_device_command.d ./src/ipc/identify_device_command.o ./src/ipc/identify_subdevice_command.d ./src/ipc/identify_subdevice_command.o ./src/ipc/interrupt_and_execute_scene_command.d ./src/ipc/interrupt_and_execute_scene_command.o ./src/ipc/interrupt_scene_command.d ./src/ipc/interrupt_scene_command.o ./src/ipc/ipc_ctrl.d ./src/ipc/ipc_ctrl.o ./src/ipc/ipc_socket_adapter.d ./src/ipc/ipc_socket_adapter.o ./src/ipc/ipcsocket.d ./src/ipc/ipcsocket.o ./src/ipc/is_channel_connected_command.d ./src/ipc/is_channel_connected_command.o ./src/ipc/is_client_connected_command.d ./src/ipc/is_client_connected_command.o ./src/ipc/is_device_connected_command.d ./src/ipc/is_device_connected_command.o ./src/ipc/on_channel_config_changed_command.d ./src/ipc/on_channel_config_changed_command.o ./src/ipc/on_channel_deleted_command.d ./src/ipc/on_channel_deleted_command.o ./src/ipc/on_device_config_changed_command.d ./src/ipc/on_device_config_changed_command.o ./src/ipc/on_device_deleted_command.d ./src/ipc/on_device_deleted_command.o ./src/ipc/on_mqtt_settings_changed_command.d ./src/ipc/on_mqtt_settings_changed_command.o ./src/ipc/on_scene_added_command.d ./src/ipc/on_scene_added_command.o ./src/ipc/on_scene_changed_command.d ./src/ipc/on_scene_changed_command.o ./src/ipc/on_scene_removed_command.d ./src/ipc/on_scene_removed_command.o ./src/ipc/on_state_webhook_changed_command.d ./src/ipc/on_state_webhook_changed_command.o ./src/ipc/on_user_settings_changed_command.d ./src/ipc/on_user_settings_changed_command.o ./src/ipc/on_vbt_changed_command.d ./src/ipc/on_vbt_changed_command.o ./src/ipc/pair_subdevice_command.d ./src/ipc/pair_subdevice_command.o ./src/ipc/recalibrate_command.d ./src/ipc/recalibrate_command.o ./src/ipc/reset_counters_command.d ./src/ipc/reset_counters_command.o ./src/ipc/restart_device_command.d ./src/ipc/restart_device_command.o ./src/ipc/restart_subdevice_command.d ./src/ipc/restart_subdevice_command.o ./src/ipc/send_push_command.d ./src/ipc/send_push_command.o ./src/ipc/set_cg_char_command.d ./src/ipc/set_cg_char_command.o ./src/ipc/set_cg_rgbw_command.d ./src/ipc/set_cg_rgbw_command.o ./src/ipc/set_char_command.d ./src/ipc/set_char_command.o ./src/ipc/set_digiglass_value_command.d ./src/ipc/set_digiglass_value_command.o ./src/ipc/set_rgbw_command.d ./src/ipc/set_rgbw_command.o ./src/ipc/take_ocr_photo_command.d ./src/ipc/take_ocr_photo_command.o ./src/ipc/user_reconnect_command.d ./src/ipc/user_reconnect_command.o

.PHONY: clean-src-2f-ipc

