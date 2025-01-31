CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -I$(INCDIR) -g

NAME = ircserv

SRCDIR = srcs
INCDIR = inc
OBJDIR = obj

SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

GREEN=\033[0;32m
RED=\033[0;31m
NC=\033[0m

all: $(NAME)
	@echo "$(GREEN)Build successful$(NC)"

$(NAME): $(OBJS)
	@echo "$(GREEN)Linking object files$(NC)"
	$(CXX) $(OBJS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	@echo "$(GREEN)Compiling $<$(NC)"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	@mkdir -p $(OBJDIR)

clean:
	@echo "$(RED)Cleaning up$(NC)"
	rm -rf $(OBJDIR)
	rm -rf *.dSYM

fclean: clean
	@echo "$(RED)Force cleaning$(NC)"
	rm -f $(NAME)

re: fclean all
	@echo "$(GREEN)Rebuild complete$(NC)"

.PHONY: all clean fclean re

# SRCS =	main.c init_env.c clean_fd.c get_opt.c x.c main_loop.c \
# 	init_fd.c do_select.c check_fd.c \
# 	srv_create.c srv_accept.c \
# 	client_read.c client_write.c

# OBJS = ${SRCS:.c=.o}

# NAME = bircd

# CFLAGS = -I. -g3 -Wall -Werror
# LDFLAGS = 

# CC = gcc
# RM = rm -f

# ${NAME}:	${OBJS}
# 		${CC} -o ${NAME} ${OBJS} ${LDFLAGS}

# allc:		${NAME}

# clean:
# 		${RM} ${OBJS} *~ #*#

# fclean:		clean
# 		${RM} ${NAME}

# re:		fclean all
