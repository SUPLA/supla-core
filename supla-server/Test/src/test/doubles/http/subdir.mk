################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/doubles/http/AccessTokenProviderMock.cpp \
../src/test/doubles/http/CurlAdapterMock.cpp \
../src/test/doubles/http/HttphrottlingMock.cpp 

CPP_DEPS += \
./src/test/doubles/http/AccessTokenProviderMock.d \
./src/test/doubles/http/CurlAdapterMock.d \
./src/test/doubles/http/HttphrottlingMock.d 

OBJS += \
./src/test/doubles/http/AccessTokenProviderMock.o \
./src/test/doubles/http/CurlAdapterMock.o \
./src/test/doubles/http/HttphrottlingMock.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/doubles/http/%.o: ../src/test/doubles/http/%.cpp src/test/doubles/http/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-doubles-2f-http

clean-src-2f-test-2f-doubles-2f-http:
	-$(RM) ./src/test/doubles/http/AccessTokenProviderMock.d ./src/test/doubles/http/AccessTokenProviderMock.o ./src/test/doubles/http/CurlAdapterMock.d ./src/test/doubles/http/CurlAdapterMock.o ./src/test/doubles/http/HttphrottlingMock.d ./src/test/doubles/http/HttphrottlingMock.o

.PHONY: clean-src-2f-test-2f-doubles-2f-http

