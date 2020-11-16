################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/google/googlehomeclient.cpp \
../src/google/googlehomecredentials.cpp \
../src/google/googlehomerequest.cpp \
../src/google/googlehomestatereportrequest.cpp \
../src/google/googlehomesyncrequest.cpp 

OBJS += \
./src/google/googlehomeclient.o \
./src/google/googlehomecredentials.o \
./src/google/googlehomerequest.o \
./src/google/googlehomestatereportrequest.o \
./src/google/googlehomesyncrequest.o 

CPP_DEPS += \
./src/google/googlehomeclient.d \
./src/google/googlehomecredentials.d \
./src/google/googlehomerequest.d \
./src/google/googlehomestatereportrequest.d \
./src/google/googlehomesyncrequest.d 


# Each subdirectory must supply rules for building sources it contributes
src/google/%.o: ../src/google/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -DSERVER_VERSION_23 -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


