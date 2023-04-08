################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/google/google_home_client.cpp \
../src/google/google_home_credentials.cpp \
../src/google/google_home_credentials_dao.cpp \
../src/google/google_home_state_report_request.cpp \
../src/google/google_home_state_report_search_condition.cpp \
../src/google/google_home_state_report_throttling.cpp \
../src/google/google_home_sync_request.cpp \
../src/google/google_home_sync_search_condition.cpp 

CPP_DEPS += \
./src/google/google_home_client.d \
./src/google/google_home_credentials.d \
./src/google/google_home_credentials_dao.d \
./src/google/google_home_state_report_request.d \
./src/google/google_home_state_report_search_condition.d \
./src/google/google_home_state_report_throttling.d \
./src/google/google_home_sync_request.d \
./src/google/google_home_sync_search_condition.d 

OBJS += \
./src/google/google_home_client.o \
./src/google/google_home_credentials.o \
./src/google/google_home_credentials_dao.o \
./src/google/google_home_state_report_request.o \
./src/google/google_home_state_report_search_condition.o \
./src/google/google_home_state_report_throttling.o \
./src/google/google_home_sync_request.o \
./src/google/google_home_sync_search_condition.o 


# Each subdirectory must supply rules for building sources it contributes
src/google/%.o: ../src/google/%.cpp src/google/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-google

clean-src-2f-google:
	-$(RM) ./src/google/google_home_client.d ./src/google/google_home_client.o ./src/google/google_home_credentials.d ./src/google/google_home_credentials.o ./src/google/google_home_credentials_dao.d ./src/google/google_home_credentials_dao.o ./src/google/google_home_state_report_request.d ./src/google/google_home_state_report_request.o ./src/google/google_home_state_report_search_condition.d ./src/google/google_home_state_report_search_condition.o ./src/google/google_home_state_report_throttling.d ./src/google/google_home_state_report_throttling.o ./src/google/google_home_sync_request.d ./src/google/google_home_sync_request.o ./src/google/google_home_sync_search_condition.d ./src/google/google_home_sync_search_condition.o

.PHONY: clean-src-2f-google

