################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/distributedobjects/dobject.cpp \
../src/distributedobjects/dobject_client_oriented.cpp \
../src/distributedobjects/dobject_related_identifier.cpp \
../src/distributedobjects/dobject_remote_update_indicator.cpp \
../src/distributedobjects/dobject_repository.cpp \
../src/distributedobjects/dobjects.cpp 

CPP_DEPS += \
./src/distributedobjects/dobject.d \
./src/distributedobjects/dobject_client_oriented.d \
./src/distributedobjects/dobject_related_identifier.d \
./src/distributedobjects/dobject_remote_update_indicator.d \
./src/distributedobjects/dobject_repository.d \
./src/distributedobjects/dobjects.d 

OBJS += \
./src/distributedobjects/dobject.o \
./src/distributedobjects/dobject_client_oriented.o \
./src/distributedobjects/dobject_related_identifier.o \
./src/distributedobjects/dobject_remote_update_indicator.o \
./src/distributedobjects/dobject_repository.o \
./src/distributedobjects/dobjects.o 


# Each subdirectory must supply rules for building sources it contributes
src/distributedobjects/%.o: ../src/distributedobjects/%.cpp src/distributedobjects/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -DUSE_DEPRECATED_EMEV_V1 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-distributedobjects

clean-src-2f-distributedobjects:
	-$(RM) ./src/distributedobjects/dobject.d ./src/distributedobjects/dobject.o ./src/distributedobjects/dobject_client_oriented.d ./src/distributedobjects/dobject_client_oriented.o ./src/distributedobjects/dobject_related_identifier.d ./src/distributedobjects/dobject_related_identifier.o ./src/distributedobjects/dobject_remote_update_indicator.d ./src/distributedobjects/dobject_remote_update_indicator.o ./src/distributedobjects/dobject_repository.d ./src/distributedobjects/dobject_repository.o ./src/distributedobjects/dobjects.d ./src/distributedobjects/dobjects.o

.PHONY: clean-src-2f-distributedobjects

