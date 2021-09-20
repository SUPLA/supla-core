################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/doubles/asynctask/AsyncTaskMock.cpp \
../src/test/doubles/asynctask/AsyncTaskQueueMock.cpp \
../src/test/doubles/asynctask/AsyncTaskThreadPoolMock.cpp 

OBJS += \
./src/test/doubles/asynctask/AsyncTaskMock.o \
./src/test/doubles/asynctask/AsyncTaskQueueMock.o \
./src/test/doubles/asynctask/AsyncTaskThreadPoolMock.o 

CPP_DEPS += \
./src/test/doubles/asynctask/AsyncTaskMock.d \
./src/test/doubles/asynctask/AsyncTaskQueueMock.d \
./src/test/doubles/asynctask/AsyncTaskThreadPoolMock.d 


# Each subdirectory must supply rules for building sources it contributes
src/test/doubles/asynctask/%.o: ../src/test/doubles/asynctask/%.cpp src/test/doubles/asynctask/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


