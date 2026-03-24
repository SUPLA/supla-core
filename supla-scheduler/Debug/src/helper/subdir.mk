################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/helper/json_helper.cpp 

CPP_DEPS += \
./src/helper/json_helper.d 

OBJS += \
./src/helper/json_helper.o 


# Each subdirectory must supply rules for building sources it contributes
src/helper/%.o: ../src/helper/%.cpp src/helper/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -I$(INCMYSQL) -I../src/external/cJSON -I../src -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-helper

clean-src-2f-helper:
	-$(RM) ./src/helper/json_helper.d ./src/helper/json_helper.o

.PHONY: clean-src-2f-helper

