# these names are valid for Linux as well as WSL
NAME_MESSAGEBROKER = MessageBroker.exe
NAME_CLIENTA = ClientA.exe
NAME_CLIENTB = ClientB.exe
NAME_CLIENTC = ClientC.exe
NAME_COMMON_LIB = Common.lib 

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
			common/Client/Client.cpp \
			Main.cpp

# sorce files common to all 3 clients
SRC_COMMON = common/Client/Client.cpp \
			common/MiscFuncs.cpp

SRC_CA = 	ClientA/ClientA.cpp
SRC_CB = 	ClientB/ClientB.cpp
SRC_CC = 	ClientC/ClientC.cpp

# turn CPP filenames WITH path prefixed on them, into OBJ filenames, all in the same output directory.
# make sure you don't have multiple output OBJ files with the same name or you're toast.
OBJ_MB		=	$(addprefix $(OBJ_PATH), $(notdir $(SRC_MB:.cpp=.o)))
OBJ_CA		=	$(addprefix $(OBJ_PATH), $(notdir $(SRC_CA:.cpp=.o)))
OBJ_CB		=	$(addprefix $(OBJ_PATH), $(notdir $(SRC_CB:.cpp=.o)))
OBJ_CC		=	$(addprefix $(OBJ_PATH), $(notdir $(SRC_CC:.cpp=.o)))
OBJ_COMMON	=	$(addprefix $(OBJ_PATH), $(notdir $(SRC_COMMON:.cpp=.o)))

# get OBJ files by compiling CPP files
$(OBJ_PATH)%.o:MessageBroker/%.cpp
	mkdir -p $(OBJ_PATH)
	$(CC) $(CF) $(CFI) -c $< -o $@

# common path
$(OBJ_PATH)%.o:common/%.cpp
	mkdir -p $(OBJ_PATH)
	$(CC) $(CF) $(CFI) -c $< -o $@

# common/Client path
$(OBJ_PATH)%.o:common/Client/%.cpp
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

all: $(NAME_MESSAGEBROKER) $(NAME_COMMON_LIB) $(NAME_CLIENTA) $(NAME_CLIENTB) $(NAME_CLIENTC)

clean:
	rm -rf $(OBJ_PATH)
	rm -rf $(BUILD_PATH)

# get EXE files by combining OBJ  files
$(NAME_MESSAGEBROKER):	$(OBJ_MB)
	mkdir -p $(BUILD_PATH)
	$(CC) $(CF) $(CFI) -o $(BUILD_PATH)$(NAME_MESSAGEBROKER) $(OBJ_MB)

# the common library builds, but we'll just use the .o files
$(NAME_COMMON_LIB):	$(OBJ_COMMON)
	mkdir -p $(BUILD_PATH)
	$(CC) $(CF) $(CFI) -o $(BUILD_PATH)$(NAME_COMMON_LIB) $(OBJ_MB)

// combine the common library .o files
$(NAME_CLIENTA):	$(OBJ_CA) $(OBJ_COMMON)
	mkdir -p $(BUILD_PATH)
	$(CC) $(CF) $(CFI) -o $(BUILD_PATH)$(NAME_CLIENTA) $(OBJ_CA) $(OBJ_COMMON)

$(NAME_CLIENTB):	$(OBJ_CB) $(OBJ_COMMON)
	mkdir -p $(BUILD_PATH)
	$(CC) $(CF) $(CFI) -o $(BUILD_PATH)$(NAME_CLIENTB) $(OBJ_CB) $(OBJ_COMMON)

$(NAME_CLIENTC):	$(OBJ_CC) $(OBJ_COMMON)
	mkdir -p $(BUILD_PATH)
	$(CC) $(CF) $(CFI) -o $(BUILD_PATH)$(NAME_CLIENTC) $(OBJ_CC) $(OBJ_COMMON)
