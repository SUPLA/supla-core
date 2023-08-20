################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/datalogger/electricity_logger.cpp \
../src/datalogger/electricity_logger_dao.cpp \
../src/datalogger/impulse_logger.cpp \
../src/datalogger/impulse_logger_dao.cpp \
../src/datalogger/temperature_logger.cpp \
../src/datalogger/temperature_logger_dao.cpp \
../src/datalogger/thermostat_logger.cpp \
../src/datalogger/thermostat_logger_dao.cpp \
../src/datalogger/voltage_threshold_logger.cpp \
../src/datalogger/voltage_threshold_logger_dao.cpp 

CPP_DEPS += \
./src/datalogger/electricity_logger.d \
./src/datalogger/electricity_logger_dao.d \
./src/datalogger/impulse_logger.d \
./src/datalogger/impulse_logger_dao.d \
./src/datalogger/temperature_logger.d \
./src/datalogger/temperature_logger_dao.d \
./src/datalogger/thermostat_logger.d \
./src/datalogger/thermostat_logger_dao.d \
./src/datalogger/voltage_threshold_logger.d \
./src/datalogger/voltage_threshold_logger_dao.d 

OBJS += \
./src/datalogger/electricity_logger.o \
./src/datalogger/electricity_logger_dao.o \
./src/datalogger/impulse_logger.o \
./src/datalogger/impulse_logger_dao.o \
./src/datalogger/temperature_logger.o \
./src/datalogger/temperature_logger_dao.o \
./src/datalogger/thermostat_logger.o \
./src/datalogger/thermostat_logger_dao.o \
./src/datalogger/voltage_threshold_logger.o \
./src/datalogger/voltage_threshold_logger_dao.o 


# Each subdirectory must supply rules for building sources it contributes
src/datalogger/%.o: ../src/datalogger/%.cpp src/datalogger/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-datalogger

clean-src-2f-datalogger:
	-$(RM) ./src/datalogger/electricity_logger.d ./src/datalogger/electricity_logger.o ./src/datalogger/electricity_logger_dao.d ./src/datalogger/electricity_logger_dao.o ./src/datalogger/impulse_logger.d ./src/datalogger/impulse_logger.o ./src/datalogger/impulse_logger_dao.d ./src/datalogger/impulse_logger_dao.o ./src/datalogger/temperature_logger.d ./src/datalogger/temperature_logger.o ./src/datalogger/temperature_logger_dao.d ./src/datalogger/temperature_logger_dao.o ./src/datalogger/thermostat_logger.d ./src/datalogger/thermostat_logger.o ./src/datalogger/thermostat_logger_dao.d ./src/datalogger/thermostat_logger_dao.o ./src/datalogger/voltage_threshold_logger.d ./src/datalogger/voltage_threshold_logger.o ./src/datalogger/voltage_threshold_logger_dao.d ./src/datalogger/voltage_threshold_logger_dao.o

.PHONY: clean-src-2f-datalogger

