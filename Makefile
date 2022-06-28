#cordic-test-sincos \cordic-test-atan \cordic-test-sinhcosh \cordic-test-arctanh \cordic-test-ln \cordic-test-sqrt \cordic-test-phase \cordic-test-atan-all 
TARGETS = cordic-test-atan-all \
        cordic-test-sinhcosh-all \
        cordic-test-arctanh-all \
        cordic-test-ln-all \
        cordic-test-sqrt-all \
        cordic-test-sincos-many \
        cordic-test-sqrt \
        cordic-test-phase-many \
        cordic-test-sinhcosh \
        cordic-test-sincos \
        cordic-test-phase \
        cordic-test-ln \
        cordic-test-arctanh \
        cordic-test-atan \
	cordic-test-phase-debug \
	cordic-test-phase-18MSB

CPPFLAGS =
all: $(TARGETS)
DEBUG = 0
cordic-test-phase-debug: cordic-test-phase-debug.c cordic_verilog.h
	gcc $< -Ddebug=1 -o $@ -lm
cordic-test-%: cordic-test-%.c cordic_verilog.h cordic_error.c cordic_error.h
	gcc $< -Ddebug=$(DEBUG) cordic_error.c -o $@ -lm -Wall

clean:
	rm -f $(TARGETS)
