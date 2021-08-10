################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/asynctask/AsyncTaskMock.cpp \
../src/test/asynctask/AsyncTaskQueueMock.cpp \
../src/test/asynctask/AsyncTaskTest.cpp \
../src/test/asynctask/AsyncTaskThreadPoolMock.cpp \
../src/test/asynctask/AsyncTaskThreadPoolTest.cpp 

OBJS += \
./src/test/asynctask/AsyncTaskMock.o \
./src/test/asynctask/AsyncTaskQueueMock.o \
./src/test/asynctask/AsyncTaskTest.o \
./src/test/asynctask/AsyncTaskThreadPoolMock.o \
./src/test/asynctask/AsyncTaskThreadPoolTest.o 

CPP_DEPS += \
./src/test/asynctask/AsyncTaskMock.d \
./src/test/asynctask/AsyncTaskQueueMock.d \
./src/test/asynctask/AsyncTaskTest.d \
./src/test/asynctask/AsyncTaskThreadPoolMock.d \
./src/test/asynctask/AsyncTaskThreadPoolTest.d 


# Each subdirectory must supply rules for building sources it contributes
src/test/asynctask/%.o: ../src/test/asynctask/%.cpp src/test/asynctask/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


