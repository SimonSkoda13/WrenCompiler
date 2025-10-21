# Vysvetlivky k stavom konečného automatu

<div style="display: flex; gap: 40px; margin-top: 20px;">

<div style="flex: 1;">

### Konečné stavy

- **F1** = OP_ADD
- **F2** = OP_SUB
- **F3** = OP_MUL
- **F4** = OP_DIV
- **F5** = OP_ASSIGN
- **F6** = OP_EQUALS
- **F7** = OP_LESS_THAN
- **F8** = OP_LESS_EQUAL
- **F9** = OP_GREATER_THAN
- **F10** = OP_GREATER_THAN_EQUAL
- **F11** = OP_NOT_EQUALS
- **F12** = LEFT_PAREN
- **F13** = RIGHT_PAREN
- **F14** = LEFT_BRACE
- **F15** = RIGHT_BRACE
- **F16** = NUM_INT
- **F17** = NUM_FLOAT
- **F18** = NUM_HEX
- **F19** = NUM_EXP
- **F20** = END_OF_FILE
- **F21** = STRING_OR_MULTILINE
- **F22** = STRING_LITERAL
- **F23** = IDENTIFIER_OR_KEYWORD
- **F24** = GLOBAL_VAR
- **F25** = DOT

</div>

<div style="flex: 1;">

### Pomocné stavy

- **C1** = LINE_COMMENT
- **C2** = BLOCK_COMMENT
- **C3** = BLOCK_COMMENT_END
- **Q1** = NOT
- **Q2** = HEX_OR_INT_LITERAL
- **Q3** = HEX_NUM_START
- **Q4** = DECIMAL_POINT
- **Q5** = NUM_EXP_START
- **Q6** = NUM_EXP_SIGN
- **Q7** = STRING_LITERAL_START
- **Q8** = STRING_MULTILINE_CONTENT
- **Q9** = STRING_MULTILINE_END_START
- **Q10** = STRING_MULTILINE_END_START
- **Q11** = STRING_LITERAL_CONTENT
- **Q12** = ESCAPE_SEQUENCE_BACKSLASH
- **Q13** = ESCAPE_SEQUENCE_HEX_START
- **Q14** = ESCAPE_SEQUENCE_HEX_END
- **Q15** = GLOBAL_VAR_START
- **Q16** = GLOBAL_VAR_NAME

</div>

</div>
