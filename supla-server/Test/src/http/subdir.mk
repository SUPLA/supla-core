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
../src/http/http_event_hub.cpp \
../src/http/oauth_credentials.cpp \
../src/http/voice_assistant_client.cpp 

CPP_DEPS += \
./src/http/abstract_curl_adapter.d \
./src/http/asynctask_http_request.d \
./src/http/asynctask_http_thread_bucket.d \
./src/http/asynctask_http_thread_pool.d \
./src/http/curl_adapter.d \
./src/http/http_event_hub.d \
./src/http/oauth_credentials.d \
./src/http/voice_assistant_client.d 

OBJS += \
./src/http/abstract_curl_adapter.o \
./src/http/asynctask_http_request.o \
./src/http/asynctask_http_thread_bucket.o \
./src/http/asynctask_http_thread_pool.o \
./src/http/curl_adapter.o \
./src/http/http_event_hub.o \
./src/http/oauth_credentials.o \
./src/http/voice_assistant_client.o 


# Each subdirectory must supply rules for building sources it contributes
src/http/%.o: ../src/http/%.cpp src/http/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-http

clean-src-2f-http:
	-$(RM) ./src/http/abstract_curl_adapter.d ./src/http/abstract_curl_adapter.o ./src/http/asynctask_http_request.d ./src/http/asynctask_http_request.o ./src/http/asynctask_http_thread_bucket.d ./src/http/asynctask_http_thread_bucket.o ./src/http/asynctask_http_thread_pool.d ./src/http/asynctask_http_thread_pool.o ./src/http/curl_adapter.d ./src/http/curl_adapter.o ./src/http/http_event_hub.d ./src/http/http_event_hub.o ./src/http/oauth_credentials.d ./src/http/oauth_credentials.o ./src/http/voice_assistant_client.d ./src/http/voice_assistant_client.o

.PHONY: clean-src-2f-http

