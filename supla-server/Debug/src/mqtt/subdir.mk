################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/mqtt/mqtt_abstract_action_message_provider.cpp \
../src/mqtt/mqtt_abstract_state_message_provider.cpp \
../src/mqtt/mqtt_abstract_value_setter.cpp \
../src/mqtt/mqtt_action_message_provider.cpp \
../src/mqtt/mqtt_channel_message_provider.cpp \
../src/mqtt/mqtt_channelandstate_message_provider.cpp \
../src/mqtt/mqtt_channelandstate_removed_topics_provider.cpp \
../src/mqtt/mqtt_client.cpp \
../src/mqtt/mqtt_client_datasource.cpp \
../src/mqtt/mqtt_client_db_datasource.cpp \
../src/mqtt/mqtt_client_ini_settings.cpp \
../src/mqtt/mqtt_client_library_adapter.cpp \
../src/mqtt/mqtt_client_settings.cpp \
../src/mqtt/mqtt_client_suite.cpp \
../src/mqtt/mqtt_db.cpp \
../src/mqtt/mqtt_device_message_provider.cpp \
../src/mqtt/mqtt_ds_context.cpp \
../src/mqtt/mqtt_message_provider.cpp \
../src/mqtt/mqtt_publisher.cpp \
../src/mqtt/mqtt_publisher_datasource.cpp \
../src/mqtt/mqtt_removed_topics_provider.cpp \
../src/mqtt/mqtt_state_message_provider.cpp \
../src/mqtt/mqtt_subscriber.cpp \
../src/mqtt/mqtt_subscriber_datasource.cpp \
../src/mqtt/mqtt_subscription_topic_provider.cpp \
../src/mqtt/mqtt_topic_cmp.cpp \
../src/mqtt/mqtt_unpublisher.cpp \
../src/mqtt/mqtt_unpublisher_datasource.cpp \
../src/mqtt/mqtt_unpublisher_user_topic_provider.cpp \
../src/mqtt/mqtt_user_message_provider.cpp \
../src/mqtt/mqtt_value_setter.cpp \
../src/mqtt/mqttc_library_adapter.cpp 

C_SRCS += \
../src/mqtt/mqtt.c 

CPP_DEPS += \
./src/mqtt/mqtt_abstract_action_message_provider.d \
./src/mqtt/mqtt_abstract_state_message_provider.d \
./src/mqtt/mqtt_abstract_value_setter.d \
./src/mqtt/mqtt_action_message_provider.d \
./src/mqtt/mqtt_channel_message_provider.d \
./src/mqtt/mqtt_channelandstate_message_provider.d \
./src/mqtt/mqtt_channelandstate_removed_topics_provider.d \
./src/mqtt/mqtt_client.d \
./src/mqtt/mqtt_client_datasource.d \
./src/mqtt/mqtt_client_db_datasource.d \
./src/mqtt/mqtt_client_ini_settings.d \
./src/mqtt/mqtt_client_library_adapter.d \
./src/mqtt/mqtt_client_settings.d \
./src/mqtt/mqtt_client_suite.d \
./src/mqtt/mqtt_db.d \
./src/mqtt/mqtt_device_message_provider.d \
./src/mqtt/mqtt_ds_context.d \
./src/mqtt/mqtt_message_provider.d \
./src/mqtt/mqtt_publisher.d \
./src/mqtt/mqtt_publisher_datasource.d \
./src/mqtt/mqtt_removed_topics_provider.d \
./src/mqtt/mqtt_state_message_provider.d \
./src/mqtt/mqtt_subscriber.d \
./src/mqtt/mqtt_subscriber_datasource.d \
./src/mqtt/mqtt_subscription_topic_provider.d \
./src/mqtt/mqtt_topic_cmp.d \
./src/mqtt/mqtt_unpublisher.d \
./src/mqtt/mqtt_unpublisher_datasource.d \
./src/mqtt/mqtt_unpublisher_user_topic_provider.d \
./src/mqtt/mqtt_user_message_provider.d \
./src/mqtt/mqtt_value_setter.d \
./src/mqtt/mqttc_library_adapter.d 

C_DEPS += \
./src/mqtt/mqtt.d 

OBJS += \
./src/mqtt/mqtt.o \
./src/mqtt/mqtt_abstract_action_message_provider.o \
./src/mqtt/mqtt_abstract_state_message_provider.o \
./src/mqtt/mqtt_abstract_value_setter.o \
./src/mqtt/mqtt_action_message_provider.o \
./src/mqtt/mqtt_channel_message_provider.o \
./src/mqtt/mqtt_channelandstate_message_provider.o \
./src/mqtt/mqtt_channelandstate_removed_topics_provider.o \
./src/mqtt/mqtt_client.o \
./src/mqtt/mqtt_client_datasource.o \
./src/mqtt/mqtt_client_db_datasource.o \
./src/mqtt/mqtt_client_ini_settings.o \
./src/mqtt/mqtt_client_library_adapter.o \
./src/mqtt/mqtt_client_settings.o \
./src/mqtt/mqtt_client_suite.o \
./src/mqtt/mqtt_db.o \
./src/mqtt/mqtt_device_message_provider.o \
./src/mqtt/mqtt_ds_context.o \
./src/mqtt/mqtt_message_provider.o \
./src/mqtt/mqtt_publisher.o \
./src/mqtt/mqtt_publisher_datasource.o \
./src/mqtt/mqtt_removed_topics_provider.o \
./src/mqtt/mqtt_state_message_provider.o \
./src/mqtt/mqtt_subscriber.o \
./src/mqtt/mqtt_subscriber_datasource.o \
./src/mqtt/mqtt_subscription_topic_provider.o \
./src/mqtt/mqtt_topic_cmp.o \
./src/mqtt/mqtt_unpublisher.o \
./src/mqtt/mqtt_unpublisher_datasource.o \
./src/mqtt/mqtt_unpublisher_user_topic_provider.o \
./src/mqtt/mqtt_user_message_provider.o \
./src/mqtt/mqtt_value_setter.o \
./src/mqtt/mqttc_library_adapter.o 


# Each subdirectory must supply rules for building sources it contributes
src/mqtt/%.o: ../src/mqtt/%.c src/mqtt/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	$(CC) -D__OPENSSL_TOOLS=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__DEBUG=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(SSLDIR)/include -I../src/mqtt -I../src/user -I../src/device -I../src/client -I../src -O3 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all  -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/mqtt/%.o: ../src/mqtt/%.cpp src/mqtt/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O3 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-mqtt

clean-src-2f-mqtt:
	-$(RM) ./src/mqtt/mqtt.d ./src/mqtt/mqtt.o ./src/mqtt/mqtt_abstract_action_message_provider.d ./src/mqtt/mqtt_abstract_action_message_provider.o ./src/mqtt/mqtt_abstract_state_message_provider.d ./src/mqtt/mqtt_abstract_state_message_provider.o ./src/mqtt/mqtt_abstract_value_setter.d ./src/mqtt/mqtt_abstract_value_setter.o ./src/mqtt/mqtt_action_message_provider.d ./src/mqtt/mqtt_action_message_provider.o ./src/mqtt/mqtt_channel_message_provider.d ./src/mqtt/mqtt_channel_message_provider.o ./src/mqtt/mqtt_channelandstate_message_provider.d ./src/mqtt/mqtt_channelandstate_message_provider.o ./src/mqtt/mqtt_channelandstate_removed_topics_provider.d ./src/mqtt/mqtt_channelandstate_removed_topics_provider.o ./src/mqtt/mqtt_client.d ./src/mqtt/mqtt_client.o ./src/mqtt/mqtt_client_datasource.d ./src/mqtt/mqtt_client_datasource.o ./src/mqtt/mqtt_client_db_datasource.d ./src/mqtt/mqtt_client_db_datasource.o ./src/mqtt/mqtt_client_ini_settings.d ./src/mqtt/mqtt_client_ini_settings.o ./src/mqtt/mqtt_client_library_adapter.d ./src/mqtt/mqtt_client_library_adapter.o ./src/mqtt/mqtt_client_settings.d ./src/mqtt/mqtt_client_settings.o ./src/mqtt/mqtt_client_suite.d ./src/mqtt/mqtt_client_suite.o ./src/mqtt/mqtt_db.d ./src/mqtt/mqtt_db.o ./src/mqtt/mqtt_device_message_provider.d ./src/mqtt/mqtt_device_message_provider.o ./src/mqtt/mqtt_ds_context.d ./src/mqtt/mqtt_ds_context.o ./src/mqtt/mqtt_message_provider.d ./src/mqtt/mqtt_message_provider.o ./src/mqtt/mqtt_publisher.d ./src/mqtt/mqtt_publisher.o ./src/mqtt/mqtt_publisher_datasource.d ./src/mqtt/mqtt_publisher_datasource.o ./src/mqtt/mqtt_removed_topics_provider.d ./src/mqtt/mqtt_removed_topics_provider.o ./src/mqtt/mqtt_state_message_provider.d ./src/mqtt/mqtt_state_message_provider.o ./src/mqtt/mqtt_subscriber.d ./src/mqtt/mqtt_subscriber.o ./src/mqtt/mqtt_subscriber_datasource.d ./src/mqtt/mqtt_subscriber_datasource.o ./src/mqtt/mqtt_subscription_topic_provider.d ./src/mqtt/mqtt_subscription_topic_provider.o ./src/mqtt/mqtt_topic_cmp.d ./src/mqtt/mqtt_topic_cmp.o ./src/mqtt/mqtt_unpublisher.d ./src/mqtt/mqtt_unpublisher.o ./src/mqtt/mqtt_unpublisher_datasource.d ./src/mqtt/mqtt_unpublisher_datasource.o ./src/mqtt/mqtt_unpublisher_user_topic_provider.d ./src/mqtt/mqtt_unpublisher_user_topic_provider.o ./src/mqtt/mqtt_user_message_provider.d ./src/mqtt/mqtt_user_message_provider.o ./src/mqtt/mqtt_value_setter.d ./src/mqtt/mqtt_value_setter.o ./src/mqtt/mqttc_library_adapter.d ./src/mqtt/mqttc_library_adapter.o

.PHONY: clean-src-2f-mqtt

