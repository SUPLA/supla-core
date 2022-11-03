################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/http/httprequest.cpp \
../src/http/httprequestactiontriggerextraparams.cpp \
../src/http/httprequestextraparams.cpp \
../src/http/httprequestqueue.cpp \
../src/http/httprequestvoiceassistantextraparams.cpp \
../src/http/trivialhttp.cpp \
../src/http/trivialhttpfactory.cpp \
../src/http/trivialhttps.cpp 

CPP_DEPS += \
./src/http/httprequest.d \
./src/http/httprequestactiontriggerextraparams.d \
./src/http/httprequestextraparams.d \
./src/http/httprequestqueue.d \
./src/http/httprequestvoiceassistantextraparams.d \
./src/http/trivialhttp.d \
./src/http/trivialhttpfactory.d \
./src/http/trivialhttps.d 

OBJS += \
./src/http/httprequest.o \
./src/http/httprequestactiontriggerextraparams.o \
./src/http/httprequestextraparams.o \
./src/http/httprequestqueue.o \
./src/http/httprequestvoiceassistantextraparams.o \
./src/http/trivialhttp.o \
./src/http/trivialhttpfactory.o \
./src/http/trivialhttps.o 


# Each subdirectory must supply rules for building sources it contributes
src/http/%.o: ../src/http/%.cpp src/http/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__BCRYPT=1 -DSPROTO_WITHOUT_OUT_BUFFER -DSRPC_WITHOUT_OUT_QUEUE -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -I$(INCMYSQL) -I../src/mqtt -I../src/client -I../src/user -I../src/device -I../src -I$(SSLDIR)/include -O2 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -std=c++11 -fPIE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src-2f-http

clean-src-2f-http:
	-$(RM) ./src/http/httprequest.d ./src/http/httprequest.o ./src/http/httprequestactiontriggerextraparams.d ./src/http/httprequestactiontriggerextraparams.o ./src/http/httprequestextraparams.d ./src/http/httprequestextraparams.o ./src/http/httprequestqueue.d ./src/http/httprequestqueue.o ./src/http/httprequestvoiceassistantextraparams.d ./src/http/httprequestvoiceassistantextraparams.o ./src/http/trivialhttp.d ./src/http/trivialhttp.o ./src/http/trivialhttpfactory.d ./src/http/trivialhttpfactory.o ./src/http/trivialhttps.d ./src/http/trivialhttps.o

.PHONY: clean-src-2f-http

