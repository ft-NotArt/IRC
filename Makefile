# COLORS

#Provided by Feuille Morte
define generate_random_color
python3 -c "import random; \
print(''.join(['\033[38;5;' + str(random.randint(16, 255)) + 'm' + c + '\033[0m' for c in '$(1)']))"
endef

# TARGET

NAME		=	IRC

# FLAGS

MAKEFLAGS	=	-s
CXX			=	c++
CXXFLAGS	=	-Wall -Wextra -Werror -g -std=c++98 -Iinc

# FILES

FILES		=	main Server Channel User Reply

SRC			=	$(addsuffix .cpp, $(FILES))
OBJ			=	$(addsuffix .o, $(addprefix src/, $(FILES)))

# RULES

all			:	$(NAME)

$(NAME)		:	$(OBJ)
				$(CXX) $(CXXFLAGS) $^ -o $@
				@$(call generate_random_color, Compiled $@)

clean		:
				$(RM) $(OBJ) cool_shrubbery
				@$(call generate_random_color, Cleaned $(OBJ))

fclean		:	clean
				$(RM) $(NAME)
				@$(call generate_random_color, Cleaned $(NAME))

re			:	fclean all

.PHONY		=	all clean fclean re

# DEBUG

irssi		:
				irssi -c localhost -p 5000 -n gobelin -w gobelin123

nc			:
				nc localhost 5000

.PHONY		=	connect_irssi connect_nc
