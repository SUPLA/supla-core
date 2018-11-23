################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/http/httprequest.cpp \
../src/http/httprequestqueue.cpp \
../src/http/trivialhttp.cpp \
../src/http/trivialhttps.cpp 

OBJS += \
./src/http/httprequest.o \
./src/http/httprequestqueue.o \
./src/http/trivialhttp.o \
./src/http/trivialhttps.o 

CPP_DEPS += \
./src/http/httprequest.d \
./src/http/httprequestqueue.d \
./src/http/trivialhttp.d \
./src/http/trivialhttps.d 


# Each subdirectory must supply rules for building sources it contributes
src/http/%.o: ../src/http/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -D__OPENSSL_TOOLS=1 -I/usr/include/mysql -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O3 -Wall -c -fmessage-length=0 -fstack-protector-all -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


