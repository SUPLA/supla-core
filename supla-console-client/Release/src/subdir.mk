################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/supla-console-client.cpp 

C_SRCS += \
../src/client_loop.c \
../src/clientcfg.c 

CPP_DEPS += \
./src/supla-console-client.d 

C_DEPS += \
./src/client_loop.d \
./src/clientcfg.d 

OBJS += \
./src/client_loop.o \
./src/clientcfg.o \
./src/supla-console-client.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -D__LOG_CALLBACK=1 -DNOMYSQL=1 -I$(SSLDIR)/include -O3 -Wall -fsigned-char  -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.cpp src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -DNOMYSQL=1 -I$(SSLDIR)/include -O3 -Wall -fsigned-char  -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/client_loop.d ./src/client_loop.o ./src/clientcfg.d ./src/clientcfg.o ./src/supla-console-client.d ./src/supla-console-client.o

.PHONY: clean-src

