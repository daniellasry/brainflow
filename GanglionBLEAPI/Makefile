CC = g++

TARGET = GanglionLibrary.so

MACHINE = $(shell $(CC) -dumpmachine)
PLATFORM = POSIX
LIBS = -lm
LDFLAGS = -shared

SRCS := $(wildcard ./Bglib/*.cpp)
OBJS := $(SRCS:.cpp=.o)
DEPS := $(SRCS:.cpp=.d)

all: $(TARGET)

%.o: %.cpp
	$(CC) -I./Interface/ -O3 -Wall -c -fmessage-length=0 -fPIC -DPLATFORM_$(PLATFORM) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"

$(TARGET): $(OBJS)
	@echo 'Building target: $@'
	$(CC) -o"$(TARGET)" $(OBJS) $(LIBS) $(LDFLAGS)
	@echo 'Finished building target: $@'

clean:
	-$(RM) $(OBJS) $(DEPS) $(TARGET)

.PHONY: all clean

