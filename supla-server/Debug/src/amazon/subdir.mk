################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/amazon/alexa_change_report_request.cpp \
../src/amazon/alexa_change_report_search_condition.cpp \
../src/amazon/alexa_client.cpp \
../src/amazon/alexa_credentials.cpp \
../src/amazon/alexa_credentials_dao.cpp \
../src/amazon/alexa_request.cpp \
../src/amazon/alexa_response_request.cpp \
../src/amazon/alexa_response_search_condition.cpp 

CPP_DEPS += \
./src/amazon/alexa_change_report_request.d \
./src/amazon/alexa_change_report_search_condition.d \
./src/amazon/alexa_client.d \
./src/amazon/alexa_credentials.d \
./src/amazon/alexa_credentials_dao.d \
./src/amazon/alexa_request.d \
./src/amazon/alexa_response_request.d \
./src/amazon/alexa_response_search_condition.d 

OBJS += \
./src/amazon/alexa_change_report_request.o \
./src/amazon/alexa_change_report_search_condition.o \
./src/amazon/alexa_client.o \
./src/amazon/alexa_credentials.o \
./src/amazon/alexa_credentials_dao.o \
./src/amazon/alexa_request.o \
./src/amazon/alexa_response_request.o \
./src/amazon/alexa_response_search_condition.o 


# Each subdirectory must supply rules for building sources it contributes
src/amazon/%.o: ../src/amazon/%.cpp src/amazon/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-amazon

clean-src-2f-amazon:
	-$(RM) ./src/amazon/alexa_change_report_request.d ./src/amazon/alexa_change_report_request.o ./src/amazon/alexa_change_report_search_condition.d ./src/amazon/alexa_change_report_search_condition.o ./src/amazon/alexa_client.d ./src/amazon/alexa_client.o ./src/amazon/alexa_credentials.d ./src/amazon/alexa_credentials.o ./src/amazon/alexa_credentials_dao.d ./src/amazon/alexa_credentials_dao.o ./src/amazon/alexa_request.d ./src/amazon/alexa_request.o ./src/amazon/alexa_response_request.d ./src/amazon/alexa_response_request.o ./src/amazon/alexa_response_search_condition.d ./src/amazon/alexa_response_search_condition.o

.PHONY: clean-src-2f-amazon

