TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CFLAGS *= -O3

#MPI settings
QMAKE_CC = mpicc
QMAKE_LINK = $$QMAKE_CC

#QMAKE_CC = gcc
#QMAKE_CFLAGS *= -fopenmp

LIBS += -lm
#LIBS += -fopenmp
#LIBS += -pthread


SOURCES += \
    TPA-GA.c

OTHER_FILES += \
    README.md

HEADERS += \
    travel_manager.h \
    mpi_manager.h \
    city.h \
    circular_buffer.h \
    algorithm.h \
    cb_extension.h \
    genetic_algorithm.h


