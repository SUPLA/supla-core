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
	$(CXX) -D__DEBUG=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-converter

clean-src-2f-converter:
	-$(RM) ./src/converter/abstract_value_to_action_converter.d ./src/converter/abstract_value_to_action_converter.o ./src/converter/any_value_to_action_converter.d ./src/converter/any_value_to_action_converter.o ./src/converter/gate_value_to_action_converter.d ./src/converter/gate_value_to_action_converter.o ./src/converter/onoff_value_to_action_converter.d ./src/converter/onoff_value_to_action_converter.o ./src/converter/rgbw_value_to_action_converter.d ./src/converter/rgbw_value_to_action_converter.o ./src/converter/rs_value_to_action_converter.d ./src/converter/rs_value_to_action_converter.o

.PHONY: clean-src-2f-converter

