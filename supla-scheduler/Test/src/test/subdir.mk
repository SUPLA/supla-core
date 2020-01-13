################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/ActionTest.cpp \
../src/test/AllTests.cpp \
../src/test/WorkerTest.cpp 

OBJS += \
./src/test/ActionTest.o \
./src/test/AllTests.o \
./src/test/WorkerTest.o 

CPP_DEPS += \
./src/test/ActionTest.d \
./src/test/AllTests.d \
./src/test/WorkerTest.d 


# Each subdirectory must supply rules for building sources it contributes
src/test/%.o: ../src/test/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -I$(INCMYSQL) -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


