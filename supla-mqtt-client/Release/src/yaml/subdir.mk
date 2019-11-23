################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/yaml/yaml.cpp 

OBJS += \
./src/yaml/yaml.o 

CPP_DEPS += \
./src/yaml/yaml.d 


# Each subdirectory must supply rules for building sources it contributes
src/yaml/%.o: ../src/yaml/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__NO_DATABASE -D__NO_USER -I$(SSLDIR)/include -O3 -Wall -fsigned-char  -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


