################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/amazon/AlexaClientTest.cpp 

OBJS += \
./src/test/amazon/AlexaClientTest.o 

CPP_DEPS += \
./src/test/amazon/AlexaClientTest.d 


# Each subdirectory must supply rules for building sources it contributes
src/test/amazon/%.o: ../src/test/amazon/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__OPENSSL_TOOLS=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I/usr/include/mysql -I../src -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


