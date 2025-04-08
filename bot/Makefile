# COLORS

#Provided by Feuille Morte
define generate_random_color
python3 -c "import random; \
print(''.join(['\033[38;5;' + str(random.randint(16, 255)) + 'm' + c + '\033[0m' for c in '$(1)']))"
endef

# TARGET

NAME		=	Bot

# FLAGS

MAKEFLAGS	=	-s
CXX			=	c++
CXXFLAGS	=	-Wall -Wextra -Werror -g -std=c++98 -I../inc -I.

# FILES

FILES		=	main Bot

SRC			=	$(addsuffix .cpp, $(FILES))
OBJ			=	$(addsuffix .o, $(FILES))

# RULES

all			:	$(NAME)

$(NAME)		:	$(OBJ)
				$(CXX) $(CXXFLAGS) $^ -o $@
				@$(call generate_random_color, Compiled $@)

clean		:
				$(RM) $(OBJ)
				@$(call generate_random_color, Cleaned $(OBJ))

fclean		:	clean
				$(RM) $(NAME)
				@$(call generate_random_color, Cleaned $(NAME))

re			:	fclean all

.PHONY		=	all clean fclean re
