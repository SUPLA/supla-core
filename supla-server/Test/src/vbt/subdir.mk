################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/vbt/value_based_trigger.cpp \
../src/vbt/value_based_trigger_dao.cpp \
../src/vbt/value_based_triggers.cpp \
../src/vbt/vbt_condition_result.cpp \
../src/vbt/vbt_on_change_condition.cpp \
../src/vbt/vbt_value.cpp 

CPP_DEPS += \
./src/vbt/value_based_trigger.d \
./src/vbt/value_based_trigger_dao.d \
./src/vbt/value_based_triggers.d \
./src/vbt/vbt_condition_result.d \
./src/vbt/vbt_on_change_condition.d \
./src/vbt/vbt_value.d 

OBJS += \
./src/vbt/value_based_trigger.o \
./src/vbt/value_based_trigger_dao.o \
./src/vbt/value_based_triggers.o \
./src/vbt/vbt_condition_result.o \
./src/vbt/vbt_on_change_condition.o \
./src/vbt/vbt_value.o 


# Each subdirectory must supply rules for building sources it contributes
src/vbt/%.o: ../src/vbt/%.cpp src/vbt/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-vbt

clean-src-2f-vbt:
	-$(RM) ./src/vbt/value_based_trigger.d ./src/vbt/value_based_trigger.o ./src/vbt/value_based_trigger_dao.d ./src/vbt/value_based_trigger_dao.o ./src/vbt/value_based_triggers.d ./src/vbt/value_based_triggers.o ./src/vbt/vbt_condition_result.d ./src/vbt/vbt_condition_result.o ./src/vbt/vbt_on_change_condition.d ./src/vbt/vbt_on_change_condition.o ./src/vbt/vbt_value.d ./src/vbt/vbt_value.o

.PHONY: clean-src-2f-vbt

