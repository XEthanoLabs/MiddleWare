# these names are valid for Linux as well as WSL
NAME_MESSAGEBROKER = MessageBroker.exe
NAME_CLIENTA = ClientA.exe
NAME_CLIENTB = ClientB.exe
NAME_CLIENTC = ClientC.exe

# compiler, flags, and includes
CC = 		g++
CF =		-g -Wall -Wextra
CFI = 		-I . -I ~/repos/boost_1_84_0/

# directories to build to
OBJ_PATH = ./obj/
BUILD_PATH = ./build/

# these are the files to use for each target.
# might be able to use some kind of wildcard?
SRC_MB = 	MessageBroker/MessageBroker.cpp \
			MessageBroker/TopicChat.cpp \
			MessageBroker/ConnectedClient.cpp \
			common/MiscFuncs.cpp \
			Main.cpp

SRC_CA = 	ClientA/ClientA.cpp \
			common/MiscFuncs.cpp

SRC_CB = 	ClientB/ClientB.cpp \
			common/MiscFuncs.cpp

SRC_CC = 	ClientC/ClientC.cpp \
			common/MiscFuncs.cpp

# turn CPP filenames WITH path prefixed on them, into OBJ filenames, all in the same output directory.
# make sure you don't have multiple output OBJ files with the same name or you're toast.
OBJ_MB =	$(addprefix $(OBJ_PATH), $(notdir $(SRC_MB:.cpp=.o)))
OBJ_CA =	$(addprefix $(OBJ_PATH), $(notdir $(SRC_CA:.cpp=.o)))
OBJ_CB =	$(addprefix $(OBJ_PATH), $(notdir $(SRC_CB:.cpp=.o)))
OBJ_CC =	$(addprefix $(OBJ_PATH), $(notdir $(SRC_CC:.cpp=.o)))

# get OBJ files by compiling CPP files
$(OBJ_PATH)%.o:MessageBroker/%.cpp
	mkdir -p $(OBJ_PATH)
	$(CC) $(CF) $(CFI) -c $< -o $@

# common path
$(OBJ_PATH)%.o:common/%.cpp
	mkdir -p $(OBJ_PATH)
	$(CC) $(CF) $(CFI) -c $< -o $@


# get OBJ files by compiling CPP files
$(OBJ_PATH)%.o:ClientA/%.cpp
	mkdir -p $(OBJ_PATH)
	$(CC) $(CF) $(CFI) -c $< -o $@

# get OBJ files by compiling CPP files
$(OBJ_PATH)%.o:ClientB/%.cpp
	mkdir -p $(OBJ_PATH)
	$(CC) $(CF) $(CFI) -c $< -o $@

# get OBJ files by compiling CPP files
$(OBJ_PATH)%.o:ClientC/%.cpp
	mkdir -p $(OBJ_PATH)
	$(CC) $(CF) $(CFI) -c $< -o $@

all: $(NAME_MESSAGEBROKER) $(NAME_CLIENTA) $(NAME_CLIENTB) $(NAME_CLIENTC)

clean:
	rm -rf $(OBJ_PATH)
	rm -rf $(BUILD_PATH)

# get EXE files by combining OBJ  files
$(NAME_MESSAGEBROKER):	$(OBJ_MB)
	mkdir -p $(BUILD_PATH)
	$(CC) $(CF) $(CFI) -o $(BUILD_PATH)$(NAME_MESSAGEBROKER) $(OBJ_MB)

# get EXE files by combining OBJ  files
$(NAME_CLIENTA):	$(OBJ_CA)
	mkdir -p $(BUILD_PATH)
	$(CC) $(CF) $(CFI) -o $(BUILD_PATH)$(NAME_CLIENTA) $(OBJ_CA)

# get EXE files by combining OBJ  files
$(NAME_CLIENTB):	$(OBJ_CB)
	mkdir -p $(BUILD_PATH)
	$(CC) $(CF) $(CFI) -o $(BUILD_PATH)$(NAME_CLIENTB) $(OBJ_CB)

# get EXE files by combining OBJ  files
$(NAME_CLIENTC):	$(OBJ_CC)
	mkdir -p $(BUILD_PATH)
	$(CC) $(CF) $(CFI) -o $(BUILD_PATH)$(NAME_CLIENTC) $(OBJ_CC)
