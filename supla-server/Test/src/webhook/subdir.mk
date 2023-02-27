################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/webhook/state_webhook_client2.cpp \
../src/webhook/state_webhook_credentials2.cpp \
../src/webhook/state_webhook_credentials_dao.cpp \
../src/webhook/state_webhook_request2.cpp \
../src/webhook/state_webhook_search_condition.cpp \
../src/webhook/webhookbasicclient.cpp \
../src/webhook/webhookbasiccredentials.cpp 

CPP_DEPS += \
./src/webhook/state_webhook_client2.d \
./src/webhook/state_webhook_credentials2.d \
./src/webhook/state_webhook_credentials_dao.d \
./src/webhook/state_webhook_request2.d \
./src/webhook/state_webhook_search_condition.d \
./src/webhook/webhookbasicclient.d \
./src/webhook/webhookbasiccredentials.d 

OBJS += \
./src/webhook/state_webhook_client2.o \
./src/webhook/state_webhook_credentials2.o \
./src/webhook/state_webhook_credentials_dao.o \
./src/webhook/state_webhook_request2.o \
./src/webhook/state_webhook_search_condition.o \
./src/webhook/webhookbasicclient.o \
./src/webhook/webhookbasiccredentials.o 


# Each subdirectory must supply rules for building sources it contributes
src/webhook/%.o: ../src/webhook/%.cpp src/webhook/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-webhook

clean-src-2f-webhook:
	-$(RM) ./src/webhook/state_webhook_client2.d ./src/webhook/state_webhook_client2.o ./src/webhook/state_webhook_credentials2.d ./src/webhook/state_webhook_credentials2.o ./src/webhook/state_webhook_credentials_dao.d ./src/webhook/state_webhook_credentials_dao.o ./src/webhook/state_webhook_request2.d ./src/webhook/state_webhook_request2.o ./src/webhook/state_webhook_search_condition.d ./src/webhook/state_webhook_search_condition.o ./src/webhook/webhookbasicclient.d ./src/webhook/webhookbasicclient.o ./src/webhook/webhookbasiccredentials.d ./src/webhook/webhookbasiccredentials.o

.PHONY: clean-src-2f-webhook

