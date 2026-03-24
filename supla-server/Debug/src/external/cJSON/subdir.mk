################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/external/cJSON/cJSON.c \
../src/external/cJSON/cJSON_Utils.c 

C_DEPS += \
./src/external/cJSON/cJSON.d \
./src/external/cJSON/cJSON_Utils.d 

OBJS += \
./src/external/cJSON/cJSON.o \
./src/external/cJSON/cJSON_Utils.o 


# Each subdirectory must supply rules for building sources it contributes
src/external/cJSON/%.o: ../src/external/cJSON/%.c src/external/cJSON/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	$(CC) -std=c17 -D__OPENSSL_TOOLS=1 -DMQTTC_PAL_FILE=../src/mqtt/mqtt_pal.h -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__DEBUG=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(SSLDIR)/include -I../src/external/inja/include -I../src/external/MQTT-C/include -I../src/external/cJSON -I../src/mqtt -I../src/user -I../src/device -I../src/client -I../src -I/usr/include/postgresql -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all  -D_FORTIFY_SOURCE=2 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-external-2f-cJSON

clean-src-2f-external-2f-cJSON:
	-$(RM) ./src/external/cJSON/cJSON.d ./src/external/cJSON/cJSON.o ./src/external/cJSON/cJSON_Utils.d ./src/external/cJSON/cJSON_Utils.o

.PHONY: clean-src-2f-external-2f-cJSON

