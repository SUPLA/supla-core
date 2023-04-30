################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/push/AccessTokenProviderTest.cpp \
../src/test/push/AccessTokenTest.cpp \
../src/test/push/DeliveryTaskTest.cpp \
../src/test/push/PushNotificationRecipientsTest.cpp \
../src/test/push/PushNotificationTest.cpp 

CPP_DEPS += \
./src/test/push/AccessTokenProviderTest.d \
./src/test/push/AccessTokenTest.d \
./src/test/push/DeliveryTaskTest.d \
./src/test/push/PushNotificationRecipientsTest.d \
./src/test/push/PushNotificationTest.d 

OBJS += \
./src/test/push/AccessTokenProviderTest.o \
./src/test/push/AccessTokenTest.o \
./src/test/push/DeliveryTaskTest.o \
./src/test/push/PushNotificationRecipientsTest.o \
./src/test/push/PushNotificationTest.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/push/%.o: ../src/test/push/%.cpp src/test/push/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-push

clean-src-2f-test-2f-push:
	-$(RM) ./src/test/push/AccessTokenProviderTest.d ./src/test/push/AccessTokenProviderTest.o ./src/test/push/AccessTokenTest.d ./src/test/push/AccessTokenTest.o ./src/test/push/DeliveryTaskTest.d ./src/test/push/DeliveryTaskTest.o ./src/test/push/PushNotificationRecipientsTest.d ./src/test/push/PushNotificationRecipientsTest.o ./src/test/push/PushNotificationTest.d ./src/test/push/PushNotificationTest.o

.PHONY: clean-src-2f-test-2f-push

