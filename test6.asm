;TITLE: Claims																																
;AUTHOR: Shashwat Kumar Singh
;Declaration of Authorship


start:  ldc 1000         ; Set up stack pointer
        a2sp
        adj -1

        ldc counter      ; Load counter address
        ldnl 0           ; Get initial value (5)
        ldc 0            ; Initialize array index
        stl 1            ; Store array index
        
loop:   ldc counter      ; Get current count
        ldnl 0
        
        ldc result       ; Store current count in array
        ldl 1            ; Get current array index
        add              ; Calculate address
        stl 2            ; Save address
        ldc counter
        ldnl 0
        ldl 2
        stnl 0           ; Store count in array
        
        ldc counter      ; Check if done
        ldnl 0
        brz done         ; If counter = 0, exit
        
        ldc counter      ; Decrement counter
        ldnl 0           ; Get current value
        ldc 1            
        sub              ; Subtract 1
        ldc counter
        stnl 0           ; Store decremented value
        
        ldc 1            ; Increment array index
        ldl 1
        add
        stl 1
        
        br loop          ; Continue loop

done:   HALT

; Data section
counter: data 5          ; Start counting from 5

; Array to store countdown sequence
result: data 0           ; Will store: 5,4,3,2,1,0
        data 0
        data 0
        data 0
        data 0
        data 0