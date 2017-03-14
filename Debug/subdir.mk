################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../firstRead.c \
../main.c \
../secondRead.c \
../utility.c 

O_SRCS += \
../firstRead.o \
../main.o \
../secondRead.o \
../utility.o 

OBJS += \
./firstRead.o \
./main.o \
./secondRead.o \
./utility.o 

C_DEPS += \
./firstRead.d \
./main.d \
./secondRead.d \
./utility.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


