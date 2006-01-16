#include <jni.h>
#include "placement_SolveVPSC.h"
#include <stdio.h>
#include "solve_VPSC.h"
#include "variable.h"
#include "constraint.h"
#include "remove_rectangle_overlap.h"
#include "generate-constraints.h"
#include <assert.h>
#include <map>
#define MaxSize 500

JNIEXPORT jdouble JNICALL Java_placement_SolveVPSC_solve
  (JNIEnv *env, jobject obj, jobjectArray vName, jdoubleArray vWeight, jdoubleArray vDesPos, jintArray cLeft, jintArray cRight, jdoubleArray cGap, jdoubleArray vResult, jint mode)
{
	jsize n = env->GetArrayLength(vWeight);
	jsize m = env->GetArrayLength(cLeft);
	int i;
	double *lvWeight = env->GetDoubleArrayElements(vWeight, 0);
	double *lvDesPos = env->GetDoubleArrayElements(vDesPos, 0);
	long *lcLeft = env->GetIntArrayElements(cLeft, 0);
	long *lcRight = env->GetIntArrayElements(cRight, 0);
	double *lcGap = env->GetDoubleArrayElements(cGap, 0);
	Variable **vs=new Variable*[n];
	Constraint **cs=new Constraint*[m];
	for (i=0; i<n; i++) {
		jstring lvName = (jstring)env->GetObjectArrayElement(vName, i);
		const char *name = env->GetStringUTFChars(lvName, NULL); 
		// once upon a time variables had real names, now you'll have to 
		// track them by number.
		vs[i]=new Variable(i,lvDesPos[i],lvWeight[i]);
	}
	for (i=0; i<m; i++) {
		cs[i]=new Constraint(vs[lcLeft[i]],vs[lcRight[i]],lcGap[i]);
	}
	double cost=0;
	VPSC vpsc(vs,n,cs,m);
	if(mode==0) {
		vpsc.satisfy();
	} else {
		vpsc.solve();
	}
	for (i=0; i<n; i++) {
		double p=vs[i]->position();
		env->SetDoubleArrayRegion(vResult, i,1,&p);
	}
	for (i=0; i<m; i++) {
		delete cs[i];
	}
	delete [] cs;
	for (i=0; i<n; i++) {
		delete vs[i];
	}
	env->ReleaseIntArrayElements(cLeft, lcLeft, 0);
	env->ReleaseIntArrayElements(cRight, lcRight, 0);
	env->ReleaseDoubleArrayElements(cGap, lcGap, 0);
	env->ReleaseDoubleArrayElements(vWeight, lvWeight, 0);
	env->ReleaseDoubleArrayElements(vDesPos, lvDesPos, 0);
	delete [] vs;
	return cost;
}

static Variable **vs;
static Constraint **cs;
static int m,n;
JNIEXPORT jint JNICALL Java_placement_SolveVPSC_generateXConstraints
(JNIEnv *env, jobject obj, jdoubleArray rMinX, jdoubleArray rMaxX, jdoubleArray rMinY, jdoubleArray rMaxY, jdoubleArray rWeight) {
	n = (int)env->GetArrayLength(rWeight);
	Rectangle **rs=new Rectangle*[n];
	double *ws = env->GetDoubleArrayElements(rWeight, 0);
	double *minX = env->GetDoubleArrayElements(rMinX, 0);
	double *maxX = env->GetDoubleArrayElements(rMaxX, 0);
	double *minY = env->GetDoubleArrayElements(rMinY, 0);
	double *maxY = env->GetDoubleArrayElements(rMaxY, 0);
	for(int i=0;i<n;i++) rs[i]=new Rectangle(minX[i],maxX[i],minY[i],maxY[i]);
	m = generateXConstraints(rs, ws, n, vs, cs, true);
	return m;
}

JNIEXPORT jint JNICALL Java_placement_SolveVPSC_generateYConstraints
(JNIEnv *env, jobject obj, jdoubleArray rMinX, jdoubleArray rMaxX, jdoubleArray rMinY, jdoubleArray rMaxY, jdoubleArray rWeight) {
	n = (int)env->GetArrayLength(rWeight);
	Rectangle **rs=new Rectangle*[n];
	double *ws = env->GetDoubleArrayElements(rWeight, 0);
	double *minX = env->GetDoubleArrayElements(rMinX, 0);
	double *maxX = env->GetDoubleArrayElements(rMaxX, 0);
	double *minY = env->GetDoubleArrayElements(rMinY, 0);
	double *maxY = env->GetDoubleArrayElements(rMaxY, 0);
	for(int i=0;i<n;i++) rs[i]=new Rectangle(minX[i],maxX[i],minY[i],maxY[i]);
	m = generateYConstraints(rs, ws, n, vs, cs);
	return m;
}
using namespace std;
JNIEXPORT void JNICALL Java_placement_SolveVPSC_getConstraints
(JNIEnv *env, jobject obj, jintArray cLeft, jintArray cRight, jdoubleArray cGap) {
	map<Variable*,int> vmap;
	for(int i=0;i<n;i++) {
		vmap[vs[i]]=i;
	}
	
	for(int i=0;i<m;i++) {
		jint l=vmap[cs[i]->left];
		jint r=vmap[cs[i]->right];
		double g=cs[i]->gap;
		env->SetIntArrayRegion(cLeft, i,1,&l);
		env->SetIntArrayRegion(cRight, i,1,&r);
		env->SetDoubleArrayRegion(cGap, i,1,&g);
	}
}
JNIEXPORT void JNICALL Java_placement_SolveVPSC_removeOverlaps
(JNIEnv *env, jobject obj, jdoubleArray rMinX, jdoubleArray rMaxX, jdoubleArray rMinY, jdoubleArray rMaxY) {
	//assert(1==2); //break for debugging
	n = (int)env->GetArrayLength(rMinX);
	Rectangle **rs=new Rectangle*[n];
	double *minX = env->GetDoubleArrayElements(rMinX, 0);
	double *maxX = env->GetDoubleArrayElements(rMaxX, 0);
	double *minY = env->GetDoubleArrayElements(rMinY, 0);
	double *maxY = env->GetDoubleArrayElements(rMaxY, 0);
	for(int i=0;i<n;i++) rs[i]=new Rectangle(minX[i],maxX[i],minY[i],maxY[i]);
	removeRectangleOverlap(rs,n,0,0);
	for (i=0; i<n; i++) {
		double x=rs[i]->getMinX();
		double y=rs[i]->getMinY();
		env->SetDoubleArrayRegion(rMinX, i,1,&x);
		env->SetDoubleArrayRegion(rMinY, i,1,&y);
	}
	delete [] rs;
	env->ReleaseDoubleArrayElements(rMaxX, maxX, 0);
	env->ReleaseDoubleArrayElements(rMaxY, maxY, 0);
}