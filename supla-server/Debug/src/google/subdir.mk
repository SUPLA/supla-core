################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/google/googlehome.cpp \
../src/google/googlehomeclient.cpp \
../src/google/googlehomerequest.cpp 

OBJS += \
./src/google/googlehome.o \
./src/google/googlehomeclient.o \
./src/google/googlehomerequest.o 

CPP_DEPS += \
./src/google/googlehome.d \
./src/google/googlehomeclient.d \
./src/google/googlehomerequest.d 


# Each subdirectory must supply rules for building sources it contributes
src/google/%.o: ../src/google/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I/usr/include/mysql -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


