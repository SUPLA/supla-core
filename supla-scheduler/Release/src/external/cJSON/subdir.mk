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
	gcc -I$(INCMYSQL) -I../src/external/cJSON -I../src -O2 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-external-2f-cJSON

clean-src-2f-external-2f-cJSON:
	-$(RM) ./src/external/cJSON/cJSON.d ./src/external/cJSON/cJSON.o ./src/external/cJSON/cJSON_Utils.d ./src/external/cJSON/cJSON_Utils.o

.PHONY: clean-src-2f-external-2f-cJSON

