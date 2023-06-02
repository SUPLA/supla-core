################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/srpc/MultipartCallStoreTest.cpp \
../src/test/srpc/MultipartCallTest.cpp 

CPP_DEPS += \
./src/test/srpc/MultipartCallStoreTest.d \
./src/test/srpc/MultipartCallTest.d 

OBJS += \
./src/test/srpc/MultipartCallStoreTest.o \
./src/test/srpc/MultipartCallTest.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/srpc/%.o: ../src/test/srpc/%.cpp src/test/srpc/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-srpc

clean-src-2f-test-2f-srpc:
	-$(RM) ./src/test/srpc/MultipartCallStoreTest.d ./src/test/srpc/MultipartCallStoreTest.o ./src/test/srpc/MultipartCallTest.d ./src/test/srpc/MultipartCallTest.o

.PHONY: clean-src-2f-test-2f-srpc

