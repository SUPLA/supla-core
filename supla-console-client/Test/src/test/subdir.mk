################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/AllTests.cpp 

CPP_DEPS += \
./src/test/AllTests.d 

OBJS += \
./src/test/AllTests.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/%.o: ../src/test/%.cpp src/test/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__TEST=1 -DNOMYSQL=1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -I$(SSLDIR)/include -I../src/supla-client-lib -I../src/test -O0 -g3 -Wall -fsigned-char  -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test

clean-src-2f-test:
	-$(RM) ./src/test/AllTests.d ./src/test/AllTests.o

.PHONY: clean-src-2f-test

