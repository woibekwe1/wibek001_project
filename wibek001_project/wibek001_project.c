/*
 * wibek001_project.c
 *
 * Created: 5/18/2013 11:08:24 AM
 *  Author: willibeamin
 */ 

#include <avr/io.h>
#include <bit.h>
#include <timer.h>
#include <scheduler.h>
#include <keypad.h>
#include <tone.h>
#include <lcd_8bit_task.h>
#include <equation.h>
int randomSeeder = 0; 
unsigned char powerUPActivation = 0;
int finalscore = 0; 
unsigned char startgame = 0; 
//*************HAND SHAKING WITH ANSWER AND SCROLLING EQUATIONS
unsigned char sendAnswer = 0; 
unsigned char checkAnswer = 0; 

unsigned char mel = 0; 

//*************HAND SHAKING WITH Scrolling and loosing the game
unsigned char youLose = 0; 

//*************HAND SHAKING WITH  scrolling and powerup
unsigned char pUP = 0;  
unsigned char pDOWN = 0;  
void init_PWM() {
	TCCR2 = (1 << WGM21) | (1 << COM20) | (1 << CS22);
}

void set_PWM(double frequency) {
	unsigned long temp;
	if (frequency < 1)
	OCR2 = 0;
	else
	OCR2 = (int)(16000000 / (128 * frequency)) - 1;
}

enum SpeakMel{  WAITM, SMON, SMOFF } speakMel;
unsigned char melody[165] = { 4, 0, 0, 4, 0, 0, 6, 6, 7, 7, 4, 0, 0, 4, 0, 0, 3, 3, 2, 2, 4, 0, 0, 4, 0, 0, 6, 6, 7, 7, 4, 0, 0, 4, 0, 0, 3, 3, 2, 2, 4, 0, 0, 4, 0, 0, 6, 6, 7, 7, 4, 0, 0, 4, 0, 0, 3, 3, 2, 2, 4, 0, 0, 4, 0, 0, 6, 6, 7, 7, 4, 0, 0, 4, 0, 0, 3, 3, 2, 2,
							  6, 4, 1, 1, 1, 1, 1, 1, 1, 1, 0, 6, 4, 9, 9, 9, 9, 9, 9, 9, 9, 0, 6, 4, 8, 8, 8, 8, 8, 8, 8, 8, 0, 10, 8, 0, 0, 0, 0, 0, 0, 0, 0,    
						     10, 12, 2, 2, 2, 2, 2, 2, 2, 2, 0,  10, 12, 3, 3, 3, 3, 3, 3, 3, 3, 0, 10, 12, 11, 11, 11, 11, 11, 11, 11, 11, 0, 11, 1, 0, 0, 0, 0, 0, 0, 0, 0 }; // };
	
unsigned char smcounter = 0; 
void speakerMelody()
{
	switch( speakMel )
	{
		case -1: 
			speakMel = WAITM;					
			break; 
		case WAITM:
			if( mel )
			{
				speakMel = SMON;
			}
			else
			{
				speakMel = WAITM;
			}
			break; 
		case SMON:
			speakMel = SMOFF; 
			break; 
		case SMOFF: 
			speakMel = SMON; 
			break;
		default: 
			speakMel = WAITM;
			break;  
	}
	
	switch( speakMel )
	{
		case -1:
			break;
		case SMON:
			if(smcounter == 165 )
			{
				smcounter = 0;
			}
			set_PWM( tone( melody[smcounter++] ));
			break;
		case SMOFF:
			break;
		default:
			break;
	}
	
}
unsigned char pvalue = 0;
unsigned char inputTemp;
enum LCDScroll { CLEAR, SHIFT, PRINT, CORRECT, INCORRECT, DEAD} lcdscroll;
E equations; 
unsigned char scrollCount = 0; 
unsigned char counter = 0; 
unsigned char powerUpCounter = 0; 
unsigned char answer = 4; 
unsigned char correctAnswer = 0;  
unsigned char tmpA = 0x04; 
unsigned char tmpB = 0xFF;
unsigned char lives = 4; 
unsigned char operator = 0; 
unsigned char difficulty1 = 1; 
unsigned char difficulty2 = 10; 
unsigned char number[6];
unsigned char levelOfDifficulty = 0; 
unsigned int left = 0;
unsigned int right = 0;
void scroller()
{
	switch(lcdscroll)
	{
		case -1:
			lcdscroll = CLEAR; 
			break;
		case CLEAR: 
			if( startgame == 1 )
			{
				lcdscroll = SHIFT;
			}
			else 
			{
				lcdscroll = CLEAR;
			}
			break; 
		case SHIFT:
			if( lives == 0 )
			{
				lcdscroll = DEAD;
				youLose = 1; 
			} 
			else if ( LCD_string_g[0] == '=' || LCD_string_g[0] == '.' )
			{
				lcdscroll = INCORRECT; 
			}
			else if ( ( GetBit(PINA, 4) && GetBit(PORTA, 2) ) )
			{
				pUP = 0;
				powerUpCounter++;
				tmpB = 0xFF;
				powerUPActivation = 0;
				tmpA = SetBit(tmpA, 2, 0);
				PINA = SetBit(PINA, 4, 0);
				finalscore += 5;
				levelOfDifficulty++;
				if( levelOfDifficulty == 7 )
				{
					difficulty2++;
				}
				else if ( levelOfDifficulty == 10 )
				{
					difficulty1++;
					levelOfDifficulty = 0;
				}
				lcdscroll = CORRECT;
			}
			else if ( inputTemp == 0 )  
			{  
				pUP = 0; 
				powerUpCounter++;
				tmpB = tmpB << 1; 
				finalscore += 5;
				levelOfDifficulty++; 
				if( levelOfDifficulty == 7 ) 
				{
					difficulty2++;
				}
				else if ( levelOfDifficulty == 10 ) 
				{
					difficulty1++;
					levelOfDifficulty = 0; 
				}
				lcdscroll = CORRECT; 
			}
			else 
			{
				lcdscroll = PRINT;
			}
			break; 
		case INCORRECT:
			lcdscroll = CLEAR; 
			break; 
		case CORRECT: 
			lcdscroll = CLEAR; 
			break; 
		case PRINT: 
			lcdscroll = CLEAR; 
			break; 
		case DEAD:
			LCD_go_g = 0; 
			break; 
		default: 
			lcdscroll = -1; 
			break; 
	}
	
	switch(lcdscroll)
	{
		case -1:
			break;
		case CLEAR:
			LCD_go_g = 0; 
			break;
		case SHIFT:
			if (scrollCount > 10)
			{
				LCD_char_pos = 0;					
				strcpy( LCD_string_g,  strcat( LCD_string_g, "                                                  ") + 1 );
				counter++; 
				scrollCount = 0; 
			}	 
			scrollCount++;
			break;
		case INCORRECT: 
			lives--;
			tmpA = tmpA & 0xF0 | lives;
			tmpB = 0xFF;
			left = rand() % difficulty2;
			right = rand() % difficulty2;
			operator = rand() % difficulty1; 
			if( operator == 0 || operator == 1 ) 
			{
				answer = left + right;
			}
			else if ( operator == 2 ) 
			{
				if ( right > left  ) 
				{
					unsigned char temp = left; 
					left = right;
					right = temp;  
				}
				answer = left - right;
			}	
			else if ( operator == 3 ) 
			{
				answer = left * right;
			}
			else if ( operator  == 4 )
			{
				answer = left % right;
			}
			
			strcpy( LCD_string_g, "               " );
			itoa( left, number, 10 ); 
			strcat( LCD_string_g, number );
			if ( operator == 1 || operator == 0  ) 
			{
				strcat( LCD_string_g, " + ");
			}
			else if (operator == 2 ) 
			{
				strcat( LCD_string_g, " - ");
			}
			else if ( operator == 3 )
			{
				strcat( LCD_string_g, " * ");
			}	
			else 
			{
				strcat( LCD_string_g, " % ");
			}	
			itoa( right, number, 10 ); 		
			strcat( LCD_string_g, number );
			strcat( LCD_string_g, "  =                                                  ");
			break; 
		case CORRECT: 
			if( powerUpCounter == 7 ) 
			{
				powerUpCounter = 0; 
				pUP = 1; 
			}
			finalscore += 5; 
			left = rand() % difficulty2;
			right = rand() % difficulty2;
			operator = rand() % difficulty1;
			if( operator == 0 || operator == 1 )
			{
				answer = left + right;
			}
			else if ( operator == 2 )
			{
				if ( right > left  )
				{
					unsigned char temp = left;
					left = right;
					right = temp;
				}
				answer = left - right;
			}
			else if ( operator == 3 )
			{
				answer = left * right;
			}
			else if ( operator  == 4 )
			{
				answer = left % right;
			}
			
			strcpy( LCD_string_g, "               " );
			itoa( left, number, 10 );
			strcat( LCD_string_g, number );
			if ( operator == 1 || operator == 0  )
			{
				strcat( LCD_string_g, " + ");
			}
			else if (operator == 2 )
			{
				strcat( LCD_string_g, " - ");
			}
			else if ( operator == 3 )
			{
				strcat( LCD_string_g, " * ");
			}
			else
			{
				strcat( LCD_string_g, " % ");
			}
			itoa( right, number, 10 );
			strcat( LCD_string_g, number );
			strcat( LCD_string_g, "  =                                                  ");
			break; 
		case PRINT: 
			LCD_go_g = 1; 
			break;
		case DEAD: 
			break; 
		default:
			break;
	}
	PORTA = tmpA; 
	PORTB = tmpB; 
}

enum States { UNHOLD, CAPTURE, HOLD } state; 
unsigned char tp = 0; 
unsigned char convertedAnswer[10] = {"                                "};
unsigned char* ansArray;
unsigned char answerCounter = 0;
void keypadtest() 
{
	pvalue = GetKeypadKey(); 
	switch ( state )
	{
		case -1: 
			state = UNHOLD;
			break; 
		case UNHOLD: 
			if( pvalue != '\0' )
			{ 
				state = CAPTURE;
			}
			else
			{
				state = UNHOLD;
			}
			break;
		case CAPTURE: 
			state = HOLD; 
			break; 
		case HOLD: 
			 if( pvalue == '\0' )
			 {
				 state = UNHOLD;
			 }
			 else
			 {
				 state = HOLD;
			 }
			break; 
		default: 
			state = UNHOLD;
			break; 
	}
	
	switch ( state )
	{
		case -1: 
			break; 
		case UNHOLD: 
			break; 
		case CAPTURE:
			if( pvalue == '*' || pvalue == '#' )
			{
				convertedAnswer[answerCounter++] = '\0';
				answerCounter = 0;
				inputTemp = answer - atoi(convertedAnswer); 
			}	
			else
			{
				convertedAnswer[answerCounter++] = pvalue;
			}			
			break; 
		case HOLD: 
			LCD_go_g = 1; 
			break; 
		default: 
			break; 
	}
}

enum Ans { WAIT,TYPEIN, CONVERT } ans;
unsigned char inputTemp; 
unsigned char* ansArray;  

void equationAnswer() 
{
	inputTemp =  GetKeypadKey();  
	switch( ans ) 
	{
		case -1: 
			ans = WAIT;
			break;
		case WAIT: 
			if(startgame)
			{
				ans = TYPEIN;
			}
			else
			{
				ans = WAIT;
			}
			break;  
		case TYPEIN: 
			if( (answerCounter > 9  ||  inputTemp == '\0') )
			{
				ans = CONVERT;
			}
			else 
			{
				ans = TYPEIN;
			}
			break; 
		case CONVERT: 
			ans = TYPEIN; 
			break; 
		default: 
			ans = -1; 
			break; 
	}
	switch( ans )
	{
		case -1:
			break;
		case TYPEIN:
			if ( inputTemp != '\0' && inputTemp != '*' && inputTemp != '#' )
			{
				answerCounter++;
				strcat(ansArray, inputTemp );
			}			
			break;
		case CONVERT:
			answer = 0; //answer - atoi( ansArray ); 
			strcpy(ansArray, ""); 
			sendAnswer = 1; 
			counter = 0; 
			break;
		default:
			break;
	}
}
enum powerUpClear { pucheck, pu_ON, puHOLD, pu_OFF } pc;
unsigned char tmpA2 = 0; 
 
void powerUP() 
{
	tmpA2 = PINA;
	powerUPActivation = GetBit(tmpA2, 4);
	switch( pc ) 
	{
		case -1: 
			pc = pucheck; 
			break; 
		case pucheck:
			if( pUP )
			{
				pc = pu_ON; 
			} 
			else 
			{
				pc = pucheck; 
			}
			break; 
		case pu_ON:
			if( powerUPActivation  )
			{
				pc = HOLD; 
			}
			else
			{
				pc = pu_ON; 
			}
			break;
		case puHOLD:
			if( !powerUPActivation  )
			{
				pc = pu_OFF;
			}
			else
			{
				pc = HOLD;
			}
			break;
		case pu_OFF:
			inputTemp = 0;
			pc = pucheck;
			break;  
		default: 
			pc = pucheck;
			break; 
	}
	
	switch( pc )
	{
		case -1:
			break;
		case pucheck:
			break;
		case pu_ON: 
			tmpA = SetBit(tmpA, 2, 1); 
			break;
		case puHOLD:
			break;
		case pu_OFF:
			
			tmpA = SetBit(tmpA, 2, 0); 
			pUP = 0; 
			break;
		default:
			break;
	}	
	PORTA = tmpA; 
}

enum L {  WINNING, LOSE, SCORE } lose;
void youLost() 
{
	switch(lose)
	{
		case -1: 
			break;
		case WINNING:
			if( youLose )
			{
				lose = LOSE;
				LCD_go_g = 1; 
			}
			else
			{
				lose = WINNING;
			}
			break; 
		case LOSE:
			lose = SCORE;
			break; 
		case SCORE:
			lose = WINNING;
			break; 
		default: 
			break; 
	}
	
	switch(lose)
	{
		case -1:
			break;
		case WINNING:
			break;
		case LOSE:
			strcpy(LCD_string_g, "   GAME OVER          ");
			break;
		case SCORE:
			LCD_go_g = 0; 
			LCD_go_g = 1; 
			strcpy( LCD_string_g, "YOUR SCORE: ");
			unsigned char temper[10]; 
			itoa(finalscore, temper, 10 );
			strcat( LCD_string_g, temper );
			strcat( LCD_string_g, "       " );
			break;
		default:
			break;
	}
		
}


enum M { Welcome, Menu, GAME, INSTRUCTIONS} menu;
unsigned char menuKey = 0; 
void mainMenu()
{
	menuKey = GetKeypadKey(); 
	switch( menu )
	{
		case -1: 
			menu = Welcome;
			break; 
		case Welcome:
			menu = Menu;
			break;
		case Menu: 
			if( menuKey == '*')
			{
				srand(randomSeeder);
				startgame = 1;
				LCD_go_g = 1;
				menu = GAME;
			}
			else if( menuKey == '2')
			{
				menu = INSTRUCTIONS;
			}
			else
			{
				menu = Menu;
			}
			break;
		case GAME: 
			break;
		case INSTRUCTIONS: 
			break; 
		default:
			break; 
	}
	switch( menu )
	{
		case -1:
			break;
		case Welcome:
			break; 
		case Menu:
			LCD_go_g = 1;
			strcpy( LCD_string_g, "Welcome! Press * ....Ready Begin                       " );
			//mel  1;
			break;
		case INSTRUCTIONS:
			break;
		case GAME:
			startgame = 1; 
			break; 
		default:
			break;
	}
}

int main(void)
{
	init_PWM();
	//while( GetKeypadKey() == '\0' ); 
	//equations = load();
	//strcpy( LCD_string_g,  "               2 + 2 =                                           " );
	//strcpy( LCD_string_g, equations[1].e ) ; 
	//LCD_write_str = 0;
	DDRB = 0xFF; PORTB = 0xFF;// Set port B to output
	DDRC = 0xFF; // Set port D to output
	DDRA = 0xE7; PORTA = 0x18;
	DDRD = 0xF0; PORTD = 0x0F;
	
	// Period for the tasks
	unsigned long int SMTick1_calc = 100;
	unsigned long int SMTick2_calc = 1;
	unsigned long int SMTick3_calc = 5;
	unsigned long int SMTick4_calc = 10;
	unsigned long int SMTick5_calc = 10;
	unsigned long int SMTick6_calc = 1000;
	unsigned long int SMTick7_calc = 10;
	unsigned long int SMTick8_calc = 10;
	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
	tmpGCD = findGCD(tmpGCD, SMTick3_calc);
	tmpGCD = findGCD(tmpGCD, SMTick4_calc);
	tmpGCD = findGCD(tmpGCD, SMTick5_calc);
	tmpGCD = findGCD(tmpGCD, SMTick6_calc);
	tmpGCD = findGCD(tmpGCD, SMTick7_calc);
	tmpGCD = findGCD(tmpGCD, SMTick8_calc);
	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;

	//Recalculate GCD periods for scheduler
	unsigned long int SMTick1_period = SMTick1_calc/GCD;
	unsigned long int SMTick2_period = SMTick2_calc/GCD;
	unsigned long int SMTick3_period = SMTick3_calc/GCD;
	unsigned long int SMTick4_period = SMTick4_calc/GCD;
	unsigned long int SMTick5_period = SMTick5_calc/GCD;
	unsigned long int SMTick6_period = SMTick6_calc/GCD;
	unsigned long int SMTick7_period = SMTick7_calc/GCD;
	unsigned long int SMTick8_period = SMTick8_calc/GCD;
	//Declare an array of tasks
	static task task1, task2, task3, task4, task5, task6, task7, task8;
	task *tasks[] = { &task1, &task2, &task3, &task4, &task5, &task6, &task7, &task8 };
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	// Task 1
	task1.state = -1;//Task initial state.
	task1.period = SMTick1_period;//Task Period.
	task1.elapsedTime = SMTick1_period;//Task current elapsed time.
	task1.TickFct = &speakerMelody;//Function pointer for the tick.

	// Task 2
	task2.state = -1;//Task initial state.
	task2.period = SMTick2_period;//Task Period.
	task2.elapsedTime = SMTick2_period;//Task current elapsed time.
	task2.TickFct = &LCDI_SMTick;//Function pointer for the tick.

	// Task 3
	task3.state = -1;//Task initial state.
	task3.period = SMTick3_period;//Task Period.
	task3.elapsedTime = SMTick3_period; // Task current elasped time.
	task3.TickFct = &keypadtest; // Function pointer for the tick.

	// Task 4
	task4.state = -1;//Task initial state.
	task4.period = SMTick4_period;//Task Period.
	task4.elapsedTime = SMTick4_period; // Task current elasped time.
	task4.TickFct = &scroller; // Function pointer for the tick.
	
	// Task 5
	task5.state = -1;//Task initial state.
	task5.period = SMTick5_period;//Task Period.
	task5.elapsedTime = SMTick5_period; // Task current elasped time.
	task5.TickFct = &powerUP; // Function pointer for the tick.
	
	// Task 6
	task6.state = -1;//Task initial state.
	task6.period = SMTick6_period;//Task Period.
	task6.elapsedTime = SMTick6_period; // Task current elasped time.
	task6.TickFct = &youLost; // Function pointer for the tick.
	
	// Task 7
	task7.state = -1;//Task initial state.
	task7.period = SMTick7_period;//Task Period.
	task7.elapsedTime = SMTick7_period; // Task current elasped time.
	task7.TickFct = &mainMenu; // Function pointer for the tick.
	
	// Task 8
	task8.state = -1;//Task initial state.
	task8.period = SMTick8_period;//Task Period.
	task8.elapsedTime = SMTick8_period; // Task current elasped time.
	task8.TickFct = &powerUP; // Function pointer for the tick.
	
	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();
	
	// Scheduler for-loop iterator
	while(1)
	{
		randomSeeder++;
		// Scheduler code
		for ( unsigned short i = 0; i < numTasks; i++ )
		{
			randomSeeder++;
			// Task is ready to tick
			if ( tasks[i]->elapsedTime == tasks[i]->period )
			{
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		randomSeeder++;
		TimerFlag = 0;
	}
}

