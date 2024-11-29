    start:  ldc num1          ; Load address of num1 into A
            ldnl 0            ; Load value at address num1 into A
            
            ldc num2          ; Load address of num2 into A
            ldnl 0            ; Load value at address num2 into A
            sub               ; Compare num1 and num2 (A = num1 - num2)
            brz skip1         ; If num1 == num2, keep num1 in A
            brlz use_num2     ; If num1 < num2, branch to use num2
    skip1:  ldc num1          ; Reload num1 to A (num1 is larger)
            br next           ; Go to the next step
    use_num2: ldc num2        ; Load num2 into A (num2 is larger)
    
    next:   ldc num3          ; Load address of num3 into A
            ldnl 0            ; Load value at address num3 into A
            sub               ; Compare current max with num3
            brz skip2         ; If current max == num3, keep current max in A
            brlz use_num3     ; If current max < num3, branch to use num3
    skip2:  br next2          ; Go to the next step
    use_num3: ldc num3        ; Load num3 into A (num3 is larger)
    
    next2:  ldc num4          ; Load address of num4 into A
            ldnl 0            ; Load value at address num4 into A
            sub               ; Compare current max with num4
            brz skip3         ; If current max == num4, keep current max in A
            brlz use_num4     ; If current max < num4, branch to use num4
    skip3:  br next3          ; Go to the next step
    use_num4: ldc num4        ; Load num4 into A (num4 is larger)
    
    next3:  ldc num5          ; Load address of num5 into A
            ldnl 0            ; Load value at address num5 into A
            sub               ; Compare current max with num5
            brz skip4         ; If current max == num5, keep current max in A
            brlz use_num5     ; If current max < num5, branch to use num5
    skip4:  br done           ; All comparisons done, max is in A
    use_num5: ldc num5        ; Load num5 into A (num5 is larger)

    done:   ldc result        ; Load address of result into A
            stnl 0            ; Store the maximum value in result
            HALT              ; Stop execution

    ; Data section
    num1:   data 50           ; First number
    num2:   data 30           ; Second number
    num3:   data 70           ; Third number
    num4:   data 20           ; Fourth number
    num5:   data 60           ; Fifth number
    result: data 0            ; Memory location to store the maximum
