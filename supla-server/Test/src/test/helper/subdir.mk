################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/helper/InjaSandboxTest.cpp 

CPP_DEPS += \
./src/test/helper/InjaSandboxTest.d 

OBJS += \
./src/test/helper/InjaSandboxTest.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/helper/%.o: ../src/test/helper/%.cpp src/test/helper/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++17 -D__DEBUG=1 -DMQTTC_PAL_FILE=../src/mqtt/mqtt_pal.h -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/external/inja/include -I../src/external/cJSON -I../src/external/MQTT-C/include -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-helper

clean-src-2f-test-2f-helper:
	-$(RM) ./src/test/helper/InjaSandboxTest.d ./src/test/helper/InjaSandboxTest.o

.PHONY: clean-src-2f-test-2f-helper

