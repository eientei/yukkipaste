#ifndef __YU_MACRO__
#define __YU_MACRO__

/* converts #defined literal into string literal in two pre-processor passes */
#define STR(A) STR_(A)
#define STR_(A) #A

#endif /* __YU_MACRO__ */
