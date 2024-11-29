; Test error handling
label:
label: ; duplicate label definiton
br nonesuch ; no such label
ldc ; missing operand
ldc 5, 6; extra on end of line
0def: ; bogus label name
fibble; bogus mnemonic
0def ; bogus mnemonic
