NAME = minishell

CC = cc
CFLAGS = -Wall -Wextra -Werror
AR = ar rcs
RM = rm -f

SRCDIR = src
INCDIR = includes
OBJDIR = obj
LIBFTDIR = libft
LIBFT = $(LIBFTDIR)/libft.a

SRCS = $(shell find $(SRCDIR) -name '*.c')
OBJS = $(SRCS:%.c=$(OBJDIR)/%.o)

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCDIR) -I$(LIBFTDIR) -c $< -o $@

all: $(NAME)

$(LIBFT):
	$(MAKE) -C $(LIBFTDIR)

$(NAME): $(LIBFT) $(OBJS)
	$(CC) $(CFLAGS) -I$(INCDIR) -I$(LIBFTDIR) $(OBJS) $(LIBFT) -lreadline -o $(NAME)

clean:
	$(MAKE) -C $(LIBFTDIR) clean
	$(RM) -r $(OBJDIR)

fclean: clean
	$(MAKE) -C $(LIBFTDIR) fclean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
