################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/vbt/abstract_vbt_condition.cpp \
../src/vbt/value_based_trigger.cpp \
../src/vbt/value_based_triggers.cpp 

CPP_DEPS += \
./src/vbt/abstract_vbt_condition.d \
./src/vbt/value_based_trigger.d \
./src/vbt/value_based_triggers.d 

OBJS += \
./src/vbt/abstract_vbt_condition.o \
./src/vbt/value_based_trigger.o \
./src/vbt/value_based_triggers.o 


# Each subdirectory must supply rules for building sources it contributes
src/vbt/%.o: ../src/vbt/%.cpp src/vbt/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-vbt

clean-src-2f-vbt:
	-$(RM) ./src/vbt/abstract_vbt_condition.d ./src/vbt/abstract_vbt_condition.o ./src/vbt/value_based_trigger.d ./src/vbt/value_based_trigger.o ./src/vbt/value_based_triggers.d ./src/vbt/value_based_triggers.o

.PHONY: clean-src-2f-vbt
