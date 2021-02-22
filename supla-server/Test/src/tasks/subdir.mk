################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/tasks/abstract_execution_condition.cpp \
../src/tasks/abstract_scheduled_task.cpp \
../src/tasks/abstract_scheduled_task_action.cpp 

OBJS += \
./src/tasks/abstract_execution_condition.o \
./src/tasks/abstract_scheduled_task.o \
./src/tasks/abstract_scheduled_task_action.o 

CPP_DEPS += \
./src/tasks/abstract_execution_condition.d \
./src/tasks/abstract_scheduled_task.d \
./src/tasks/abstract_scheduled_task_action.d 


# Each subdirectory must supply rules for building sources it contributes
src/tasks/%.o: ../src/tasks/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DSERVER_VERSION_23 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


