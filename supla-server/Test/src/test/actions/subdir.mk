################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/actions/ActionExecutorTest.cpp \
../src/test/actions/ActionTriggerCopyTest.cpp \
../src/test/actions/ActionTriggerTest.cpp 

CPP_DEPS += \
./src/test/actions/ActionExecutorTest.d \
./src/test/actions/ActionTriggerCopyTest.d \
./src/test/actions/ActionTriggerTest.d 

OBJS += \
./src/test/actions/ActionExecutorTest.o \
./src/test/actions/ActionTriggerCopyTest.o \
./src/test/actions/ActionTriggerTest.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/actions/%.o: ../src/test/actions/%.cpp src/test/actions/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-actions

clean-src-2f-test-2f-actions:
	-$(RM) ./src/test/actions/ActionExecutorTest.d ./src/test/actions/ActionExecutorTest.o ./src/test/actions/ActionTriggerCopyTest.d ./src/test/actions/ActionTriggerCopyTest.o ./src/test/actions/ActionTriggerTest.d ./src/test/actions/ActionTriggerTest.o

.PHONY: clean-src-2f-test-2f-actions

