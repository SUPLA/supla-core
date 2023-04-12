################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/push/apns_client.cpp \
../src/push/fcm_client.cpp \
../src/push/push_notification.cpp \
../src/push/push_notification_delivery_task.cpp \
../src/push/push_notification_gateway_credentials.cpp \
../src/push/push_notification_recipient.cpp \
../src/push/push_notification_recipients.cpp \
../src/push/push_notification_throttling.cpp 

CPP_DEPS += \
./src/push/apns_client.d \
./src/push/fcm_client.d \
./src/push/push_notification.d \
./src/push/push_notification_delivery_task.d \
./src/push/push_notification_gateway_credentials.d \
./src/push/push_notification_recipient.d \
./src/push/push_notification_recipients.d \
./src/push/push_notification_throttling.d 

OBJS += \
./src/push/apns_client.o \
./src/push/fcm_client.o \
./src/push/push_notification.o \
./src/push/push_notification_delivery_task.o \
./src/push/push_notification_gateway_credentials.o \
./src/push/push_notification_recipient.o \
./src/push/push_notification_recipients.o \
./src/push/push_notification_throttling.o 


# Each subdirectory must supply rules for building sources it contributes
src/push/%.o: ../src/push/%.cpp src/push/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-push

clean-src-2f-push:
	-$(RM) ./src/push/apns_client.d ./src/push/apns_client.o ./src/push/fcm_client.d ./src/push/fcm_client.o ./src/push/push_notification.d ./src/push/push_notification.o ./src/push/push_notification_delivery_task.d ./src/push/push_notification_delivery_task.o ./src/push/push_notification_gateway_credentials.d ./src/push/push_notification_gateway_credentials.o ./src/push/push_notification_recipient.d ./src/push/push_notification_recipient.o ./src/push/push_notification_recipients.d ./src/push/push_notification_recipients.o ./src/push/push_notification_throttling.d ./src/push/push_notification_throttling.o

.PHONY: clean-src-2f-push

