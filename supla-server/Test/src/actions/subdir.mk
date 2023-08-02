################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/actions/abstract_action_config.cpp \
../src/actions/abstract_action_executor.cpp \
../src/actions/abstract_action_parameters.cpp \
../src/actions/action_config.cpp \
../src/actions/action_executor.cpp \
../src/actions/action_gate_openclose.cpp \
../src/actions/action_gate_openclose_search_condition.cpp \
../src/actions/action_hvac_parameters.cpp \
../src/actions/action_rgbw_parameters.cpp \
../src/actions/action_rs_parameters.cpp \
../src/actions/action_trigger.cpp 

CPP_DEPS += \
./src/actions/abstract_action_config.d \
./src/actions/abstract_action_executor.d \
./src/actions/abstract_action_parameters.d \
./src/actions/action_config.d \
./src/actions/action_executor.d \
./src/actions/action_gate_openclose.d \
./src/actions/action_gate_openclose_search_condition.d \
./src/actions/action_hvac_parameters.d \
./src/actions/action_rgbw_parameters.d \
./src/actions/action_rs_parameters.d \
./src/actions/action_trigger.d 

OBJS += \
./src/actions/abstract_action_config.o \
./src/actions/abstract_action_executor.o \
./src/actions/abstract_action_parameters.o \
./src/actions/action_config.o \
./src/actions/action_executor.o \
./src/actions/action_gate_openclose.o \
./src/actions/action_gate_openclose_search_condition.o \
./src/actions/action_hvac_parameters.o \
./src/actions/action_rgbw_parameters.o \
./src/actions/action_rs_parameters.o \
./src/actions/action_trigger.o 


# Each subdirectory must supply rules for building sources it contributes
src/actions/%.o: ../src/actions/%.cpp src/actions/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-actions

clean-src-2f-actions:
	-$(RM) ./src/actions/abstract_action_config.d ./src/actions/abstract_action_config.o ./src/actions/abstract_action_executor.d ./src/actions/abstract_action_executor.o ./src/actions/abstract_action_parameters.d ./src/actions/abstract_action_parameters.o ./src/actions/action_config.d ./src/actions/action_config.o ./src/actions/action_executor.d ./src/actions/action_executor.o ./src/actions/action_gate_openclose.d ./src/actions/action_gate_openclose.o ./src/actions/action_gate_openclose_search_condition.d ./src/actions/action_gate_openclose_search_condition.o ./src/actions/action_hvac_parameters.d ./src/actions/action_hvac_parameters.o ./src/actions/action_rgbw_parameters.d ./src/actions/action_rgbw_parameters.o ./src/actions/action_rs_parameters.d ./src/actions/action_rs_parameters.o ./src/actions/action_trigger.d ./src/actions/action_trigger.o

.PHONY: clean-src-2f-actions

