################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/helper/inja_sandbox.cpp \
../src/helper/json_helper.cpp 

CPP_DEPS += \
./src/helper/inja_sandbox.d \
./src/helper/json_helper.d 

OBJS += \
./src/helper/inja_sandbox.o \
./src/helper/json_helper.o 


# Each subdirectory must supply rules for building sources it contributes
src/helper/%.o: ../src/helper/%.cpp src/helper/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -std=c++17 -D__DEBUG=1 -DMQTTC_PAL_FILE=../src/mqtt/mqtt_pal.h -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/external/inja/include -I../src/external/MQTT-C/include -I../src/external/cJSON -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-helper

clean-src-2f-helper:
	-$(RM) ./src/helper/inja_sandbox.d ./src/helper/inja_sandbox.o ./src/helper/json_helper.d ./src/helper/json_helper.o

.PHONY: clean-src-2f-helper

