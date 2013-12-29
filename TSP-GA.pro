TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

#MPI settings
QMAKE_CC = mpicc
QMAKE_LINK = $$QMAKE_CC

LIBS += -lm


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


