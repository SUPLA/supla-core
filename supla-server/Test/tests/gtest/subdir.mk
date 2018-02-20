################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../tests/gtest/gtest-all.cc 

CC_DEPS += \
./tests/gtest/gtest-all.d 

OBJS += \
./tests/gtest/gtest-all.o 


# Each subdirectory must supply rules for building sources it contributes
tests/gtest/%.o: ../tests/gtest/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__OPEN_SSL=1 -D__DEBUG=1 -D__BCRYPT=1 -I/usr/include/mysql -I/usr/src/openssl/include -O2 -g3 -Wall -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


