################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/test/integration/datalogger/ElectricityLoggerDaoIntegrationTest.cpp \
../src/test/integration/datalogger/ImpulseLoggerDaoIntegrationTest.cpp \
../src/test/integration/datalogger/TemperatureLoggerDaoIntegrationTest.cpp \
../src/test/integration/datalogger/ThermostatLoggerDaoIntegrationTest.cpp \
../src/test/integration/datalogger/VoltageThreadsholdLoggerDaoIntegrationTest.cpp 

CPP_DEPS += \
./src/test/integration/datalogger/ElectricityLoggerDaoIntegrationTest.d \
./src/test/integration/datalogger/ImpulseLoggerDaoIntegrationTest.d \
./src/test/integration/datalogger/TemperatureLoggerDaoIntegrationTest.d \
./src/test/integration/datalogger/ThermostatLoggerDaoIntegrationTest.d \
./src/test/integration/datalogger/VoltageThreadsholdLoggerDaoIntegrationTest.d 

OBJS += \
./src/test/integration/datalogger/ElectricityLoggerDaoIntegrationTest.o \
./src/test/integration/datalogger/ImpulseLoggerDaoIntegrationTest.o \
./src/test/integration/datalogger/TemperatureLoggerDaoIntegrationTest.o \
./src/test/integration/datalogger/ThermostatLoggerDaoIntegrationTest.o \
./src/test/integration/datalogger/VoltageThreadsholdLoggerDaoIntegrationTest.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/integration/datalogger/%.o: ../src/test/integration/datalogger/%.cpp src/test/integration/datalogger/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-test-2f-integration-2f-datalogger

clean-src-2f-test-2f-integration-2f-datalogger:
	-$(RM) ./src/test/integration/datalogger/ElectricityLoggerDaoIntegrationTest.d ./src/test/integration/datalogger/ElectricityLoggerDaoIntegrationTest.o ./src/test/integration/datalogger/ImpulseLoggerDaoIntegrationTest.d ./src/test/integration/datalogger/ImpulseLoggerDaoIntegrationTest.o ./src/test/integration/datalogger/TemperatureLoggerDaoIntegrationTest.d ./src/test/integration/datalogger/TemperatureLoggerDaoIntegrationTest.o ./src/test/integration/datalogger/ThermostatLoggerDaoIntegrationTest.d ./src/test/integration/datalogger/ThermostatLoggerDaoIntegrationTest.o ./src/test/integration/datalogger/VoltageThreadsholdLoggerDaoIntegrationTest.d ./src/test/integration/datalogger/VoltageThreadsholdLoggerDaoIntegrationTest.o

.PHONY: clean-src-2f-test-2f-integration-2f-datalogger

