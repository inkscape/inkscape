#include "unicoderange.h"

#include <stdlib.h>
#include <string.h>

static unsigned int hex2int(char* s){
	int res=0;
	int i=0, mul=1;
	while(s[i+1]!='\0') i++;

	while(i>=0){
		if (s[i] > '9') res += mul * (s[i]-'A'+10);
		else res += mul * (s[i]-'0');
		i--;
		mul*=16;
	}
	return res;
}

UnicodeRange::UnicodeRange(const gchar* value){
	gchar* val = (gchar*) value;
	while(val[0] != '\0'){
		if (val[0]=='U' && val[1]=='+'){
			val += add_range(val);
		} else {
//			g_warning("adding unichar. unichar=%c", g_utf8_get_char(&val[0]));
			this->unichars.push_back(g_utf8_get_char(&val[0]));
			val++;
		}
		//skip spaces or commas
		while(val[0]==' ' || val[0]==',') val++;
	}
}

int
UnicodeRange::add_range(gchar* val){
		Urange r;
		//U+
		val+=2;
		int i=0, count=2;
		while(val[i]!='\0' && val[i]!='-' && val[i]!=' ' && val[i]!=',') i++;
		r.start = (gchar*) malloc((i+1)*sizeof(gchar*));
		strncpy(r.start, val, i);
		r.start[i] = '\0';
		val+=i;
		count+=i;
		i=0;
		if (val[0]=='-'){
			val++;
			while(val[i]!='\0' && val[i]!='-' && val[i]!=' ' && val[i]!=',') i++;
			r.end = (gchar*) malloc((i+1)*sizeof(gchar*));
			strncpy(r.end, val, i);
			r.end[i] = '\0';
			val+=i;
			count+=i;
		} else {
			r.end=NULL;
		}
//		g_warning("adding range. from %s to %s", r.start, r.end);
		this->range.push_back(r);
		return count+1;
}

bool UnicodeRange::contains(gchar unicode){
	for(unsigned int i=0;i<this->unichars.size();i++){
		if ((gunichar) unicode == this->unichars[i]) return true;
	}

	unsigned int unival;
	unival = g_utf8_get_char (&unicode);
//	g_warning("unival=%d", unival);
	char uni[9] = "00000000";
	uni[8]= '\0';
	unsigned char val;
	for (unsigned int i=7; unival>0; i--){
		val = unival & 0xf;
		unival = unival >> 4;
		if (val < 10) uni[i] = '0' + val;
		else uni[i] = 'A'+ val - 10;
	}
//	g_warning("uni=%s", uni);

	bool found;
	for(unsigned int i=0;i<this->range.size();i++){
		Urange r = this->range[i];
		if (r.end){
//			g_warning("hex2int: start=%d", hex2int(r.start));
//			g_warning("hex2int: end=%d", hex2int(r.end));
			if (unival >= hex2int(r.start) && unival <= hex2int(r.end)) return true;
		} else {
			found = true;
			for (int pos=0;pos<8;pos++){
				if (uni[pos]!='?' && uni[pos]!=r.start[pos]) found = false;
			}
			if (found) return true;
		}
	}
	return false;
}
