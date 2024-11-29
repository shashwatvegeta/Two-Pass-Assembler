import os
import subprocess
import google.generativeai as genai
import sys

class AssemblyImprover:
    def __init__(self):
        
        api_key = 'AIzaSyA3zSxR7ljFbQZisDr3kiwPJNZVi9d5Q1s'  
        genai.configure(api_key=api_key)
        self.model = genai.GenerativeModel("gemini-1.5-flash")

    def improve_code(self, asm_code: str) -> str:
        """Send assembly code to Gemini for improvement and analysis."""
        prompt = f"""
        Analyze and improve this assembly code which I made to work on my assembler and emulator:
        Here is the sample addition code for reference:
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
            num1:     data 50         
            num2:     data 30         
            num3:     data 20         
            result:   data 0  
        I have certain rules also I am only using two register A and B.
        and ldnl is not unnnecessary it is very important consider these while generating something new 
        the rules are as follows:
            Mnemonic | Opcode | Operand | Formal Specification | Description
            data | - | value | - | Reserve a memory location, initialized to the value specified
            ldc | 0 | value | B := A; A := value; | Load accumulator with the value specified
            adc | 1 | value | A := A + value; | Add the value specified to the accumulator
            ldl | 2 | offset | B := A; A := memory[SP + offset]; | Load local
            stl | 3 | offset | memory[SP + offset] := A; A := B; | Store local
            ldnl | 4 | offset | A := memory[A + offset]; | Load non-local
            stnl | 5 | offset | memory[A + offset] := B; | Store non-local
            add | 6 | - | A := B + A; | Addition
            sub | 7 | - | A := B - A; | Subtraction
            shl | 8 | - | A := B << A; | Shift left
            shr | 9 | - | A := B >> A; | Shift right
            adj | 10 | value | SP := SP + value; | Adjust SP
            a2sp | 11 | - | SP := A; A := B; | Transfer A to SP
            sp2a | 12 | - | B := A; A := SP; | Transfer SP to A
            call | 13 | offset | B := A; A := PC; PC := PC + offset; | Call procedure
            return | 14 | - | PC := A; A := B; | Return from procedure
            brz | 15 | offset | if A == 0 then PC := PC + offset; | If accumulator is zero, branch to specified offset
            brlz | 16 | offset | if A < 0 then PC := PC + offset; | If accumulator is less than zero, branch to specified offset
            br | 17 | offset | PC := PC + offset; | Branch to specified offset
            HALT | 18 | - | - | Stop the emulator. This is not a "real" instruction but tells the emulator when to finish.
            SET | - | value | - | Set the label on this line to the specified value (rather than the PC). Optional extension for additional marks.
        Take this sample code and rules as a reference and improve the following code:        
        {asm_code}

        Please provide:
        1. An improved version of the code with detailed comments explaining what each instruction does
        2. Any optimizations made
        3. Potential error fixes
        
        Return ONLY the improved code with comments, nothing else.
        remove the first and last line while returning ```assembly and last
        remove these two lines after the final answer
        """

        try:
            response = self.model.generate_content(prompt)
            return response.text.strip()
        except Exception as e:
            print(f"Error generating content: {str(e)}")
            return asm_code  # Return original code if generation fails

    def process_file(self, input_file: str) -> tuple[bool, str]:
        """Process an assembly file through Gemini and run the assembler."""
        try:
            with open(input_file, 'r') as f:
                original_code = f.read()

            improved_code = self.improve_code(original_code)

            with open(input_file, 'w') as f:
                f.write(improved_code)

            result = subprocess.run(['./asm', input_file], 
                                 capture_output=True, 
                                 text=True)

            base_name = os.path.splitext(input_file)[0]
            log_file = f"{base_name}.log"


            if os.path.exists(log_file):
                with open(log_file, 'r') as f:
                    log_content = f.read()
            else:
                log_content = "Log file not generated"


            success = os.path.exists(f"{base_name}.o") and \
                     os.path.exists(f"{base_name}.lst")

            return success, log_content

        except Exception as e:
            return False, f"Error occurred: {str(e)}"

def main():
    if len(sys.argv) != 2:
        print("Usage: python improve_asm.py <assembly_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    if not os.path.exists(input_file):
        print(f"File not found: {input_file}")
        sys.exit(1)


    backup_file = input_file + '.backup'
    with open(input_file, 'r') as src, open(backup_file, 'w') as dst:
        dst.write(src.read())

    improver = AssemblyImprover()
    success, log_content = improver.process_file(input_file)

    print("\n=== Assembly Processing Results ===")
    if success:
        print("✅ Assembly successful!")
        print("\nGenerated files:")
        base_name = os.path.splitext(input_file)[0]
        print(f"- {base_name}.o (object file)")
        print(f"- {base_name}.lst (listing file)")
        print(f"- {base_name}.log (log file)")
    else:
        print("❌ Assembly failed!")
    
    print("\nLog content:")
    print(log_content)
    
    print(f"\nOriginal code backed up to: {backup_file}")

if __name__ == "__main__":
    main()