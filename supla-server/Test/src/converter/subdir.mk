################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/converter/abstract_value_to_action_converter.cpp \
../src/converter/any_value_to_action_converter.cpp \
../src/converter/gate_value_to_action_converter.cpp \
../src/converter/onoff_value_to_action_converter.cpp \
../src/converter/rgbw_value_to_action_converter.cpp \
../src/converter/rs_value_to_action_converter.cpp 

CPP_DEPS += \
./src/converter/abstract_value_to_action_converter.d \
./src/converter/any_value_to_action_converter.d \
./src/converter/gate_value_to_action_converter.d \
./src/converter/onoff_value_to_action_converter.d \
./src/converter/rgbw_value_to_action_converter.d \
./src/converter/rs_value_to_action_converter.d 

OBJS += \
./src/converter/abstract_value_to_action_converter.o \
./src/converter/any_value_to_action_converter.o \
./src/converter/gate_value_to_action_converter.o \
./src/converter/onoff_value_to_action_converter.o \
./src/converter/rgbw_value_to_action_converter.o \
./src/converter/rs_value_to_action_converter.o 


# Each subdirectory must supply rules for building sources it contributes
src/converter/%.o: ../src/converter/%.cpp src/converter/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-converter

clean-src-2f-converter:
	-$(RM) ./src/converter/abstract_value_to_action_converter.d ./src/converter/abstract_value_to_action_converter.o ./src/converter/any_value_to_action_converter.d ./src/converter/any_value_to_action_converter.o ./src/converter/gate_value_to_action_converter.d ./src/converter/gate_value_to_action_converter.o ./src/converter/onoff_value_to_action_converter.d ./src/converter/onoff_value_to_action_converter.o ./src/converter/rgbw_value_to_action_converter.d ./src/converter/rgbw_value_to_action_converter.o ./src/converter/rs_value_to_action_converter.d ./src/converter/rs_value_to_action_converter.o

.PHONY: clean-src-2f-converter

