CC	= gcc
CFLAGS	= -O0 -Wall
PROGRAM = multi-ref-test-exe

all: $(PROGRAM)
$(PROGRAM): multi-reference-test.o libgldll.a
	$(CC) $(CFLAGS) $< -o $(PROGRAM) -L . -lgldll

multi-reference-test.o: multi-reference-test.c
	$(CC) $(CFLAGS) -c $< -o $@

glthreads.o: glthreads.c
	$(CC) $(CFLAGS) -c $< -o $@

libgldll.a: glthreads.o
	ar rs $@ $<

.PHONY:clean
clean:
	@rm -f $(PROGRAM) *.o libgldll.a

.PHONY:test
test: $(PROGRAM)
	@echo "The check of multi-reference scenario is successful when it returns 0."
	@./$(PROGRAM) > /dev/null 2>&1 && echo ">>> $$?"
