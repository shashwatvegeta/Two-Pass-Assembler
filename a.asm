    start:  ldc 50            ; Load the first number (50) into A
            ldc 30            ; Load the second number (30) into A, move the first number to B
            add               ; Add the two numbers, result in A
            adc 20            ; Load the third number (20) into A, move the sum to B
            stl 0             ; Add the third number to the result, final result in A
            HALT              ; Stop execution

