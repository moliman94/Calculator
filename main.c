/*
 * calculator_app_1.1.c
 *
 * Created: 03-Apr-20 8:04:26 PM
 * Author : Mohamed Soliman
 */ 

#include <avr/io.h>
#include "Header.h"
#include "LCD.h"
#include "keypad.h"

#define ARR_SIZE 10


int main(void)
{
	LCD_init();
	keypad_init();
	uint8 key;
	uint8 debounce_flag = 0;   //debounce flag
	sint16 operand_1 = 0, operand_2 = 0, result = 0, last_result;
	uint8 last_operator = 0, operator = 0;
	uint8 first_operator = 0;
	uint8 operand_1_flag = 0, operand_2_flag = 0, operator_flag = 0;
	uint8 result_flag = 0;
	uint8 zero_flag = 0;
	uint8 cons_operators = 0, max_cons_operators = 0;
	
	uint8 operands_arr[ARR_SIZE] = {0};
	uint8 operators_arr[ARR_SIZE] = {0};
	uint8 new_operands_arr[ARR_SIZE] = {0};
	uint8 new_operators_arr[ARR_SIZE] = {0};
	uint8 operand = 0, operand_flag = 0;
	uint8 i = 0, j = 0;
	uint8 priority = 0, max_priority = 0;
	uint8 new_size_operands = ARR_SIZE, new_size_operators = ARR_SIZE;
	
    while (1) 
    {
		/*
		 * do-while loop to read which key is pressed
		 */
		do
		{
			key = keypad_read();
			/*
			 * Check if any of numbers' keys is pressed
	   		 */
			if(debounce_flag == 0 && ((key=='0') | (key=='1') | (key=='2') | (key=='3') | (key=='4') | (key=='5') | (key=='6') | (key=='7') | (key=='8') | (key=='9')))
			{
				/* Check if there is a previous result saved or not
				 * because if there is a result on the screen and I pressed a number
				 * it will clear the screen to start a new operation
				 */
				if(result_flag == 1)
				{
					LCD_send_command(0x01);  //clear screen
					//zeroing the flags to start new operation
					operator_flag = 0;
					operand_flag = 0;
					for(i=0; i<ARR_SIZE; i++)
					{
						operands_arr[i] = 0;
						operators_arr[i] = 0;
					}
					result_flag = 0;
				}
				_delay_ms(5);           //delay for debouncing
				LCD_print_char(key);    //printing the pressed number on keypad
				/*
				 * Checking operator flag which indicates if I have entered operator before or not
				 * because if I didn't entered any operator it means that I am writing the first operand of the operation
				 * and if I entered operator before it means I am writing the 2nd or 3rd, ... operator
			 	 */
				
					/*
					 * equation to add ones, tens, hundreds, ... to each other
					 * for example if want to enter number 123:
					 * first I will enter 1, so operand will equal 1;
					 * second I will enter 2, so operand will equal (1*10)+2 which equal 12
					 * third I will enter 3, so operand will equal (12*10)+3 which equal 123
					 * and so on.
					 */
					operand = (operand*10) + (key - '0');
					operands_arr[operand_flag] = operand; 
					cons_operators = 0;
				
				debounce_flag = 1;
			}
			/*
			 * Check if any of operators' keys is pressed
	   		 */
			else if(debounce_flag == 0 && ((key=='+') | (key=='-') | (key=='*') | (key=='/')))
			{
				if(operator == 0) first_operator = key;
				/* Check if there is a previous result saved or not
				 * because if there is a result on the screen and I pressed an operator
				 * it will assign the result to operand_1 and continue the operation
				 */
				cons_operators++;
				if(cons_operators > max_cons_operators) max_cons_operators = cons_operators;
				if(result_flag == 1)
				{
					LCD_send_command(0x01);  //clear screen
					LCD_goto_xy(1,1);        //back to the starting position
					LCD_print_num(result);   //printing the previous result
					operands_arr[0] = result;
					operand_flag++;
					//operator_flag = 0;      //zeroing operator_flag to enter a new operator for the new operation
					result_flag = 0;
				}
				_delay_ms(5);
				LCD_print_char(key);
				operator = key;
				operators_arr[operator_flag] = operator;
				operand_flag++;
				//operand_flag++;
				operator_flag++;  
				operand = 0;         //incrementing operator flag to indicate that I have entered an operator and to indicate number of entered operators
				/*
				 * If operator_flag > 1, it means that I have added more than one operator to add a new operand like (9+5-3)
				 * so if this case is achieved I will make the operation of the first two operands and assign the result to operand_1
				 * (14-3), then make the normal operation and the result will be (11)
				 */
				
				/* assigning the operator to a variable called last_operator 
				 * to maintain it in case I changed the operator before third operand
				 */
				     
				debounce_flag = 1;
			}
			
			else if(debounce_flag == 1 && key == 0)
			{
				_delay_ms(5);
				debounce_flag = 0;
			}
			key = keypad_read();
		} while(key != '=' && key != 'c');  //stay on the loop receiving data until I press '=' to show the result or press 'c' to clear the screen
		
		/*
		 * Check if 'c' key is pressed to clear the screen
		 * and clear all operands and flags
		 * and be ready for a new operation
		 */
		if(debounce_flag == 0 && key == 'c')
		{
			_delay_ms(5);
			LCD_send_command(0x01);   //clear screen
			operator_flag = 0;
			operand_flag = 0;
			for(i=0; i<ARR_SIZE; i++)
			{
				operands_arr[i] = 0;
				operators_arr[i] = 0;
				new_operands_arr[i] = 0;
				new_operators_arr[i] = 0;
			}
			result_flag = 0;
			result = 0;
			operand = 0;
			debounce_flag = 1;
			max_cons_operators = 0;
		}
		/*
		 * Check if 'c' key is pressed to clear the screen
		 * and clear all operands and flags
		 * and be ready for a new operation
		 */
		else if(debounce_flag == 0 && key == '=')
		{
			/*
			 * if statement to check if I have entered the equation in a wrong format
			 * by checking values of flags which should meet a specific value if the equation format is right
			 */
			if(/*(operators_arr[0] == '*' || operators_arr[0] == '/') || /*(operand_flag == 0 && operator_flag != 0) ||*/ max_cons_operators > 1)
			{
				LCD_send_command(0x01);
				LCD_print_string("Syntax error");
				operator_flag = 0;
				operand_flag = 0;
				for(i=0; i<ARR_SIZE; i++)
				{
					operands_arr[i] = 0;
					operators_arr[i] = 0;
				}
				max_cons_operators = 0;
			}
			else
			{
				_delay_ms(5);
				/*if(operand_flag == 0)
				{
					LCD_goto_xy(2,1);
					result = operands_arr[0];
					LCD_print_num(result);
					result_flag = 1;
				}	
				else
				{*/
					i = 0;
					j = 0;
						for(j=0; j<ARR_SIZE; j++)
						{
							if(operators_arr[i] == '*')
							{
								if(operators_arr[i+1] != 0)
								{
									new_operands_arr[j] = operands_arr[j] * operands_arr[j+1];
									//operands_arr[i+1] = operands_arr[i] * operands_arr[i+1];
									j++;
									new_operators_arr[i] = operators_arr[i+1];
									i++;
								}
								else
								{
									new_operands_arr[j] = operands_arr[j] * operands_arr[j+1];
									break;
								}
							}
							else if(operators_arr[i] == '/')
							{
								new_operands_arr[j] = operands_arr[j] / operands_arr[j+1];
								//operands_arr[i+1] = operands_arr[i] / operands_arr[i+1];
								j++;
								new_operators_arr[i] = operators_arr[i+1];
								i++;
							}
							else if(operators_arr[i] == '+' || operators_arr[i] == '-')
							{
								new_operands_arr[j] = operands_arr[j];
								new_operators_arr[i] = operators_arr[i];
								i++;
								j++;
								if(operators_arr[i+1] == '*')
								{
									new_operands_arr[j] = operands_arr[j] * operands_arr[j+1];
									//operands_arr[i+1] = operands_arr[i] * operands_arr[i+1];
									j++;
									//new_operators_arr[i] = operators_arr[i];
									//i++;
								}
								
								
								
							}
							else if(operators_arr[i] == 0)
							{
								new_operands_arr[j] = operands_arr[j];
								new_operators_arr[i] = operators_arr[i];
								break;
							}
							//i++;
							//j++;
						}
						
						result = new_operands_arr[0];
						for(i=0; i<ARR_SIZE; i++)
						{
							if(new_operators_arr[i] == '+')
							{
								result += new_operands_arr[i+1]; 
							}
							else if(new_operators_arr[i] == '-')
							{
								result -= new_operands_arr[i+1]; 
							}
							/*else if(new_operators_arr[i] == '*')
							{
								result *= new_operands_arr[i+1];
							}
							else if(new_operators_arr[i] == '/')
							{
								result /= new_operands_arr[i+1];
							}*/
							else if(new_operators_arr[i] == 0)
							{
								break;
							}
						}
	
					LCD_goto_xy(2,1);
					if(zero_flag == 1)
					{
						LCD_print_string("Math error");
						operand_1_flag = 0;
						operand_2_flag = 0;
						zero_flag = 0;
					}
					else 
					{
						LCD_print_num(result);
						result_flag = 1;
						//operator = 0;
						operand_1_flag = 0;
						operand_2_flag = 0;
					}
					operator_flag = 0;
				//}
			}
			debounce_flag = 1;
		}
		else if(debounce_flag == 1 && key == 0)
		{
			_delay_ms(5);
			debounce_flag = 0;
		}
    }
}

