src = $(wildcard *.c)                    #拿到所有*.c
target  = $(patsubst %.c, %, $(src))     #将src中所有%.c替换成%
 
ALL:$(target)
 
%:%.c
	gcc $< -o $@
 
clean:
	-rm -rf $(target)
 
.PHONY:clean ALL
