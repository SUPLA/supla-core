################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/vbt/conditions/abstract_vbt_condition.cpp 

CPP_DEPS += \
./src/vbt/conditions/abstract_vbt_condition.d 

OBJS += \
./src/vbt/conditions/abstract_vbt_condition.o 


# Each subdirectory must supply rules for building sources it contributes
src/vbt/conditions/%.o: ../src/vbt/conditions/%.cpp src/vbt/conditions/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -D__SUPLA_SERVER=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O2 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-vbt-2f-conditions

clean-src-2f-vbt-2f-conditions:
	-$(RM) ./src/vbt/conditions/abstract_vbt_condition.d ./src/vbt/conditions/abstract_vbt_condition.o

.PHONY: clean-src-2f-vbt-2f-conditions

