.PHONY: all clean viewer dvi dist test
SRCS = 	athenianTransform.c \
		fileReader.c \
		graphics.c \
		s21_matrix.c
		
PACKAGE   = viewer
VERSION   = ` date +"%Y.%m.%d" `
ARCHIVE   = $(PACKAGE)-$(VERSION)

OBJS = ${SRCS:.c=.o}
CC = gcc
CFLAGS = -Wall -Werror -Wextra

all: clean viewer

install: uninstall viewer
	@mkdir "/Users/$(USER)/Desktop/3DViewer/"
	@mkdir "/Users/$(USER)/Desktop/3DViewer/objects"
	@install ./viewer *.ui "/Users/$(USER)/Desktop/3DViewer/"

uninstall:
	@rm -rf "/Users/$(USER)/Desktop/3DViewer/"
	
dist:
	@tar -zcf $(ARCHIVE).tar *.c *.h Makefile *.ui

viewer:
	${CC} ${CFLAGS} -c ${SRCS} `pkg-config --cflags --libs gtk4` `pkg-config --cflags --libs MagickWand`
	${CC} ${CFLAGS} viewer.c ${SRCS} `pkg-config --cflags --libs gtk4` -o viewer `pkg-config --cflags --libs MagickWand`
	
test: unit_test

dvi:
	open dvi/index.html

unit_test:
	${CC} -c s21_viewer-test.c s21_matrix.c athenianTransform.c fileReader.c
	${CC} s21_viewer-test.o s21_matrix.o athenianTransform.o fileReader.o -lcheck -o test
	./test

gcov_report: viewer
	@gcc --coverage s21_viewer-test.c -lcheck s21_matrix.c athenianTransform.c fileReader.c -o unit-test
	@./unit-test
	@lcov -t "test" -o test.info -c -d .
	@genhtml -o report test.info
	
clean:
	@/bin/rm -rf *.o *.a unit-test *.gcno *gcda report *.info viewer main test

checks: cppcheck leaks style

style:
	python3 ../materials/linters/cpplint.py --extensions=c *.c *.h

cppcheck:
	cppcheck --enable=all --suppress=missingIncludeSystem . *.c

leaks:
	CK_FORK=no leaks --atExit -- ./viewer
	
run: clean viewer
	./viewer
