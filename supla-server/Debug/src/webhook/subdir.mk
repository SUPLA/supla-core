################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/webhook/state_webhook_client.cpp \
../src/webhook/state_webhook_credentials.cpp \
../src/webhook/state_webhook_credentials_dao.cpp \
../src/webhook/state_webhook_request.cpp \
../src/webhook/state_webhook_search_condition.cpp 

CPP_DEPS += \
./src/webhook/state_webhook_client.d \
./src/webhook/state_webhook_credentials.d \
./src/webhook/state_webhook_credentials_dao.d \
./src/webhook/state_webhook_request.d \
./src/webhook/state_webhook_search_condition.d 

OBJS += \
./src/webhook/state_webhook_client.o \
./src/webhook/state_webhook_credentials.o \
./src/webhook/state_webhook_credentials_dao.o \
./src/webhook/state_webhook_request.o \
./src/webhook/state_webhook_search_condition.o 


# Each subdirectory must supply rules for building sources it contributes
src/webhook/%.o: ../src/webhook/%.cpp src/webhook/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-webhook

clean-src-2f-webhook:
	-$(RM) ./src/webhook/state_webhook_client.d ./src/webhook/state_webhook_client.o ./src/webhook/state_webhook_credentials.d ./src/webhook/state_webhook_credentials.o ./src/webhook/state_webhook_credentials_dao.d ./src/webhook/state_webhook_credentials_dao.o ./src/webhook/state_webhook_request.d ./src/webhook/state_webhook_request.o ./src/webhook/state_webhook_search_condition.d ./src/webhook/state_webhook_search_condition.o

.PHONY: clean-src-2f-webhook

