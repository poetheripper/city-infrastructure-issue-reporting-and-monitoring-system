# AI Usage - Project Phase 1

1. The tool used: Gemini (Google AI)

2. The Prompts I gave:

"I am building a C program for a UNIX environment to manage city infrastructure reports. I've used a binary_file_report structure that stores the given information, for each report.
Please generate a function "int parse_condition(const char *input, char *field, char *op, char *value)" to split a 'field:operator:value' string, and a function "int match_condition(binary_report_file *r, const char *field, const char *op, const char *value)" that returns 1 if the record matches the criteria and 0 otherwise. It is very important that there can be multiple filtering criteria"

3. What was generated
The AI generated two functions. "parse_condition()" uses sscanf() with format specifiers to extract each element of the filtering condition. The function "match_condition()" uses a series of if/else statements to check the criteria string and converts the string to the correct C data type using "atoi()" or "atol(), plus it compares them them using the right operator.

4. The changes I've made
I adapted the struct pointer name in "match_condition()" to match my exact variable names. I've also changed the structure a bit, make it more clean and easy to read and comprehend, for my own understanding.

5. What I've learned
I learned how "sscanf()" can be better used with regular expression. I also learned how to dynamically map string operators (like ">=") to actual C code operators inside a functon.

# AI Usage - Project Phase 2

I've used AI based tools for the city_manager - monitor_reports conection logic and from a theoretical point of view, I've used it to better understand the signal manipulating functions and how to properly used them in a well structured code.