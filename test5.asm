start:  ldc num1          
        ldnl 0            
        ldc num2          
        ldnl 0            
        add               

        ldc num3          
        ldnl 0            
        sub               

        ldc result        
        stnl 0            
        HALT              

; Data section
num1:     data 058        
num2:     data 30         
num3:     data 20         
result:   data 0          