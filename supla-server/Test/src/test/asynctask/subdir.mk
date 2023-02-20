################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/asynctask/ActionGateOpenClose.cpp \
../src/test/asynctask/AsyncTaskBasicTest.cpp \
../src/test/asynctask/AsyncTaskMainTest.cpp \
../src/test/asynctask/AsyncTaskSearchTest.cpp \
../src/test/asynctask/AsyncTaskStateTest.cpp \
../src/test/asynctask/AsyncTaskTest.cpp \
../src/test/asynctask/AsyncTaskThreadPoolTest.cpp \
../src/test/asynctask/ChannelSearchCondition.cpp 

CPP_DEPS += \
./src/test/asynctask/ActionGateOpenClose.d \
./src/test/asynctask/AsyncTaskBasicTest.d \
./src/test/asynctask/AsyncTaskMainTest.d \
./src/test/asynctask/AsyncTaskSearchTest.d \
./src/test/asynctask/AsyncTaskStateTest.d \
./src/test/asynctask/AsyncTaskTest.d \
./src/test/asynctask/AsyncTaskThreadPoolTest.d \
./src/test/asynctask/ChannelSearchCondition.d 

OBJS += \
./src/test/asynctask/ActionGateOpenClose.o \
./src/test/asynctask/AsyncTaskBasicTest.o \
./src/test/asynctask/AsyncTaskMainTest.o \
./src/test/asynctask/AsyncTaskSearchTest.o \
./src/test/asynctask/AsyncTaskStateTest.o \
./src/test/asynctask/AsyncTaskTest.o \
./src/test/asynctask/AsyncTaskThreadPoolTest.o \
./src/test/asynctask/ChannelSearchCondition.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/asynctask/%.o: ../src/test/asynctask/%.cpp src/test/asynctask/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-asynctask

clean-src-2f-test-2f-asynctask:
	-$(RM) ./src/test/asynctask/ActionGateOpenClose.d ./src/test/asynctask/ActionGateOpenClose.o ./src/test/asynctask/AsyncTaskBasicTest.d ./src/test/asynctask/AsyncTaskBasicTest.o ./src/test/asynctask/AsyncTaskMainTest.d ./src/test/asynctask/AsyncTaskMainTest.o ./src/test/asynctask/AsyncTaskSearchTest.d ./src/test/asynctask/AsyncTaskSearchTest.o ./src/test/asynctask/AsyncTaskStateTest.d ./src/test/asynctask/AsyncTaskStateTest.o ./src/test/asynctask/AsyncTaskTest.d ./src/test/asynctask/AsyncTaskTest.o ./src/test/asynctask/AsyncTaskThreadPoolTest.d ./src/test/asynctask/AsyncTaskThreadPoolTest.o ./src/test/asynctask/ChannelSearchCondition.d ./src/test/asynctask/ChannelSearchCondition.o

.PHONY: clean-src-2f-test-2f-asynctask

