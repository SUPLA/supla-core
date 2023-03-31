################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/amazon/alexa_change_report_request.cpp \
../src/amazon/alexa_change_report_search_condition.cpp \
../src/amazon/alexa_client2.cpp \
../src/amazon/alexa_credentials2.cpp \
../src/amazon/alexa_credentials_dao.cpp \
../src/amazon/alexa_request2.cpp \
../src/amazon/alexa_response_request.cpp 

CPP_DEPS += \
./src/amazon/alexa_change_report_request.d \
./src/amazon/alexa_change_report_search_condition.d \
./src/amazon/alexa_client2.d \
./src/amazon/alexa_credentials2.d \
./src/amazon/alexa_credentials_dao.d \
./src/amazon/alexa_request2.d \
./src/amazon/alexa_response_request.d 

OBJS += \
./src/amazon/alexa_change_report_request.o \
./src/amazon/alexa_change_report_search_condition.o \
./src/amazon/alexa_client2.o \
./src/amazon/alexa_credentials2.o \
./src/amazon/alexa_credentials_dao.o \
./src/amazon/alexa_request2.o \
./src/amazon/alexa_response_request.o 


# Each subdirectory must supply rules for building sources it contributes
src/amazon/%.o: ../src/amazon/%.cpp src/amazon/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-amazon

clean-src-2f-amazon:
	-$(RM) ./src/amazon/alexa_change_report_request.d ./src/amazon/alexa_change_report_request.o ./src/amazon/alexa_change_report_search_condition.d ./src/amazon/alexa_change_report_search_condition.o ./src/amazon/alexa_client2.d ./src/amazon/alexa_client2.o ./src/amazon/alexa_credentials2.d ./src/amazon/alexa_credentials2.o ./src/amazon/alexa_credentials_dao.d ./src/amazon/alexa_credentials_dao.o ./src/amazon/alexa_request2.d ./src/amazon/alexa_request2.o ./src/amazon/alexa_response_request.d ./src/amazon/alexa_response_request.o

.PHONY: clean-src-2f-amazon

