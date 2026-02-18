DEBUG_VALUE ?= 0
DBG ?= 1
FULL_NAME ?= 1

CXX = c++
CXXFLAGS = -std=c++98 -Wall -Werror -Wextra -MMD -DDEBUG_VALUE=${DEBUG_VALUE}
SHELL = /bin/bash

EXECFLAGS = 

FILES	=									\
				srcs/main.cpp				\

SERVER	=									\
				Config.cpp					\
				Location.cpp				\
				Server.cpp					

FILES	+=	$(addprefix srcs/server/,$(SERVER))

UTILS	=									\
				utils.cpp					

FILES	+=	$(addprefix srcs/utils/,$(UTILS))

CLIENT	=									\
				Client.cpp					\

FILES	+=	$(addprefix srcs/client/,$(CLIENT))

ENGINE	=									\
				Engine.cpp					\
				Listener.cpp				\

FILES	+=	$(addprefix srcs/engine/,$(ENGINE))

PARSING_CONFIG_FILE	=			

RESPONSE_FILE =								\
				httpResponse.cpp			\

		
FILES	+=	$(addprefix srcs/parsing_config_file/,$(PARSING_CONFIG_FILE))

FILES	+=	$(addprefix srcs/response/,$(RESPONSE_FILE))

OBJS	=	$(addprefix .obj/, $(FILES:.cpp=.o))

DEPS	=	$(addprefix .obj/, $(FILES:.cpp=.d))

# path to lib folder to exec norm
NORM_LIB = \
lib/

# path to .h for the main project basic
MY_HEADER =				\

# set the main project header dir
HEADER_DIR = \
includes

# set the -I before the path
# the path to the header lib dir
ALL_I_DIR_HEADER	=				\
-I includes							\
-I includes/server					\
-I includes/response				\
-I includes/utils					\
-I includes/engine					\

CURRENT_HEADERS = $(ALL_I_DIR_HEADER)

# exec name for bonus

#executable name for the main project
NAME = webserv

G3 = OFF

MSG="modif Makefile"

MAKEFLAGS += --no-print-directory

.DEFAULT_GOAL = all

#
#
#
#
#
#			██████╗ ███████╗██████╗ ███████╗███████╗██╗███╗   ██╗██╗████████╗██╗ ██████╗ ███╗   ██╗
#			██╔══██╗██╔════╝██╔══██╗██╔════╝██╔════╝██║████╗  ██║██║╚══██╔══╝██║██╔═══██╗████╗  ██║
#			██████╔╝█████╗  ██║  ██║█████╗  █████╗  ██║██╔██╗ ██║██║   ██║   ██║██║   ██║██╔██╗ ██║
#			██╔══██╗██╔══╝  ██║  ██║██╔══╝  ██╔══╝  ██║██║╚██╗██║██║   ██║   ██║██║   ██║██║╚██╗██║
#			██║  ██║███████╗██████╔╝███████╗██║     ██║██║ ╚████║██║   ██║   ██║╚██████╔╝██║ ╚████║
#			╚═╝  ╚═╝╚══════╝╚═════╝ ╚══════╝╚═╝     ╚═╝╚═╝  ╚═══╝╚═╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝

-include $(DEPS)

ifeq ($(G3), ON)
CXXFLAGS += -g3
endif

ifeq ($(MY_NAME), FULL)
FULL_NAME = 1
else
	ifneq ($(MY_NAME),)
		FULL_NAME = 0
	endif
endif

ifeq (${DBG}, 1)
	ifeq (${DEBUG_VALUE}, 1)
override DEBUG_VALUE = 0
override DBG = 2
	endif
endif


#
#
#
#
#
#			██████╗ ██╗   ██╗██╗     ███████╗███████╗
#			██╔══██╗██║   ██║██║     ██╔════╝██╔════╝
#			██████╔╝██║   ██║██║     █████╗  ███████╗
#			██╔══██╗██║   ██║██║     ██╔══╝  ╚════██║
#			██║  ██║╚██████╔╝███████╗███████╗███████║
#			╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚══════╝╚══════╝

#	▌     ▘
#	▛▌▀▌▛▘▌▛▘▛▘
#	▙▌█▌▄▌▌▙▖▄▌

.PHONY:all
all: change_name_full clear_console reset_debug $(STATIC_LIB) $(EXTERN_LIB) start_build_aff ${NAME}


${NAME}: CURRENT_HEADERS = $(ALL_I_DIR_HEADER)
${NAME}:  $(STATIC_LIB) $(EXTERN_LIB) $(OBJS)
	@echo -e "    ${_BOLD}${_GREEN}💿  ◀◀◀ ${_LIME}Creating Executable 📑🗂️   ${_YELLOW}$(NAME)${_END}"
	@$(CXX) $(OBJS) $(STATIC_LIB) $(EXTERN_LIB) -o $(NAME) $(EXECFLAGS)


.obj/%.o: %.cpp
	@mkdir -p $(call GET_ALL_FOLDER,$@)6
	@if [ "${FULL_NAME}" = "1" ]; then\
		echo -e "	${_LIME}${_BOLD}${NAME}	${_END}${_GREEN}Compiling : ${_END}$(CXX) ${_BLUE} $(CXXFLAGS) ${_PURPLE} $<${_END}";\
	else \
		echo -e "	${_LIME}${_BOLD}${NAME}	${_END}${_GREEN}Compiling : ${_END}$(CXX) ${_BLUE} $(CXXFLAGS) ${_PURPLE} $(call GET_FILE,$(call DELETE_EXT,$<))${_END}";\
	fi;
	@$(CXX) $(CURRENT_HEADERS) $(CXXFLAGS) -c $< -o $@



#	  ▜
#	▛▘▐ █▌▀▌▛▌
#	▙▖▐▖▙▖█▌▌▌

.PHONY:re
re: fclean all

.PHONY:clean
clean:
	@rm -rf .obj
	@echo -e "    ${_RED}${_BOLD}🗑️  ◀◀◀ Deleting files 🚮 ${_YELLOW}${NAME} objects${_END}\n"

.PHONY:fclean
fclean: clean cln

.PHONY:cln
cln:
	@rm -rf $(NAME)
	@echo -e "    ${_RED}${_BOLD}🗑️  ◀◀◀ Deleting file  🚮 ${_YELLOW}$(NAME)${_END}\n"

#
#
#
#
#
#			███╗   ███╗███████╗███████╗███████╗ █████╗  ██████╗ ███████╗
#			████╗ ████║██╔════╝██╔════╝██╔════╝██╔══██╗██╔════╝ ██╔════╝
#			██╔████╔██║█████╗  ███████╗███████╗███████║██║ ████╗█████╗
#			██║╚██╔╝██║██╔══╝  ╚════██║╚════██║██╔══██║██║   ██║██╔══╝
#			██║ ╚═╝ ██║███████╗███████║███████║██║  ██║╚██████╔╝███████╗
#			╚═╝     ╚═╝╚══════╝╚══════╝╚══════╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝

.PHONY:start_build_aff
start_build_aff:
	@echo -e -n "\n${_BOLD}${_RED}Start Building  ⚙️  🛠️  ⚙️ 	${_GREEN}${_UNDER}$(NAME)${_END} :	"
	@echo "( $(call TO_MAJ_OBJ,${FILES}) / $(call NB_FILE,${FILES}) to update )"

.PHONY:clear_console
clear_console:
	clear

#
#
#
#
#
#			██████╗ ███████╗██████╗ ██╗   ██╗ ██████╗
#			██╔══██╗██╔════╝██╔══██╗██║   ██║██╔════╝
#			██║  ██║█████╗  ██████╔╝██║   ██║██║  ███╗
#			██║  ██║██╔══╝  ██╔══██╗██║   ██║██║   ██║
#			██████╔╝███████╗██████╔╝╚██████╔╝╚██████╔╝
#			╚═════╝ ╚══════╝╚═════╝  ╚═════╝  ╚═════╝


#	      ▌ ▜                ▐▘  ▜ ▜ 
#	█▌▛▌▀▌▛▌▐ █▌  ▛▌▀▌▛▛▌█▌  ▜▘▌▌▐ ▐ 
#	▙▖▌▌█▌▙▌▐▖▙▖  ▌▌█▌▌▌▌▙▖  ▐ ▙▌▐▖▐▖
#                                 

.PHONY:change_name_full
change_name_full:
	@if [ "${MY_NAME}" = "FULL" ]; then\
		$(call MODIF_FULL_NAME_VALUE,1)\
	else \
		if [ "${MY_NAME}" != "" ]; then\
			$(call MODIF_FULL_NAME_VALUE,0)\
		fi;\
	fi


#	      ▌ ▜         ▘  ▗    ▌  ▌
#	█▌▛▌▀▌▛▌▐ █▌  ▛▌▛▘▌▛▌▜▘  ▛▌█▌▛▌▌▌▛▌
#	▙▖▌▌█▌▙▌▐▖▙▖  ▙▌▌ ▌▌▌▐▖  ▙▌▙▖▙▌▙▌▙▌
#	              ▌                  ▄▌
.PHONY:debug
debug: clear_console
	@$(call PRINT_GET_DBG)\
	read -s -n 1 dbg;\
	while ! echo $$dbg | grep -Eq "[01]"; do\
		clear;\
		$(call BAD_VALUE_GET,$$dbg)\
		$(call PRINT_GET_DBG)\
		read -s -n 1 dbg;\
	done;\
	old_val="$$(cat Makefile | grep -oP '^DEBUG_VALUE \?= \K\d+')";\
	if [ "$$old_val" != "$$dbg" ]; then\
		$(call MODIF_DEBUG_VALUE,$$dbg)\
		touch ${HEADER_DIR}/header_mandatory/debug.h;\
	fi;\
	make DBG=0

.PHONY:reset_debug
reset_debug:
	@if [ "${DBG}" = "2" ]; then\
		$(call MODIF_DEBUG_VALUE,0)\
		touch ${HEADER_DIR}/header_mandatory/debug.h;\
	fi;

#
#
#
#
#
#			██╗   ██╗████████╗██╗██╗     ███████╗
#			██║   ██║╚══██╔══╝██║██║     ██╔════╝
#			██║   ██║   ██║   ██║██║     ███████╗
#			██║   ██║   ██║   ██║██║     ╚════██║
#			╚██████╔╝   ██║   ██║███████╗███████║
#			 ╚═════╝    ╚═╝   ╚═╝╚══════╝╚══════╝


.PHONY:push_all
push_all:
	make fclean_all
	git add .
	git commit -m ${MSG}
	git push


start_valgrind:
	valgrind --show-leak-kinds=all --leak-check=full --suppressions=.supp ./cub3D maps/t1.cub 

#
#
#
#
#
#			███╗   ███╗ █████╗  ██████╗██████╗  ██████╗
#			████╗ ████║██╔══██╗██╔════╝██╔══██╗██╔═══██╗
#			██╔████╔██║███████║██║     ██████╔╝██║   ██║
#			██║╚██╔╝██║██╔══██║██║     ██╔══██╗██║   ██║
#			██║ ╚═╝ ██║██║  ██║╚██████╗██║  ██║╚██████╔╝
#			╚═╝     ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝ ╚═════╝


#	      ▌▘▐▘      ▗ ▌
#	▛▛▌▛▌▛▌▌▜▘  ▛▌▀▌▜▘▛▌
#	▌▌▌▙▌▙▌▌▐   ▙▌█▌▐▖▌▌
#	            ▌
define	GET_X_FOLDER
$$(\
	echo -e $(1) | sed -E "s/(\/[^\/]+){$2}$$//g";\
)
endef

define	GET_FIRST_FOLDER
$$(\
	echo -e $(1) | sed 's/\/.*//g';\
)
endef

define	GET_ALL_FOLDER
$$(\
	var=$$(echo -e $(1) | sed -E 's/\/?[^\/]+$$//g');\
	if [ "$$var" = "" ];then\
		echo "$$var";\
	else\
		echo "$$var/";\
	fi;\
)
endef

define	GET_LIST_FOLDER
$$(\
	echo -e $(1) | sed 's/\//\t/g';\
)
endef

define	DELETE_EXT
$$(\
	echo -e "$(1)" | sed -E 's/\.[a-z]+$$//g';\
)
endef


#	    ▘  ▗
#	▛▌▛▘▌▛▌▜▘  ▛▛▌█▌▛▘▛▘▀▌▛▌█▌
#	▙▌▌ ▌▌▌▐▖  ▌▌▌▙▖▄▌▄▌█▌▙▌▙▖
#	▌                     ▄▌
define MSG_LAUCH
echo -e "${_GREEN}Lauching ${_PURPLE}${_BOLD}${NAME}${_END} ${_GREEN}with >>>${_END}"
endef

define	BAD_VALUE_GET
echo -e "${_RED}${_BOLD}Error ${_LIME}'$(1)' ${_RED}This is not a good value ${_END}";
endef

define	PRINT_GET_DBG
echo -e "${_BOLD}${_CYAN}Give a Value :${_END}";\
echo -e "\t${_PURPLE}Program ${_LIME}${_BOLD}Without ${_END}${_PURPLE}debug print >>>\t${_ORANGE}${_BOLD}0${_END}";\
echo -e "\t${_PURPLE}Program ${_RED}${_BOLD} With   ${_END}${_PURPLE}debug print >>>\t${_ORANGE}${_BOLD}1${_END}";
endef


#	▐▘▘▜            ▌
#	▜▘▌▐ █▌▛▘  ▟▖  ▛▌█▌▛▌▛▘
#	▐ ▌▐▖▙▖▄▌  ▝   ▙▌▙▖▙▌▄▌
#	                   ▌
define	GET_FILE
$$(\
	echo -e $(1) | sed 's/.*\///g';\
)
endef

define	NB_FILE
$$(\
	echo $(1) | wc -w;\
)
endef

define	TO_MAJ_OBJ
$$(\
	to_maj=0;\
	for file in ${FILES}; do\
		if [ "$(call DEPS_NEWER,$$file)" != "0" ];then\
			to_maj=$$((to_maj + 1));\
		fi;\
	done;\
	echo "$$to_maj";\
)
endef

define DEPS_NEWER
$$(\
	name="$(1)";\
	dfile=".obj/$$(echo $$name | sed 's/\.c/\.d/g')";\
	if [ -r "$$dfile" ];then\
		all="$$(cat $$dfile | sed ':a; N; $$! ba; s/\\\n//g')";\
		first=$$(echo "$$all" | sed -E 's/(.*.o):([^.o].*)/\1/g');\
		deps=$$(echo "$$all" | sed -E 's/(.*.o):([^.o].*)/\2/g');\
		older=0;\
		newer=0;\
		for file in $$deps;do\
			if [ "$$file" -nt "$$first" ];then\
				newer=$$((newer + 1));\
			else\
				older=$$((older + 1));\
			fi;\
		done;\
		echo "$$newer";\
	else\
		echo "1";\
	fi;\
)
endef


#	  ▗ ▘▜
#	▌▌▜▘▌▐ ▛▘
#	▙▌▐▖▌▐▖▄▌

define	EXIT_CODE
$$(\
	echo $$?;\
)
endef

define	MODIF_DEBUG_VALUE
content="$$(cat Makefile | sed -E "s/DEBUG_VALUE \?= [0-9]/DEBUG_VALUE \?= $(1)/g")";\
echo "$$content" > Makefile;
endef

define	MODIF_FULL_NAME_VALUE
content="$$(cat Makefile | sed -E "s/FULL_NAME \?= [0-9]/FULL_NAME \?= $(1)/g")";\
echo "$$content" > Makefile;
endef


#
#
#
#
#
#			 ██████╗ ██████╗ ██╗      ██████╗ ██████╗
#			██╔════╝██╔═══██╗██║     ██╔═══██╗██╔══██╗
#			██║     ██║   ██║██║     ██║   ██║██████╔╝
#			██║     ██║   ██║██║     ██║   ██║██╔══██╗
#			╚██████╗╚██████╔╝███████╗╚██████╔╝██║  ██║
#			 ╚═════╝ ╚═════╝ ╚══════╝ ╚═════╝ ╚═╝  ╚═╝

_END=		\033[0m
_BOLD=		\033[1m
_ITALIC=	\033[3m
_UNDER=		\033[4m
_REV=		\033[7m
_ROD=		\033[9m
_ERASE=		\033[2K

# Colors
_GREY=		\033[30m
_RED=		\033[31m
_GREEN=		\033[32m
_YELLOW=	\033[33m
_BLUE=		\033[34m
_PURPLE=	\033[35m
_CYAN=		\033[36m
_WHITE=		\033[37m
_LIME=		\033[38;2;0;255;0m
_ORANGE=	\033[38;2;255;165;0m

# Backgroud Colors
_BGREY=		\033[40m
_BRED=		\033[41m
_BGREEN=	\033[42m
_BYELLOW=	\033[43m
_BBLUE=		\033[44m
_BPURPLE=	\033[45m
_BCYAN=		\033[46m
_BWHITE=	\033[47m
_BLIME=		\033[48;2;0;255;0m
_BORANGE=	\033[48;2;255;165;0m
