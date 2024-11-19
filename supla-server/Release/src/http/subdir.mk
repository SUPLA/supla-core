################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/http/abstract_curl_adapter.cpp \
../src/http/asynctask_http_request.cpp \
../src/http/asynctask_http_thread_bucket.cpp \
../src/http/asynctask_http_thread_pool.cpp \
../src/http/curl_adapter.cpp \
../src/http/curl_instance.cpp \
../src/http/http_event_hub.cpp \
../src/http/http_throttling.cpp \
../src/http/oauth_credentials.cpp \
../src/http/remote_gateway_access_token.cpp \
../src/http/remote_gateway_access_token_provider.cpp \
../src/http/voice_assistant_client.cpp 

CPP_DEPS += \
./src/http/abstract_curl_adapter.d \
./src/http/asynctask_http_request.d \
./src/http/asynctask_http_thread_bucket.d \
./src/http/asynctask_http_thread_pool.d \
./src/http/curl_adapter.d \
./src/http/curl_instance.d \
./src/http/http_event_hub.d \
./src/http/http_throttling.d \
./src/http/oauth_credentials.d \
./src/http/remote_gateway_access_token.d \
./src/http/remote_gateway_access_token_provider.d \
./src/http/voice_assistant_client.d 

OBJS += \
./src/http/abstract_curl_adapter.o \
./src/http/asynctask_http_request.o \
./src/http/asynctask_http_thread_bucket.o \
./src/http/asynctask_http_thread_pool.o \
./src/http/curl_adapter.o \
./src/http/curl_instance.o \
./src/http/http_event_hub.o \
./src/http/http_throttling.o \
./src/http/oauth_credentials.o \
./src/http/remote_gateway_access_token.o \
./src/http/remote_gateway_access_token_provider.o \
./src/http/voice_assistant_client.o 


# Each subdirectory must supply rules for building sources it contributes
src/http/%.o: ../src/http/%.cpp src/http/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O2 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-http

clean-src-2f-http:
	-$(RM) ./src/http/abstract_curl_adapter.d ./src/http/abstract_curl_adapter.o ./src/http/asynctask_http_request.d ./src/http/asynctask_http_request.o ./src/http/asynctask_http_thread_bucket.d ./src/http/asynctask_http_thread_bucket.o ./src/http/asynctask_http_thread_pool.d ./src/http/asynctask_http_thread_pool.o ./src/http/curl_adapter.d ./src/http/curl_adapter.o ./src/http/curl_instance.d ./src/http/curl_instance.o ./src/http/http_event_hub.d ./src/http/http_event_hub.o ./src/http/http_throttling.d ./src/http/http_throttling.o ./src/http/oauth_credentials.d ./src/http/oauth_credentials.o ./src/http/remote_gateway_access_token.d ./src/http/remote_gateway_access_token.o ./src/http/remote_gateway_access_token_provider.d ./src/http/remote_gateway_access_token_provider.o ./src/http/voice_assistant_client.d ./src/http/voice_assistant_client.o

.PHONY: clean-src-2f-http

