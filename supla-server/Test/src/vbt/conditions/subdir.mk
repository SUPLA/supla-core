################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/vbt/conditions/abstract_vbt_bool_condition.cpp \
../src/vbt/conditions/abstract_vbt_condition.cpp \
../src/vbt/conditions/vbt_binary_condition.cpp 

CPP_DEPS += \
./src/vbt/conditions/abstract_vbt_bool_condition.d \
./src/vbt/conditions/abstract_vbt_condition.d \
./src/vbt/conditions/vbt_binary_condition.d 

OBJS += \
./src/vbt/conditions/abstract_vbt_bool_condition.o \
./src/vbt/conditions/abstract_vbt_condition.o \
./src/vbt/conditions/vbt_binary_condition.o 


# Each subdirectory must supply rules for building sources it contributes
src/vbt/conditions/%.o: ../src/vbt/conditions/%.cpp src/vbt/conditions/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-vbt-2f-conditions

clean-src-2f-vbt-2f-conditions:
	-$(RM) ./src/vbt/conditions/abstract_vbt_bool_condition.d ./src/vbt/conditions/abstract_vbt_bool_condition.o ./src/vbt/conditions/abstract_vbt_condition.d ./src/vbt/conditions/abstract_vbt_condition.o ./src/vbt/conditions/vbt_binary_condition.d ./src/vbt/conditions/vbt_binary_condition.o

.PHONY: clean-src-2f-vbt-2f-conditions

