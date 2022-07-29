################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/doubles/WorkerMock.cpp 

CPP_DEPS += \
./src/test/doubles/WorkerMock.d 

OBJS += \
./src/test/doubles/WorkerMock.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/doubles/%.o: ../src/test/doubles/%.cpp src/test/doubles/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -I$(INCMYSQL) -I../src -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-doubles

clean-src-2f-test-2f-doubles:
	-$(RM) ./src/test/doubles/WorkerMock.d ./src/test/doubles/WorkerMock.o

.PHONY: clean-src-2f-test-2f-doubles

