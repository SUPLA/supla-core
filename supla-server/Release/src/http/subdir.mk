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
../src/http/httprequest.cpp \
../src/http/httprequestextraparams.cpp \
../src/http/httprequestqueue.cpp \
../src/http/httprequestvoiceassistantextraparams.cpp \
../src/http/oauth_credentials.cpp \
../src/http/trivialhttp.cpp \
../src/http/trivialhttpfactory.cpp \
../src/http/trivialhttps.cpp \
../src/http/voice_assistant_client2.cpp 

CPP_DEPS += \
./src/http/abstract_curl_adapter.d \
./src/http/asynctask_http_request.d \
./src/http/asynctask_http_thread_bucket.d \
./src/http/asynctask_http_thread_pool.d \
./src/http/curl_adapter.d \
./src/http/httprequest.d \
./src/http/httprequestextraparams.d \
./src/http/httprequestqueue.d \
./src/http/httprequestvoiceassistantextraparams.d \
./src/http/oauth_credentials.d \
./src/http/trivialhttp.d \
./src/http/trivialhttpfactory.d \
./src/http/trivialhttps.d \
./src/http/voice_assistant_client2.d 

OBJS += \
./src/http/abstract_curl_adapter.o \
./src/http/asynctask_http_request.o \
./src/http/asynctask_http_thread_bucket.o \
./src/http/asynctask_http_thread_pool.o \
./src/http/curl_adapter.o \
./src/http/httprequest.o \
./src/http/httprequestextraparams.o \
./src/http/httprequestqueue.o \
./src/http/httprequestvoiceassistantextraparams.o \
./src/http/oauth_credentials.o \
./src/http/trivialhttp.o \
./src/http/trivialhttpfactory.o \
./src/http/trivialhttps.o \
./src/http/voice_assistant_client2.o 


# Each subdirectory must supply rules for building sources it contributes
src/http/%.o: ../src/http/%.cpp src/http/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O2 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-http

clean-src-2f-http:
	-$(RM) ./src/http/abstract_curl_adapter.d ./src/http/abstract_curl_adapter.o ./src/http/asynctask_http_request.d ./src/http/asynctask_http_request.o ./src/http/asynctask_http_thread_bucket.d ./src/http/asynctask_http_thread_bucket.o ./src/http/asynctask_http_thread_pool.d ./src/http/asynctask_http_thread_pool.o ./src/http/curl_adapter.d ./src/http/curl_adapter.o ./src/http/httprequest.d ./src/http/httprequest.o ./src/http/httprequestextraparams.d ./src/http/httprequestextraparams.o ./src/http/httprequestqueue.d ./src/http/httprequestqueue.o ./src/http/httprequestvoiceassistantextraparams.d ./src/http/httprequestvoiceassistantextraparams.o ./src/http/oauth_credentials.d ./src/http/oauth_credentials.o ./src/http/trivialhttp.d ./src/http/trivialhttp.o ./src/http/trivialhttpfactory.d ./src/http/trivialhttpfactory.o ./src/http/trivialhttps.d ./src/http/trivialhttps.o ./src/http/voice_assistant_client2.d ./src/http/voice_assistant_client2.o

.PHONY: clean-src-2f-http

