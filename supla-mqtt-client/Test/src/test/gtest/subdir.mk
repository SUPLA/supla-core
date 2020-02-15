################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/test/gtest/gtest-all.cc 

CC_DEPS += \
./src/test/gtest/gtest-all.d 

OBJS += \
./src/test/gtest/gtest-all.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/gtest/%.o: ../src/test/gtest/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++11 -D__DEBUG=1 -D__NO_USER -D__NO_DATABASE -I$(SSLDIR)/include -I../src/json -I../src/test -O0 -g3 -Wall -fsigned-char  -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


