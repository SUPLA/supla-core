################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/ActionParamTest.cpp \
../src/test/ActionTest.cpp \
../src/test/AllTests.cpp \
../src/test/WorkerTest.cpp 

CPP_DEPS += \
./src/test/ActionParamTest.d \
./src/test/ActionTest.d \
./src/test/AllTests.d \
./src/test/WorkerTest.d 

OBJS += \
./src/test/ActionParamTest.o \
./src/test/ActionTest.o \
./src/test/AllTests.o \
./src/test/WorkerTest.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/%.o: ../src/test/%.cpp src/test/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -I$(INCMYSQL) -I../src -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test

clean-src-2f-test:
	-$(RM) ./src/test/ActionParamTest.d ./src/test/ActionParamTest.o ./src/test/ActionTest.d ./src/test/ActionTest.o ./src/test/AllTests.d ./src/test/AllTests.o ./src/test/WorkerTest.d ./src/test/WorkerTest.o

.PHONY: clean-src-2f-test

