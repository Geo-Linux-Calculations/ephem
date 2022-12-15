/* module to compile and execute a c-style arithmetic expression.
 * public entry points are compile_expr() and execute_expr().
 *
 * one reason this is so nice and tight is that all opcodes are the same size
 * (an int) and the tokens the parser returns are directly usable as opcodes,
 * for the most part. constants and variables are compiled as an opcode
 * with an offset into the auxiliary opcode tape, opx.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#ifdef VMS
#include <stdlib.h>
#endif
#include "screen.h"
#include "ephem.h"

static void skip_double();
static int compile (int prec);
static int execute(double *result);
static int parse_fieldname ();

/* parser tokens and opcodes, as necessary */
#define	HALT	0	/* good value for HALT since program is inited to 0 */
/* binary operators (precedences in table, below) */
#define	ADD	1
#define	SUB	2
#define	MULT	3
#define	DIV	4
#define	AND	5
#define	OR	6
#define	GT	7
#define	GE	8
#define	EQ	9
#define	NE	10
#define	LT	11
#define	LE	12
/* unary op, precedence in NEG_PREC #define, below */
#define	NEG	13
/* symantically operands, ie, constants, variables and all functions */
#define	CONST	14	
#define	VAR	15
#define	ABS	16	/* add functions if desired just like this is done */
#define	SQRT	17	/* add functions if desired just like this is done */
/* purely tokens - never get compiled as such */
#define	LPAREN	255
#define	RPAREN	254
#define	ERR	(-1)

/* precedence of each of the binary operators.
 * in case of a tie, compiler associates left-to-right.
 * N.B. each entry's index must correspond to its #define!
 */
static int precedence[] = {0,5,5,6,6,2,1,4,4,3,3,4,4};
#define	NEG_PREC	7	/* negation is highest */

/* execute-time operand stack */
#define	MAX_STACK	16
static double stack[MAX_STACK], *sp;

/* space for compiled opcodes - the "program".
 * opcodes go in lower 8 bits.
 * when an opcode has an operand (as CONST and VAR) it is really in opx[] and
 *   the index is in the remaining upper bits.
 */
#define	MAX_PROG 32
static int program[MAX_PROG], *pc;
#define	OP_SHIFT	8
#define	OP_MASK		0xff

/* auxiliary operand info.
 * the operands (all but lower 8 bits) of CONST and VAR are really indeces
 * into this array. thus, no point in making this any longer than you have
 * bits more than 8 in your machine's int to index into it, ie, make
 *    MAX_OPX <= 1 << ((sizeof(int)-1)*8)
 * also, the fld's must refer to ones being flog'd, so not point in more
 * of these then that might be used for plotting and srching combined.
 */
#define	MAX_OPX	16
typedef union {
    double opu_f;		/* value when opcode is CONST */
    int opu_fld;		/* rcfpack() of field when opcode is VAR */
} OpX;
static OpX opx[MAX_OPX];
static int opxidx;

/* these are global just for easy/rapid access */
static int parens_nest;	/* to check that parens end up nested */
static char *err_msg;	/* caller provides storage; we point at it with this */
static char *cexpr, *lcexpr; /* pointers that move along caller's expression */
static int good_prog;	/* != 0 when program appears to be good */

/* compile the given c-style expression.
 * return 0 and set good_prog if ok,
 * else return -1 and a reason message in errbuf.
 */
int compile_expr (ex, errbuf)
char *ex;
char *errbuf;
{
	int instr;

	/* init the globals.
	 * also delete any flogs used in the previous program.
	 */
	cexpr = ex;
	err_msg = errbuf;
	pc = program;
	opxidx = 0;
	parens_nest = 0;
	do {
	    instr = *pc++;
	    if ((instr & OP_MASK) == VAR)
		flog_delete (opx[instr >> OP_SHIFT].opu_fld);
	} while (instr != HALT);

	pc = program;
	if (compile(0) == ERR) {
	    (void) sprintf (err_msg + strlen(err_msg), " at \"%.10s\"", lcexpr);
	    good_prog = 0;
	    return (-1);
	}
	*pc++ = HALT;
	good_prog = 1;
	return (0);
}

/* execute the expression previously compiled with compile_expr().
 * return 0 with *vp set to the answer if ok, else return -1 with a reason
 * why not message in errbuf.
 */
int execute_expr (vp, errbuf)
double *vp;
char *errbuf;
{
	int s;

	err_msg = errbuf;
	sp = stack + MAX_STACK;	/* grows towards lower addresses */
	pc = program;
	s = execute(vp);
	if (s < 0)
	    good_prog = 0;
	return (s);
}

/* this is a way for the outside world to ask whether there is currently a
 * reasonable program compiled and able to execute.
 */
int prog_isgood()
{
	return (good_prog);
}

/* get and return the opcode corresponding to the next token.
 * leave with lcexpr pointing at the new token, cexpr just after it.
 * also watch for mismatches parens and proper operator/operand alternation.
 */
static
int next_token ()
{
	static const char toomt[] = "More than %d terms";
	static const char badop[] = "Illegal operator";
	int tok = ERR;	/* just something illegal */
	char c;

	while ((c = *cexpr) == ' ')
	    cexpr++;
	lcexpr = cexpr++;

	/* mainly check for a binary operator */
	switch (c) {
	case '\0': --cexpr; tok = HALT; break; /* keep returning HALT */
	case '+': tok = ADD; break; /* compiler knows when it's really unary */
	case '-': tok = SUB; break; /* compiler knows when it's really negate */
	case '*': tok = MULT; break;
	case '/': tok = DIV; break;
	case '(': parens_nest++; tok = LPAREN; break;
	case ')':
	    if (--parens_nest < 0) {
	        (void) sprintf (err_msg, "Too many right parens");
		return (ERR);
	    } else
		tok = RPAREN;
	    break;
	case '|':
	    if (*cexpr == '|') { cexpr++; tok = OR; }
	    else { (void) sprintf (err_msg, badop); return (ERR); }
	    break;
	case '&':
	    if (*cexpr == '&') { cexpr++; tok = AND; }
	    else { (void) sprintf (err_msg, badop); return (ERR); }
	    break;
	case '=':
	    if (*cexpr == '=') { cexpr++; tok = EQ; }
	    else { (void) sprintf (err_msg, badop); return (ERR); }
	    break;
	case '!':
	    if (*cexpr == '=') { cexpr++; tok = NE; }
	    else { (void) sprintf (err_msg, badop); return (ERR); }
	    break;
	case '<':
	    if (*cexpr == '=') { cexpr++; tok = LE; }
	    else tok = LT;
	    break;
	case '>':
	    if (*cexpr == '=') { cexpr++; tok = GE; }
	    else tok = GT;
	    break;
	}

	if (tok != ERR)
	    return (tok);

	/* not op so check for a constant, variable or function */
	if (isdigit(c) || c == '.') {
	    if (opxidx > MAX_OPX) {
		(void) sprintf (err_msg, toomt, MAX_OPX);
		return (ERR);
	    }
	    opx[opxidx].opu_f = atof (lcexpr);
	    tok = CONST | (opxidx++ << OP_SHIFT);
	    skip_double();
	} else if (isalpha(c)) {
	    /* check list of functions */
	    if (strncmp (lcexpr, "abs", 3) == 0) {
		cexpr += 2;
		tok = ABS;
	    } else if (strncmp (lcexpr, "sqrt", 4) == 0) {
		cexpr += 3;
		tok = SQRT;
	    } else {
		/* not a function, so assume it's a variable */
		int fld;
		if (opxidx > MAX_OPX) {
		    (void) sprintf (err_msg, toomt, MAX_OPX);
		    return (ERR);
		}
		fld = parse_fieldname ();
		if (fld < 0) {
		    (void) sprintf (err_msg, "Unknown field");
		    return (ERR);
		} else {
		    if (flog_add (fld) < 0) { /* register with field logger */
			(void) sprintf (err_msg, "Sorry; too many fields");
			return (ERR);
		    }
		    opx[opxidx].opu_fld = fld;
		    tok = VAR | (opxidx++ << OP_SHIFT);
		}
	    }
	}

	return (tok);
}

/* move cexpr on past a double.
 * allow sci notation.
 * no need to worry about a leading '-' or '+' but allow them after an 'e'.
 * TODO: this handles all the desired cases, but also admits a bit too much
 *   such as things like 1eee2...3. geeze; to skip a double right you almost
 *   have to go ahead and crack it!
 */
static
void skip_double()
{
	int sawe = 0;	/* so we can allow '-' or '+' right after an 'e' */

	while (1) {
	    char c = *cexpr;
	    if (isdigit(c) || c=='.' || (sawe && (c=='-' || c=='+'))) {
		sawe = 0;
		cexpr++;
	    } else if (c == 'e') {
		sawe = 1;
		cexpr++;
	    } else
		break;
	}
}

/* call this whenever you want to dig out the next (sub)expression.
 * keep compiling instructions as long as the operators are higher precedence
 * than prec, then return that "look-ahead" token that wasn't (higher prec).
 * if error, fill in a message in err_msg[] and return ERR.
 */
static
int compile (prec)
int prec;
{
	int expect_binop = 0;	/* set after we have seen any operand.
				 * used by SUB so it can tell if it really 
				 * should be taken to be a NEG instead.
				 */
	int tok = next_token ();

        while (1) {
	    int p;
	    if (tok == ERR)
		return (ERR);
	    if (pc - program >= MAX_PROG) {
		(void) sprintf (err_msg, "Program is too long");
		return (ERR);
	    }

	    /* check for special things like functions, constants and parens */
            switch (tok & OP_MASK) {
            case HALT: return (tok);
	    case ADD:
		if (expect_binop)
		    break;	/* procede with binary addition */
		/* just skip a unary positive(?) */
		tok = next_token();
		continue;
	    case SUB:
		if (expect_binop)
		    break;	/* procede with binary subtract */
		tok = compile (NEG_PREC);
		*pc++ = NEG;
		expect_binop = 1;
		continue;
            case ABS: /* other funcs would be handled the same too ... */
	    case SQRT:
		/* eat up the function parenthesized argument */
		if (next_token() != LPAREN || compile (0) != RPAREN) {
		    (void) sprintf (err_msg, "Function arglist error");
		    return (ERR);
		}
		/* then handled same as ... */
            case CONST: /* handled same as... */
	    case VAR:
		*pc++ = tok;
		tok = next_token();
		expect_binop = 1;
		continue;
            case LPAREN:
		if (compile (0) != RPAREN) {
		    (void) sprintf (err_msg, "Unmatched left paren");
		    return (ERR);
		}
		tok = next_token();
		expect_binop = 1;
		continue;
            case RPAREN:
		return (RPAREN);
            }

	    /* everything else is a binary operator */
	    p = precedence[tok];
            if (p > prec) {
                int newtok = compile (p);
		if (newtok == ERR)
		    return (ERR);
                *pc++ = tok;
		expect_binop = 1;
                tok = newtok;
            } else
                return (tok);
        }
}

/* "run" the program[] compiled with compile().
 * if ok, return 0 and the final result,
 * else return -1 with a reason why not message in err_msg.
 */
static
int execute(result)
double *result;
{
	int instr; 

	do {
	    instr = *pc++;
	    switch (instr & OP_MASK) {
	    /* put these in numberic order so hopefully even the dumbest
	     * compiler will choose to use a jump table, not a cascade of ifs.
	     */
	    case HALT: break;	/* outer loop will stop us */
	    case ADD:  sp[1] = sp[1] +  sp[0]; sp++; break;
	    case SUB:  sp[1] = sp[1] -  sp[0]; sp++; break;
	    case MULT: sp[1] = sp[1] *  sp[0]; sp++; break;
	    case DIV:  sp[1] = sp[1] /  sp[0]; sp++; break;
	    case AND:  sp[1] = sp[1] && sp[0] ? 1 : 0; sp++; break;
	    case OR:   sp[1] = sp[1] || sp[0] ? 1 : 0; sp++; break;
	    case GT:   sp[1] = sp[1] >  sp[0] ? 1 : 0; sp++; break;
	    case GE:   sp[1] = sp[1] >= sp[0] ? 1 : 0; sp++; break;
	    case EQ:   sp[1] = sp[1] == sp[0] ? 1 : 0; sp++; break;
	    case NE:   sp[1] = sp[1] != sp[0] ? 1 : 0; sp++; break;
	    case LT:   sp[1] = sp[1] <  sp[0] ? 1 : 0; sp++; break;
	    case LE:   sp[1] = sp[1] <= sp[0] ? 1 : 0; sp++; break;
	    case NEG:  *sp = -*sp; break;
	    case CONST: *--sp = opx[instr >> OP_SHIFT].opu_f; break;
	    case VAR:
		if (flog_get(opx[instr>>OP_SHIFT].opu_fld, --sp, (char *)0)<0) {
		    (void) sprintf (err_msg, "Bug! VAR field not logged");
		    return (-1);
		}
		break;
	    case ABS:  *sp = fabs (*sp); break;
	    case SQRT: *sp = sqrt (*sp); break;
	    default:
		(void) sprintf (err_msg, "Bug! bad opcode: 0x%x", instr);
		return (-1);
	    }
	    if (sp < stack) {
		(void) sprintf (err_msg, "Runtime stack overflow");
		return (-1);
	    } else if (sp - stack > MAX_STACK) {
		(void) sprintf (err_msg, "Bug! runtime stack underflow");
		return (-1);
	    }
	} while (instr != HALT);

	/* result should now be on top of stack */
	if (sp != &stack[MAX_STACK - 1]) {
	    (void) sprintf (err_msg, "Bug! stack has %d items",
							(int)(MAX_STACK - (sp-stack)));
	    return (-1);
	}
	*result = *sp;
	return (0);
}

/* starting with lcexpr pointing at a string expected to be a field name,
 * return an rcfpack(r,c,0) of the field else -1 if bad.
 * when return, leave lcexpr alone but move cexpr to just after the name.
 */
static
int parse_fieldname ()
{
	int r = -1, c = -1; 	/* anything illegal */
	char *fn = lcexpr;	/* likely faster than using the global */
	char f0, f1;
	char *dp;

	/* search for first thing not an alpha char.
	 * leave it in f0 and leave dp pointing to it.
	 */
	dp = fn;
	while (isalpha(f0 = *dp))
	    dp++;

	/* crack the new field name.
	 * when done trying, leave dp pointing at first char just after it.
	 * set r and c if we recognized it.
	 */
	if (f0 == '.') {
	    int jcontext = 0;	/* need more of then as time goes on */

	    /* object.column "dot" notation pair.
	     * crack the first portion (pointed to by fn): set r.
	     * then the second portion (pointed to by dp+1): set c.
	     */
	    f0 = fn[0];
	    f1 = fn[1];
	    switch (f0) {
	    case 'c':		    r = R_CALLISTO;
		break;
	    case 'e':		    r = R_EUROPA;
		break;
	    case 'g':		    r = R_GANYMEDE;
		break;
	    case 'i':		    r = R_IO;
		break;
	    case 'j':
				    r = R_JUPITER;
		jcontext = 1;
		break;
	    case 'm':
		if (f1 == 'a')      r = R_MARS;
		else if (f1 == 'e') r = R_MERCURY;
		else if (f1 == 'o') r = R_MOON;
		break;
	    case 'n':		    r = R_NEPTUNE;
		break;
	    case 'p':		    r = R_PLUTO;
		break;
	    case 's':
		if (f1 == 'a')      r = R_SATURN;
		else if (f1 == 'u') r = R_SUN;
		break;
	    case 'u':		    r = R_URANUS;
		break;
	    case 'x':		    r = R_OBJX;
		break;
	    case 'y':		    r = R_OBJY;
		break;
	    case 'v':		    r = R_VENUS;
		break;
	    }

	    /* now crack the column (stuff after the dp) */
	    dp++;	/* point at good stuff just after the decimal pt */
	    f0 = dp[0];
	    f1 = dp[1];
	    switch (f0) {
	    case 'a':
		if (f1 == 'l')        c = C_ALT;
		else if (f1 == 'z')   c = C_AZ;
		break;
	    case 'd':		      c = C_DEC;
		break;
	    case 'e':
		if (f1 == 'd')        c = C_EDIST;
		else if (f1 == 'l')   c = C_ELONG;
		break;
	    case 'h':
		if (f1 == 'l') {
		    if (dp[2] == 'a')              c = C_HLAT;
		    else if (dp[2] == 'o')         c = C_HLONG;
		} else if (f1 == 'r' || f1 == 'u') c = C_TUP;
		break;
	    case 'j':		      c = C_JUPITER;
		break;
	    case 'm':
		if (f1 == 'a')        c = C_MARS;
		else if (f1 == 'e')   c = C_MERCURY;
		else if (f1 == 'o')   c = C_MOON;
		break;
	    case 'n':		      c = C_NEPTUNE;
		break;
	    case 'p':
		if (f1 == 'h')        c = C_PHASE;
		else if (f1 == 'l')   c = C_PLUTO;
		break;
	    case 'r':
		if (f1 == 'a') {
		    if (dp[2] == 'z') c = C_RISEAZ;
		    else 	      c = C_RA;
		} else if (f1 == 't') c = C_RISETM;
		break;
	    case 's':
		if (f1 == 'a') {
		    if (dp[2] == 'z') c = C_SETAZ;
		    else	      c = C_SATURN;
		} else if (f1 == 'd') c = C_SDIST;
		else if (f1 == 'i')   c = C_SIZE;
		else if (f1 == 't')   c = C_SETTM;
		else if (f1 == 'u')   c = C_SUN;
		break;
	    case 't':
		if (f1 == 'a')        c = C_TRANSALT;
		else if (f1 == 't')   c = C_TRANSTM;
		break;
	    case 'u':		      c = C_URANUS;
		break;
	    case 'x':		      c = jcontext ? C_OBJX : C_JMX;
		break;
	    case 'y':		      c = jcontext ? C_OBJY : C_JMY;
		break;
	    case 'z':		      c = C_JMZ;
		break;
	    case 'v':
		if (f1 == 'e')        c = C_VENUS;
		else if (f1 == 'm')   c = C_MAG;
		break;
	    }

	    /* now skip dp on past the column stuff */
	    while (isalpha(*dp))
		dp++;
	} else {
	    /* no decimal point; some other field */
	    f0 = fn[0];
	    f1 = fn[1];
	    switch (f0) {
	    case 'd':
		if (f1 == 'a')      r = R_DAWN, c = C_DAWNV;
		else if (f1 == 'u') r = R_DUSK, c = C_DUSKV;
		break;
	    case 'j':
		if (f1 == 'I') {
		    if (fn[2] == 'I') r = R_JCML, c = C_JCMLSII;
		    else 	      r = R_JCML, c = C_JCMLSI;
		}
		break;
	    case 'n':
		r = R_LON, c = C_LONV;
		break;
	    }
	}

	cexpr = dp;
	if (r <= 0 || c <= 0) return (-1);
	return (rcfpack (r, c, 0));
}
