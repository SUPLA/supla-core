################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/alexa/alexaclient.cpp \
../src/alexa/alexatoken.cpp 

OBJS += \
./src/alexa/alexaclient.o \
./src/alexa/alexatoken.o 

CPP_DEPS += \
./src/alexa/alexaclient.d \
./src/alexa/alexatoken.d 


# Each subdirectory must supply rules for building sources it contributes
src/alexa/%.o: ../src/alexa/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -D__OPENSSL_TOOLS=1 -I/usr/include/mysql -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O3 -Wall -c -fmessage-length=0 -fstack-protector-all -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


