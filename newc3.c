#include <stdio.h>
#include <stdlib.h>

float Plus    (float a, float b) { return a+b; }
float Minus   (float a, float b) { return a-b; }
float Multiply(float a, float b) { return a*b; }
float Divide  (float a, float b) { return a/b; }

void Switch(float a, float b, char opCode)
{
   float result;
   switch(opCode)
   {
      case '+' : result = Plus     (a, b); break;
      case '-' : result = Minus    (a, b); break;
      case '*' : result = Multiply (a, b); break;
      case '/' : result = Divide   (a, b); break;
   }

   printf("Switch: 2+5= %f\n",result);         // display result
}

void Switch_With_Function_Pointer(float a, float b, float (*pt2Func)(float a, float b))
{
   float result = pt2Func(a, b);    // call using function pointer

   printf("Switch replaced by function pointer: 2+5= %f\n",result);  // display result
}

void Replace_A_Switch()
{
   printf("Executing function 'Replace_A_Switch'\n");

   Switch(2, 5, /* '+' specifies function 'Plus' to be executed */ '+');
   Switch_With_Function_Pointer(2, 5, /* pointer to function 'Minus' */ &Plus);
}

int main(){
	Replace_A_Switch();
	return 0;
}
