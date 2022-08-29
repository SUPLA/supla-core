################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/conn/AuthkeyCacheTest.cpp \
../src/test/conn/ConnectionObjectTest.cpp \
../src/test/conn/ConnectionObjectsTest.cpp 

CPP_DEPS += \
./src/test/conn/AuthkeyCacheTest.d \
./src/test/conn/ConnectionObjectTest.d \
./src/test/conn/ConnectionObjectsTest.d 

OBJS += \
./src/test/conn/AuthkeyCacheTest.o \
./src/test/conn/ConnectionObjectTest.o \
./src/test/conn/ConnectionObjectsTest.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/conn/%.o: ../src/test/conn/%.cpp src/test/conn/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-conn

clean-src-2f-test-2f-conn:
	-$(RM) ./src/test/conn/AuthkeyCacheTest.d ./src/test/conn/AuthkeyCacheTest.o ./src/test/conn/ConnectionObjectTest.d ./src/test/conn/ConnectionObjectTest.o ./src/test/conn/ConnectionObjectsTest.d ./src/test/conn/ConnectionObjectsTest.o

.PHONY: clean-src-2f-test-2f-conn

