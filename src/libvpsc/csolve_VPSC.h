/**
 * \brief Bridge for C programs to access solve_VPSC (which is in C++)
 *
 * Authors:
 *   Tim Dwyer <tgdwyer@gmail.com>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU LGPL.  Read the file 'COPYING' for more information.
 */
#ifndef _CSOLVE_VPSC_H_
#define _CSOLVE_VPSC_H_
#ifdef __cplusplus
extern "C" {
#endif
typedef struct Variable Variable;
Variable* newVariable(int id, double desiredPos, double weight);
void setVariableDesiredPos(Variable *, double desiredPos);
double getVariablePos(Variable*);

typedef struct Constraint Constraint;
Constraint* newConstraint(Variable* left, Variable* right, double gap);

typedef struct VPSC VPSC;
VPSC* newVPSC(int n, Variable* vs[], int m, Constraint* cs[]);
void deleteVPSC(VPSC*);
void deleteConstraint(Constraint*);
void deleteVariable(Variable*);
Constraint** newConstraints(int m);
void deleteConstraints(int m,Constraint**);
void remapInConstraints(Variable *u, Variable *v, double dgap);
void remapOutConstraints(Variable *u, Variable *v, double dgap);
int getLeftVarID(Constraint *c);
int getRightVarID(Constraint *c);
double getSeparation(Constraint *c);

#ifndef HAVE_POINTF_S
typedef struct pointf_s { double x, y; } pointf;
typedef struct { pointf LL, UR; } boxf;
#endif
int genXConstraints(int n, boxf[], Variable** vs, Constraint*** cs,
		int transitiveClosure);
int genYConstraints(int n, boxf[], Variable** vs, Constraint*** cs);

void satisfyVPSC(VPSC*);
void solveVPSC(VPSC*);
typedef struct IncVPSC IncVPSC;
VPSC* newIncVPSC(int n, Variable* vs[], int m, Constraint* cs[]);
void splitIncVPSC(IncVPSC*);
int getSplitCnt(IncVPSC *vpsc);
#ifdef __cplusplus
}
#endif
#endif /* _CSOLVE_VPSC_H_ */
