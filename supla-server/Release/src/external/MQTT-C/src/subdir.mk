################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/external/MQTT-C/src/mqtt.c 

C_DEPS += \
./src/external/MQTT-C/src/mqtt.d 

OBJS += \
./src/external/MQTT-C/src/mqtt.o 


# Each subdirectory must supply rules for building sources it contributes
src/external/MQTT-C/src/%.o: ../src/external/MQTT-C/src/%.c src/external/MQTT-C/src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -std=c17 -D__BCRYPT=1 -DMQTTC_PAL_FILE=../src/mqtt/mqtt_pal.h -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__OPENSSL_TOOLS=1 -I$(SSLDIR)/include -I../src/external/inja/include -I../src/external/MQTT-C/include -I../src/external/cJSON -I../src/mqtt -I../src -I../src/user -I../src/device -I../src/client -O2 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-external-2f-MQTT-2d-C-2f-src

clean-src-2f-external-2f-MQTT-2d-C-2f-src:
	-$(RM) ./src/external/MQTT-C/src/mqtt.d ./src/external/MQTT-C/src/mqtt.o

.PHONY: clean-src-2f-external-2f-MQTT-2d-C-2f-src

