################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/json/json_helper.cpp 

C_SRCS += \
../src/json/cJSON.c \
../src/json/cJSON_Utils.c 

CPP_DEPS += \
./src/json/json_helper.d 

C_DEPS += \
./src/json/cJSON.d \
./src/json/cJSON_Utils.d 

OBJS += \
./src/json/cJSON.o \
./src/json/cJSON_Utils.o \
./src/json/json_helper.o 


# Each subdirectory must supply rules for building sources it contributes
src/json/%.o: ../src/json/%.c src/json/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I$(INCMYSQL) -I../src -O2 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/json/%.o: ../src/json/%.cpp src/json/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I$(INCMYSQL) -I../src -O2 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-json

clean-src-2f-json:
	-$(RM) ./src/json/cJSON.d ./src/json/cJSON.o ./src/json/cJSON_Utils.d ./src/json/cJSON_Utils.o ./src/json/json_helper.d ./src/json/json_helper.o

.PHONY: clean-src-2f-json

