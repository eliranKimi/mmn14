################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../firstRead.c \
../generalFunctions.c \
../main.c \
../secondRead.c 

OBJS += \
./firstRead.o \
./generalFunctions.o \
./main.o \
./secondRead.o 

C_DEPS += \
./firstRead.d \
./generalFunctions.d \
./main.d \
./secondRead.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


