################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/schedule/schedule_dao.cpp 

CPP_DEPS += \
./src/schedule/schedule_dao.d 

OBJS += \
./src/schedule/schedule_dao.o 


# Each subdirectory must supply rules for building sources it contributes
src/schedule/%.o: ../src/schedule/%.cpp src/schedule/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-schedule

clean-src-2f-schedule:
	-$(RM) ./src/schedule/schedule_dao.d ./src/schedule/schedule_dao.o

.PHONY: clean-src-2f-schedule

