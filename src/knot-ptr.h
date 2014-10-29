#ifndef KNOT_PTR_DETECTOR
#define KNOT_PTR_DETECTOR

void knot_deleted_callback(void* knot);
void knot_created_callback(void* knot);
void check_if_knot_deleted(void* knot);

#endif
