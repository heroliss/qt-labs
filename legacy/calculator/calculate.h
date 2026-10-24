//calculate.h
#ifndef CALCULATE_H
#define CALCULATE_H
#include"QDebug"

#include"math.h"
#include"stdlib.h"
#include "stack.h"
#include"ctype.h"
#define MAX 308           //允许输入的单个最大数值的字节数
using namespace std; 
/*************************自写数学函数*********************************/
//long double E(long double a,long double b)
//{
//	return a*pow(10,b);
	
//}
/**************************************************************/
bool judge_behind(QString line,int i)
{
	i++;
    while (i<line.length()&&line[i]==' ')
	{
		i++;
	}
	
    if(i>=line.length())
	{
		return true;
	} else
	{
        if (line[i].isDigit() )
		{
			return false;
		}
		return true;
	}
}
float in(QString c)        //读入运算符的优先级  //in<out则符号入栈  in=out则符合出栈 in>out则计算
{ 
    if(c=="-" || c=="+") return 2;
    if(c=="*" || c=="/"|| c=="%"||c=="atanb") return 4;
    if(c=="^"||c=="E") return 5;
    if(c=="(") return 0;
    if(c==")") return 7;
	
    if (c=="[") return 0;    //完全入栈
    if (c=="]")	return 7;
	
    if (c=="s"||c=="c"||c=="t"||c=="n"                  //所以单操作数运算的优先级最高且一致,其次是次方运算，乘除运算和加减运算
        ||c=="x"||c=="y"||c=="z"||c=="g"||c=="q"
        ||c=="e"||c=="r"||c=="p"||c=="i"||c=="o"||c=="a"
        ||c=="~"||c=="`"||c=="_"||c=="√") return 6.2;
	return -1; 
} 

float out(QString c)       //弹出运算符的优先级
{ 
    if(c=="-" || c=="+") return 1;
    if(c=="*" || c=="/" || c=="%"||c=="atanb") return 3;
    if(c=="^"||c=="E") return 6;
    if(c=="(") return 7;
    if(c==")") return 0;
	
    if (c=="[")	return 7;
    if (c=="]")	return 0;
	
    if (c=="s"||c=="c"||c=="t"||c=="n"
        ||c=="x"||c=="y"||c=="z"||c=="g"||c=="q"
        ||c=="e"||c=="r"||c=="p"||c=="i"||c=="o"||c=="a"
        ||c=="~"||c=="`"||c=="_"||c=="√") return 6.3;
	return -1; 
} 
/********************************************************/
long double count(long double a,QString ope,long double b,int &status)//进行计算返回结果
{ 
	long double sum; 
//	if(ope=="E") sum=E(a,b);
    if(ope=="+") sum=a+b;
    if(ope=="-") sum=a-b;
    if(ope=="*") sum=a*b;
    if(ope=="/")
	{
		if(b==0)
		{
            status=106;//当前最大
			return 0;
		}else
			sum=a/b;
	} 
    if(ope=="^") sum=pow(a,b);
    if(ope=="%") sum=fmod(a,b);
	if (ope=="atanb") sum=atan2(b,a);
	
	return sum;
} 
long double count_single(QString ope,long double a,int &status) //单操作数的运算
{
    if (ope=="x") return sin(a);
    if (ope=="y") return cos(a);
    if (ope=="z") return tan(a);
    if (ope=="s")
	{	
		if(a>1||a<-1)
		{
            status=101;       //当传出一个大于100的status值时则代表计算数据有误
			return 0;
		}
		else	return asin(a);
	}
    if (ope=="c")
	{ 
		if(a>1||a<-1)
		{
            status=102;
			return 0;
		}
		else return acos(a);
	}
    if (ope=="t") return atan(a);
    if (ope=="i") return sinh(a);
    if (ope=="o") return cosh(a);
    if (ope=="a") return tanh(a);
    if (ope=="n")
	{
		if (a<=0)
		{
            status=103;
			return 0;
		}else
			return log(a);
	}
    if (ope=="g")
	{
		if (a<=0)
		{
            status=104;
			return 0;
		}else
			return log10(a);
	}
    if (ope=="q"||ope=="√")
	{
		if (a<=0)
		{
            status=105;
			return 0;
		}else
			return sqrt(a);
	}
    if (ope=="e"||ope=="`") return ceil(a);
    if (ope=="r"||ope=="_") return floor(a);
    if (ope=="~") return -a;      //取反运算，同负号
    if (ope=="p") return exp(a);
}
bool calculate(QString line,long double &result,int &status,QString *variable) //变量字符组
{
    int len=line.length(); //输入式子的长度
	int i=0,j,point,mark=0;//i,j为字符序号,point记录小数点个数,mark表示读入的字符是否是字母变量、数字以及运算符。 
    QString operate;
    QString temp;
	long double a,b;//a、b用来存储操作数栈中弹出的操作数，便于代入函数中进行计算。 
	Stack<long double> stack1(0.0);
    Stack<QString>  stack2("(");
    while(i<len&&line[i]==' ')  //跳过空格
	{
		i++;
	}	
    if (!line[i].isDigit() && !line[i].isLetter() && line[i]!='(' && line[i]!='[' && line[i]!='~'
        && line[i]!='_' && line[i]!='√' && line[i]!='-' && line[i]!='`')
	{
        status=1;//开头错误
		return false;
	}	
    while(i<line.length())
	{ 
        len=line.length();
        while(i<len&&line[i]==' ')  //跳过空格
		{
			i++;
		}
		mark=0; //错误标记(默认0为读入错误)
        if (line[i]=='.')
		{
            status=2;//小数点落单
			return false;
		}
		/***************************数字情况*********************************************************/
        if(line[i].isDigit())//若读入的字符为数字，则继续判断下一个字符，直到下一个字符不是数字或者不是小数点，即可保证该操作数是完整的小数，然后将该数入操作数栈。
		{ 
            temp="";
            mark=1;//标记读入正确
			point=0;//记录所存的数中小数点的个数 
            while(line[i].isDigit() || line[i]=='.')
			{ 
                if(line[i]=='.') point++;
                if(point>1) {status=3;return false;} //3号错误，数字错误（多余的小数点）
                temp+=line[i];   //先赋值后j加一
				i++; 
				if(i>=len) break; 
                if (temp.length()>=MAX)
				{
                    status=4;    //1号错误提示：数值太大
					return false;
				}
			} 
            while(i<len&&line[i]==' ')  //跳过空格
			{
				i++;
			}
			
            if( i<len&&(line[i]!='-' && line[i]!='+' && line[i]!='*' && line[i]!='/' && line[i]!='^' && line[i]!=')'
                && line[i]!='√' && line[i]!='%' && !line[i].isLetter() && line[i]!=']') )
			{
                status=5; //5号错误提示：数字后接错误（数字后不能连接`，_,|,(和非法字符 ）
				return false; //所存数中含有不止一个小数点，或者数字后面跟的不是“+、-、*、/、^、)”，则为错误 
			}


            b=temp.toDouble(); //字符串转为浮点数
			
			
			stack1.push(b); 
            if (i<len && (line[i]=='√'|| line[i].isLetter()) )    //若数字后跟的是字母或根号，则压入乘号
			{	
                line.insert(i,'*');
				len++;
			}
			
			
			if(i>=len) break; 
			
			//下面直接至while循环尾部
		} 
		/******************************运算符情况******************************************************************/
		else 
		{ 
            if(line[i]=='-' || line[i]=='+' || line[i]=='*' || line[i]=='/'
                || line[i]=='^' || line[i]=='(' || line[i]==')'
                || line[i]=='[' || line[i]==']' || line[i]=='%' || line[i]=='√'
                || line[i]=='~'|| line[i]=='_'|| line[i]=='`') //若读入的字符为运算符的情况
			{ 
				mark=1; j=i;
                if(line[i]==')' ||line[i]==']')
                {
					if (i!=len-1)
                    {

						i++;
                        while (i<line.length()&&line[i]==' ')
						{
							i++;
						}
						
                        if(i>=line.length()) ;
						else
						{
                            if (line[i].isDigit()||line[i].isLetter())
							{
                                status=25;//右括号后接错误

								return false;
							}
						}
					}

				}
				i=j;
                if(line[i]=='(' )
				{
					
					if(i==len-1)
					{
                        status=6;//末尾括号错误
						return false;// “(”放表达式最后面，错误 
					}else 
					{
						i++;
                        while(i<line.length()&&line[i]==' ')  //跳过空格
						{
							i++;
						}
                        if (!line[i].isDigit()&&line[i]!='-'
                            && line[i]!='(' && !line[i].isLetter() &&line[i]!='['&&line[i]!='~'&&line[i]!='`'&&line[i]!='_'&&line[i]!='√')
						{
                            status=7;//左括号后接错误
							return false;
						}
					}
				}
				i=j;
                if(line[i]=='-' || line[i]=='+' || line[i]=='*' || line[i]=='/' || line[i]=='^')
				{ 
					if(i==len-1) 
					{
                        status=8;//末尾多余符号错误
						return false;//“+、-、*、/、^”放在表达式最后面，错误            //下面待更换变量判断
					}
					i++;
                    while(i<len-1&&line[i]==' ')  //跳过空格
					{
						i++;
					}
                    if( (!line[i].isDigit()) && (!line[i].isLetter()) && line[i]!='(' && line[i]!='-' && line[i]!='~'
                        && line[i]!='√'&& line[i]!='['&& line[i]!='`'&& line[i]!='_')//“+、-、*、/、^”后面跟的不是数字或者变量或'('，错误
					{
                        status=9;//运算符后接错误
						return false;
					}
				} 
				i=j;
                if (line[i]=='[')   //已有j=i
				{
					
					if (i==len-1)
					{
                        status=10; //末尾错误，落单的绝对值左竖线
						return false;
					}
					
					i++;
					
                    while(i<len&&line[i]==' ')  //跳过空格
					{
						i++;
					}
					
                    if (!line[i].isDigit()&&line[i]!='-'
                        && line[i]!='(' && !line[i].isLetter() &&line[i]!='['&&line[i]!='~'&&line[i]!='`'&&line[i]!='`'&&line[i]!='_'&&line[i]!='√')
					{
                        status=11;//绝对值左竖线后接错误
						return false;
					}
					
				}
				i=j;
				i--;
                while(i>=0&&line[i]==' ')  //跳过空格
				{
					i--;
				}
                if(line[j]=='-' && (i<0 || (!line[i].isDigit()&&line[i]!='.'
                                    &&line[i]!=')'&&line[i]!=']') )) //运算符是负号 进行取反运算
                {
                    line.replace(j,1,'~');
					
                }
				
				
				{ //读入的运算符与运算符栈的栈顶元素相比，并进行相应的操作 
					i=j;
                    if(in(stack2.show_top())<out((QString)line[i])||stack2.is_empty())  //读入运算符的优先级小于上个运算符的优先级，则入栈
                    {
                        stack2.push((QString)line[i]);
						i++;
					} 
					else 
                        if(in(stack2.show_top())==out((QString)line[i]))   //优先级相等，则弹出上个运算符
						{
//                            if(stack2.is_empty())
//                            {
//                                status=333;
//                                return false;
//                            }
							operate=stack2.pop();


                            if ((line[i]==']'&&operate!="[")||(line[i]==')'&&operate!="("))
							{
                                status=12;  //发现绝对值竖线与括号的错误搭配
								return false;
							}
                            if(operate=="[")
							{
								stack1.push( fabs(stack1.pop()) );
							}
							i++;
						} 
						else            //读入运算符的优先级大于上个运算符的优先级
                            if(in(stack2.show_top())>out((QString)line[i]))
							{ 
								operate=stack2.pop();  //弹出俩个数据和一个运算符
                                if (operate[0].isLetter() || operate=="√" || operate=="~"||operate=="`" || operate=="_") //单操作数运算
								{
									a=stack1.pop();
                                    stack1.push( count_single(operate,a,status));
                                    if (status>=100)return false;
									
								}
								
								else
								{
									a=stack1.pop(); 
									b=stack1.pop(); 
									
                                    if ( (operate=="/"||operate=="%") &&a==0)
									{
                                        status=13;
										return false; //除数为0时跳出
									}
                                    stack1.push(count(b,operate,a,status)); //计算出结果,并压入数据栈
                                    if (status>=100)return false;
								}
								
								
							} 
							
							if(i>=len) break; 
				} 
		} 
		else 
			/******************************读入的字符是字母变量的情况**********************************/
		{ 
            if(line[i].isLetter())
			{ 
				
				/*取消变量功能*//*换之为三角函数及宏常量功能*/
				
                mark=1,j=0; QString s="";
				int judge=0,t=i;    //j为第一此循环后i地址的暂存，t为循环前i地址的暂存
				/**************************************atanb****************************************************************/
//                while(i<line.length()&&line[i].isLetter())
//				{
//					s+=line[i];            //s存当前字符串

// /*->*/				if (s=="atanb")              //对atanb这种用字母表示的双操作数运算符进行特殊的排错处理
//					{
//						int tt=i;
//						i=t;//返回原位置
//						i--;
//                        while(i>=0&&line[i]==' ')  //向前跳过空格
//						{
//							i--;
//						}
//						if (i<=0)
//						{
//							status=23;//atanb在算式开头错误
//							return false;
//						}
						
//                        if( (!line[i].isDigit())  && line[i]!=')' && line[i]!=']')//前面只允许是数字或右括号  //????????
//						{
//							status=22;//atanb运算符前接错误
//							return false;
//						}
//						i=tt;
//						if(i>=len-1)
//						{
//							status=20;//末尾多余atanb错误
//							return false;//“+、-、*、/、^”放在表达式最后面，错误            //下面待更换变量判断
//						}
//						i++;
//                        while(i<len&&line[i]==' ')  //跳过空格
//						{
//							i++;
//						}
//                        if( (!line[i].isDigit())  && line[i]!='(' && line[i]!='-' && line[i]!='~'
//                            && line[i]!='√'&& line[i]!='['&& line[i]!='`'&& line[i]!='_')//不允许跟函数，后面跟的不是数字或者变量或'("，错误
//						{
//							status=21;//atanb运算符后接错误
//							return false;
//						}
//						judge=1;
						
//						stack2.push(s);     //若验证为函数的字符串 则压入栈
//						break;
//					}
//					i++;
//				}
//				/************************************atanb********************************************/
//                if(judge==0)
//                {
//                    j=i;
//                    i=t;
//                    s="";
//                }//若第一次循环没有找到函数则重置i
                while(judge==0&&i<line.length()&&line[i].isLetter())
				{	
					s+=line[i];            //s存当前字符串
					
					if (s=="ln"||s=="lg"||s=="ceil"||s=="floor"||s=="exp"||s=="sqrt"
						||s=="asin"||s=="acos"||s=="atan"||s=="sinh"||s=="cosh"||s=="tanh")
					{
						judge=1;
                        if (s.length()==4)
						{
                            stack2.push((QString)s[s.length()-3]);     //若验证为函数的字符串长度为4，则将倒数第3个字符压入栈
						}
						
						else
						{
                            stack2.push((QString)s[s.length()-1]);     //若验证为函数的字符串长度不为4，则将倒数第1个字符压入栈
						}
						break;
					}
					i++;
				}
				
				int isv=0;    //至此从上面传下来的字符串都不是变量
                if(judge==0){j=i;i=t;s="";}//若上一次循环没有找到函数则重置i
                    QString pi="3.1415926535897932384626433832795";
                    QString e="2.7182818284590455";
                    QString C="2.997924584E8";
                    QString NA="6.02214179E23";
                    QString DGOLD="0.618033988749895";
                    QString G="6.67428E-11";
                    QString E="10^";
                while(judge==0&&i<line.length()&&line[i].isLetter())
				{
					isv=1;  //假定下面会读取出变量
					s+=line[i];
					if (s=="M")  //数值替换并且禁止为空
					{
						if (variable[0]=="")
						{
                            status=27;               //M为空错误
							return false;
						}
						if(false==judge_behind(line,i))
						{
                            status=26;               //M后接错误
							return false;
						}

						judge=1;
                        line.remove(i,1);
                        line.insert(i,variable[0]);
						break;
					}	
					if (s=="X"||s=="x")
					{
						judge=1;
                        line.remove(i,1);
                        line.insert(i,variable[1]);
						break;
					}	
					if (s=="Y"||s=="y")
					{
						judge=1;
                        line.remove(i,1);
                        line.insert(i,variable[2]);
						break;
					}					
					if (s=="Z"||s=="z")
					{
						judge=1;
                        line.remove(i,1);
                        line.insert(i,variable[3]);
						break;
					}	
					if (s=="E")
					{
						judge=1;
                        line.remove(i,1);
                        line.insert(i,E);
						break;
					}	
					/*********************下面是数值替换，上面是宏替换************************/
					if (s=="C")
					{
						if(false==judge_behind(line,i))
						{
                            status=24;
							return false;
						}
						judge=1;
                        line.remove(i,1);
                        line.insert(i,C);
						break;
					}				
					if (s=="PI"||s=="pi")
					{
						if(false==judge_behind(line,i))
						{
                            status=24;
							return false;
						}
						judge=1;
						i--;
                        line.remove(i,2);
                        line.insert(i,pi);
						break;
					}
					if (s=="e")
					{
						if(false==judge_behind(line,i))
						{
                            status=24;
							return false;
						}						judge=1;
                        line.remove(i,1);
                        line.insert(i,e);
						break;
					}

					if (s=="NA")
					{
						if(false==judge_behind(line,i))
						{
                            status=24;
							return false;
						}						judge=1;
						i--;
                        line.remove(i,2);
                        line.insert(i,NA);
						break;
					}					
				
					if (s=="DGOLD")
					{
						if(false==judge_behind(line,i))
						{
                            status=24;
							return false;
						}						judge=1;
						i--;
                        line.remove(i,2);
                        line.insert(i,DGOLD);
						break;
					}					
					if (s=="G")
					{
						if(false==judge_behind(line,i))
						{
                            status=24;
							return false;
						}						judge=1;
                        line.remove(i,1);
                        line.insert(i,G);
						break;
					}
					
					
					if (s=="sin")           //三角函数代表字母另取
					{
						isv=0;       //修改假定的“是变量”
						judge=1;
                        stack2.push("x");
						break;
					}else if (s=="cos")
					{
						isv=0;
						judge=1;
                        stack2.push("y");
						break;
					}
					else if (s=="tan")
					{
						isv=0;
						judge=1;
                        stack2.push("z");
						break;
					}	
					
					i++;
				}
				if (judge==0)
				{
                    status=14;   //函数名错误
					return false;
				}

				if(isv==0)
				{

						i++;
                    while (i<len&&line[i]==' ')
					{
						i++;
					}

					if(i>=len) 
					{
                        status=15;       //末尾函数后无数据错误
						return false;
					} 
					/******************************/
					
					
                    if(line[i]!='-' && !line[i].isDigit() && line[i]!='(' && line[i]!='['&& line[i]!='√'
                        && line[i]!='~'&& line[i]!='`'&& line[i]!='_' && !line[i].isLetter() )//函数后面跟的不是-，(或数字则为错误
					{
                        status=16;//函数后接错误
						return false;
					}
				}
			} 
		} 
	} 
	
	if(mark==0) 
	{
        status=17;      //存在非法字符
		return false; //mark=0表示该字符是不符合要求的字符
	}
} 

while(!stack2.is_empty())//读入结束后，继续进行操作，直到运算符栈为空 
{ 
	operate=stack2.pop();  //弹出俩个数据和一个运算符
    if (operate[0].isLetter() || operate=="√" || operate=="~"|| operate=="`" || operate=="_") //单操作数运算
	{
		a=stack1.pop();
        stack1.push( count_single(operate,a,status));
        if (status>=100)return false;
	}
	
	else
	{
		a=stack1.pop(); 
		b=stack1.pop(); 
        if(operate=="(" || operate==")" || operate=="[" || operate=="]") //括号多余的情况
		{
            status=18;//存在多余括号
			return false;
		}
        if ( (operate=="/"||operate=="%") &&a==0)
		{
            status=19;
			return false; //除数为0时跳出
		}
        stack1.push(count(b,operate,a,status)); //计算出结果,并压入数据栈
        if (status>=100)return false;
	}
	
	
}

result=stack1.pop();
return true; 
}


#endif //CALCULATE_H


