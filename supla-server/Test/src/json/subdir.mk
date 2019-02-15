################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/json/cJSON.c \
../src/json/cJSON_Utils.c 

OBJS += \
./src/json/cJSON.o \
./src/json/cJSON_Utils.o 

C_DEPS += \
./src/json/cJSON.d \
./src/json/cJSON_Utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/json/%.o: ../src/json/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -D__DEBUG=1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I$(SSLDIR)/include -I../src/client -I../src/user -I../src/device -I../src -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all  -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


